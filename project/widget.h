#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QString>
#include <QTimer>

#include "jackinput.h"
#include "frequencymap.h"

#define WHEEL_DIFF_COUNT 6

// a structure representing the state of a strobed wheel
typedef struct {
    // the fundamental frequency the wheel is spinning at,
    //  in revolutions per second (i.e. Hz)
    float frequency;
    // a label to go in the center of the wheel, e.g. a pitch class
    QString label;
    // the number of wheel segments
    jack_nframes_t sampleCount;
    // the buffer of wheel segments
    jack_default_audio_sample_t *sampleBuffer;
    // a pointer to the current position in the buffer
    jack_default_audio_sample_t *sample;
    // a pointer one position past the last sample in the buffer
    jack_default_audio_sample_t *endSample;
    // a buffer counting how many input samples have been added at each sample
    //  position
    int *addCounts;
    // a pointer that advances through the addCounts list
    int *addCount;
    // the amount to increment the wheel position for each input sample
    float step;
    // accumulated error between the wheel's true position and the sample pointer
    float error;
    // statistics about the stability of the wheel contents
    float maxAmplitude;
    int zeroCrossings;
    int unders;
    int overs;
    int diffs[WHEEL_DIFF_COUNT];
    int diffIndex;
    float instability;
    bool selected;
} Wheel;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

public slots:
    // update all wheels from audio input
    void updateWheels();
    // make or remake the connection to the JACK server
    void connectInput();
    // disconnect from the JACK server
    void disconnectInput();
    // connect or disconnect from the JACK server
    void toggleConnected(bool connected);
    // toggle whether to detect the closest frequency
    void toggleAutoselect(bool value);
    // toggle the visibility of advanced controls
    void toggleAdvanced(bool showAdvanced);
    // make selections
    void selectTemperament(int index);
    void selectScale(int index);
    void updateScale();
    void selectRefPitch(int index);
    void changeRefFreq(double freq);

protected:
    // populate the UI controls
    void populateSelects();
    // initialize the array of wheel structs
    void initWheels(Scale scale);
    // update stats about the date in a wheel
    void updateWheelStats(Wheel *wheel);
    // select the wheel with the best fit to the signal
    void selectWheels();
    // destroy the array of wheel structs
    void destroyWheels();
    // repaint the widget
    void paintEvent(QPaintEvent *event);
    void drawWheel(QRect r, Wheel *wheel, float alpha);

private:
    Ui::Widget *ui;
    // the input to receive audio from
    JackInput *input;
    // a frequency mapper to select pitches, scales, and temperaments
    FrequencyMap freqs;
    // whether to detect the fundamental frequency
    bool autoselect;
    // the list of wheels being shown
    Wheel *wheels;
    // the number of wheels being shown
    int wheelCount;
    // a timer to update the wheels
    QTimer *updateTimer;
};

#endif // WIDGET_H
