#ifndef JACKINPUT_H
#define JACKINPUT_H

#include <exception>

#include <jack/jack.h>
#include <jack/ringbuffer.h>

class JackInput
{
private:
    // the JACK client we're connected as
    jack_client_t *client;
    // the JACK input port for audio
    jack_port_t *port;
    // the sample rate JACK is using
    jack_nframes_t sampleRate;
    // the buffer to store captured audio into
    jack_ringbuffer_t *buffer;
    // the frame count the last time the read function was called
    jack_nframes_t lastRead;
public:
    // initialize the input with the given buffer length in seconds
    JackInput(float bufferSeconds);
    // handle incoming audio data
    int process(jack_nframes_t nframes);
    // get all buffered audio data, leaving a certain number of seconds in
    //  the buffer to avoid underruns
    jack_nframes_t read(jack_default_audio_sample_t **out);
    // get the current sample rate
    jack_nframes_t getSampleRate() { return(sampleRate); }
    // release the JACK connections and buffer
    ~JackInput();
};

class JackInputException : public std::exception {
private:
    const char *message;
public:
    JackInputException(const char *inMessage) {
        message = inMessage;
    }
    virtual const char* what() const throw() {
        return(message);
    }
};

#endif // JACKINPUT_H
