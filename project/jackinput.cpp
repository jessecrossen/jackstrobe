#include "jackinput.h"

#include <exception>
#include <math.h>
#include <string.h>

#include <QDebug>

// route JACK input to a class instance
static int jack_process(jack_nframes_t nframes, void *context)
{
    JackInput *jackInput = (JackInput *)context;
    return(jackInput->process(nframes));
}

JackInput::JackInput(float bufferSeconds)
{
    // initialize pointers in case of failure
    client = NULL;
    port = NULL;
    buffer = NULL;
    // connect to JACK
    jack_status_t jack_status;
    client = jack_client_open("qjackstrobe", JackNoStartServer, &jack_status);
    if ((jack_status & JackServerFailed) != 0) {
        throw JackInputException("Failed to connect to the JACK server.");
    }
    else if ((jack_status & JackServerError) != 0) {
        throw JackInputException("Failed to communicate with the JACK server.");
    }
    else if ((jack_status & JackFailure) != 0) {
        throw JackInputException("Failed to create a JACK client.");
    }
    // create a port for audio input
    port = jack_port_register(client, "in",
        JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput | JackPortIsTerminal, 0);
    if (port == NULL) {
        throw JackInputException("Failed to create a JACK output port.");
    }
    // get the sample rate to convert times
    sampleRate = jack_get_sample_rate(client);
    // create a ring buffer for storing received audio
    int bufferSamples = (int)ceil(bufferSeconds * (float)sampleRate);
    size_t bytes = bufferSamples * sizeof(jack_default_audio_sample_t);
    buffer = jack_ringbuffer_create(bytes);
    if (buffer == NULL) {
        throw JackInputException("Failed to allocate a buffer for JACK input.");
    }
    jack_ringbuffer_reset(buffer);
    // activate the client for receiving audio
    int result = jack_set_process_callback(client, jack_process, (void *)this);
    if (result != 0) {
        throw JackInputException("Failed to bind a JACK processing callback.");
    }
    result = jack_activate(client);
    if (result != 0) {
        throw JackInputException("Failed to activate JACK client.");
    }
}

int JackInput::process(jack_nframes_t nframes)
{
    size_t frameSize = sizeof(jack_default_audio_sample_t);
    // handle the case where an xrun has misaligned the ring buffer
    if (buffer->write_ptr % frameSize != 0) {
        buffer->write_ptr -= buffer->write_ptr % frameSize;
    }
    const char *audio = (const char *)jack_port_get_buffer(port, nframes);
    if (buffer == NULL) return(0);
    int bytes = nframes * frameSize;
    jack_ringbuffer_write(buffer, audio, bytes);
    return(0);
}

jack_nframes_t JackInput::read(jack_default_audio_sample_t **out)
{
    size_t frameSize = sizeof(jack_default_audio_sample_t);
    // see how much data we have in the buffer
    size_t bytesAvailable = jack_ringbuffer_read_space(buffer);
    jack_nframes_t framesAvailable = bytesAvailable / frameSize;
    // try to read the number of frames since the last time we read
    jack_nframes_t thisRead = jack_time_to_frames(client, jack_get_time());
    jack_nframes_t framesToRead = thisRead - lastRead;
    lastRead = thisRead;
    // avoid underruns
    if (framesToRead > (framesAvailable / 2)) framesToRead = (framesAvailable / 2);
    size_t bytesToRead = framesToRead * frameSize;
    // allocate a buffer for the audio
    *out = new jack_default_audio_sample_t[framesToRead];
    // read audio from the buffer
    size_t bytesRead = jack_ringbuffer_read(buffer, (char *)(*out), bytesToRead);
    // return how many frames were read
    jack_nframes_t framesRead = bytesRead / frameSize;
    return(framesRead);
}

JackInput::~JackInput() {
    // disconnect gracefully from JACK
    if (client != NULL) {
        if (port != NULL) jack_port_disconnect(client, port);
        jack_deactivate(client);
        jack_client_close(client);
    }
    // free the buffer
    if (buffer != NULL) {
        jack_ringbuffer_free(buffer);
    }
}
