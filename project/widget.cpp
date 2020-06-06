#include "widget.h"
#include "ui_widget.h"

#include <math.h>
#include <string.h>

#include <QPainter>
#include <QPainterPath>
#include <QConicalGradient>
#include <QMessageBox>
#include <QDebug>
#include <QApplication>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    // initialize pointers
    input = NULL;
    wheels = NULL;
    // set up the UI
    ui->setupUi(this);
    populateSelects();
    // hide the advanced interface
    toggleAdvanced(false);
    // connect the audio input
    connectInput();
    // autoselect by default
    toggleAutoselect(true);
    // initialize wheel definitions
    selectScale(0);
    // start the update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateWheels()));
    updateTimer->start(50);
}

void Widget::populateSelects()
{
    int i, p;
    QString s;
    for (i = 0; i < freqs.scales.length(); i++) {
        ui->selectScale->addItem(freqs.scales.at(i).name, i);
    }
    for (i = 0; i < freqs.temperaments.length(); i++) {
        ui->selectTemperament->addItem(freqs.temperaments.at(i).name, i);
        if (i == freqs.temperamentIndex) ui->selectTemperament->setCurrentIndex(i);
    }
    for (i = 0; i < freqs.pitchNames.length(); i++) {
        s = freqs.pitchNames.at(i);
        p = freqs.pitches[s];
        ui->refPitch->addItem(s, p);
        if (p == 69 /* (A4) */) {
            ui->refPitch->setCurrentIndex(i);
        }
    }
    ui->refFreq->setValue(freqs.refFreq);
}

void Widget::selectScale(int index)
{
    initWheels(freqs.scales[index]);
}
void Widget::updateScale()
{
    selectScale(ui->selectScale->currentIndex());
}

void Widget::selectTemperament(int index)
{
    if (index != freqs.temperamentIndex) {
        freqs.temperamentIndex = index;
        freqs.updateFrequencies();
    }
}

void Widget::selectRefPitch(int index)
{
    int pitch = ui->refPitch->itemData(index).toInt();
    if (pitch != freqs.refPitch) {
        freqs.refPitch = pitch;
        freqs.updateFrequencies();
        updateScale();
    }
}

void Widget::changeRefFreq(double freq) {
    if (freq != freqs.refFreq) {
        freqs.refFreq = freq;
        freqs.updateFrequencies();
        updateScale();
    }
}

void Widget::initWheels(Scale scale)
{
    // remove any existing wheel definitions
    destroyWheels();
    // make new ones
    float period;
    int sampleCount, d;
    float sampleRate = (input != NULL) ? (float)input->getSampleRate() : 44100.0;
    QString pitch;
    wheelCount = scale.pitches.length();
    wheels = new Wheel[wheelCount];
    Wheel *wheel = wheels;
    for (int i = 0; i < wheelCount; i++) {
        pitch = scale.pitches.at(i);
        wheel->frequency = freqs.frequencies[freqs.pitches[pitch]];
        wheel->label = pitch;
        if (wheel->frequency >= 20.0) {
            period = sampleRate / wheel->frequency;
            sampleCount = (int)period - 1;
        }
        else {
            period = 2.0;
            sampleCount = 2;
        }
        wheel->sampleCount = sampleCount;
        wheel->sampleBuffer = new jack_default_audio_sample_t[sampleCount];
        wheel->sample = wheel->sampleBuffer;
        wheel->endSample = wheel->sample + sampleCount;
        wheel->addCounts = new int[sampleCount];
        wheel->addCount = wheel->addCounts;
        wheel->step = (float)sampleCount / period;
        wheel->error = 0.0;
        wheel->unders = 0;
        wheel->overs = 0;
        wheel->diffIndex = 0;
        for (d = 0; d < WHEEL_DIFF_COUNT; d++) {
            wheel->diffs[d] = 0;
        }
        wheel++;
    }
}

void Widget::updateWheels()
{
    if (input == NULL) return;
    // clear existing wheel data
    int w;
    Wheel *wheel = wheels;
    for (w = 0; w < wheelCount; w++) {
        memset(wheel->sampleBuffer, 0,
            wheel->sampleCount * sizeof(*(wheel->sampleBuffer)));
        memset(wheel->addCounts, 0,
            wheel->sampleCount * sizeof(*(wheel->addCounts)));
        wheel++;
    }
    // get audio input
    jack_default_audio_sample_t *samples = NULL;
    jack_nframes_t sampleCount = input->read(&samples);
    jack_default_audio_sample_t *sample = samples;
    // add input samples to wheels
    jack_nframes_t s;
    int intStep;
    for (s = 0; s < sampleCount; s++) {
        wheel = wheels;
        for (w = 0; w < wheelCount; w++) {
            *(wheel->sample) += *sample;
            *(wheel->addCount) += 1;
            // advance the wheel sample pointer
            wheel->error += wheel->step;
            if (wheel->error >= 1.0) {
                intStep = (int)wheel->error;
                wheel->sample += intStep;
                wheel->addCount += intStep;
                wheel->error -= (float)intStep;
            }
            if (wheel->sample >= wheel->endSample) {
                intStep = wheel->sample - wheel->endSample;
                wheel->sample = wheel->sampleBuffer + intStep;
                wheel->addCount = wheel->addCounts + intStep;
            }
            wheel++;
        }
        sample++;
    }
    // release sample buffer
    delete[] samples;
    // do post-processing of samples in the wheel
    wheel = wheels;
    int *addCount;
    jack_default_audio_sample_t maxAmplitude;
    jack_default_audio_sample_t amplify;
    jack_default_audio_sample_t amplitude;
    for (w = 0; w < wheelCount; w++) {
        maxAmplitude = 0.0;
        // average added samples and get the maximum amplitude
        addCount = wheel->addCounts;
        sample = wheel->sampleBuffer;
        for (s = 0; s < wheel->sampleCount; s++) {
            *sample /= (float)*addCount;
            // track the maximum amplitude
            amplitude = fabs(*sample);
            if (amplitude > maxAmplitude) maxAmplitude = amplitude;
            // advance to the next sample
            addCount++;
            sample++;
        }
        // normalize all samples to compensate for low levels
        //  (unless we're basically getting silence)
        if (maxAmplitude > 0.0001) {
            amplify = 1.0 / maxAmplitude;
            sample = wheel->sampleBuffer;
            for (s = 0; s < wheel->sampleCount; s++) {
                *sample *= amplify;
                sample++;
            }
        }
        wheel->maxAmplitude = maxAmplitude;
        // update stats
        updateWheelStats(wheel);
        wheel++;
    }
    selectWheels();
    // repaint the wheels
    repaint();
}

void Widget::updateWheelStats(Wheel *wheel)
{
    jack_nframes_t s;
    int unders, overs, diff, diffSum, d;
    // reset counters
    wheel->zeroCrossings = unders = overs = 0;
    // loop over samples
    jack_default_audio_sample_t *sample = wheel->sampleBuffer;
    jack_default_audio_sample_t last = wheel->sampleBuffer[wheel->sampleCount - 1];
    for (s = 0; s < wheel->sampleCount; s++) {
        // count samples above and below zero
        if (*sample < 0.0) {
            if (last >= 0.0) wheel->zeroCrossings++;
            unders++;
        }
        else {
            if (last < 0.0) wheel->zeroCrossings++;
            overs++;
        }
        last = *sample;
        sample++;
    }
    diff = abs(wheel->unders - unders) + abs(wheel->overs - overs);
    wheel->unders = unders;
    wheel->overs = overs;
    wheel->diffs[wheel->diffIndex] = diff;
    wheel->diffIndex = (wheel->diffIndex + 1) % WHEEL_DIFF_COUNT;
    diffSum = 0;
    for (d = 0; d < WHEEL_DIFF_COUNT; d++) {
        diffSum += wheel->diffs[d];
    }
    wheel->instability =
        (float)diffSum / (float)(WHEEL_DIFF_COUNT * wheel->sampleCount);
}

void Widget::selectWheels()
{
    int i;
    float maxAmplitude = 0.0;
    for (i = 0; i < wheelCount; i++) {
        wheels[i].selected = true;
        if (wheels[i].maxAmplitude > maxAmplitude) {
            maxAmplitude = wheels[i].maxAmplitude;
        }
    }
    if (autoselect) {
        float cutoff = maxAmplitude * 0.95;
        int minZC = 1000000;
        for (i = 0; i < wheelCount; i++) {
            if (wheels[i].maxAmplitude < cutoff) {
                wheels[i].selected = false;
            }
            else if (wheels[i].zeroCrossings < minZC) {
                minZC = wheels[i].zeroCrossings;
            }
        }
        for (i = 0; i < wheelCount; i++) {
            if (wheels[i].zeroCrossings > minZC) {
                wheels[i].selected = false;
            }
        }
    }
}

void Widget::destroyWheels()
{
    if (wheels != NULL) {
        for (int i = 0; i < wheelCount; i++) {
            delete[] wheels[i].sampleBuffer;
            delete[] wheels[i].addCounts;
        }
        delete[] wheels;
        wheels = NULL;
    }
    wheelCount = 0;
}

void Widget::toggleConnected(bool connected)
{
    if (connected) connectInput();
    else disconnectInput();
}

void Widget::connectInput()
{
    // if input is already connected, we're done
    if (input != NULL) return;
    try {
        input = new JackInput(0.2);
        ui->toggleConnected->setChecked(true);
    }
    catch (JackInputException& e) {
        ui->toggleConnected->setChecked(false);
        QMessageBox *msg = new QMessageBox(QMessageBox::Critical,
            "JACK Input Error", e.what());
        msg->exec();
    }
}

void Widget::disconnectInput()
{
    if (input != NULL) {
        delete input;
        input = NULL;
        ui->toggleConnected->setChecked(false);
    }
}

void Widget::toggleAutoselect(bool value)
{
    autoselect = value;
    ui->toggleAutoselect->setChecked(value);
}

void Widget::toggleAdvanced(bool showAdvanced)
{
    if (! showAdvanced) {
        ui->advancedFrame->setMaximumWidth(0);
        ui->toggleAdvanced->setText("»");
        ui->toggleAdvanced->setChecked(false);
    }
    else {
        ui->advancedFrame->setMaximumWidth(QWIDGETSIZE_MAX);
        ui->toggleAdvanced->setText("«");
        ui->toggleAdvanced->setChecked(true);
    }
}

void Widget::paintEvent(QPaintEvent *)
{
    // check boundary conditions
    if ((! (width() > 0)) || (! (height() > 0))) return;
    if ((! (wheelCount > 0)) || (wheels == NULL)) return;
    // set layout params
    QRect bounds = ui->wheelArea->geometry();
    int w = bounds.width();
    int h = bounds.height();
    // lay out the wheels in a grid of squares
    int columns = 1;
    int rows;
    while ((rows = h / (w / columns)) * columns < wheelCount) {
        columns++;
    }
    w /= columns;
    h /= rows;
    // draw the wheels
    float alpha;
    int margin = 6;
    QRect r(bounds.x(), bounds.y(), w, h);
    r.adjust(margin, margin, - margin, - margin);
    Wheel *wheel = wheels;
    for (int i = 0; i < wheelCount; i++) {
        if (wheel->instability <= 0.05) alpha = 1.0;
        else {
            alpha = 1.0 - (wheel->instability - 0.05) / 0.10;
            if (alpha < 0.05) alpha = 0.05;
        }
        if (autoselect) alpha *= wheel->selected ? 1.0 : 0.05;
        drawWheel(r, wheel, alpha);
        // advance to the next position
        r.moveLeft(r.left() + w);
        if (r.right() > width()) {
            r.moveLeft(bounds.x());
            r.moveTop(r.top() + h);
        }
        wheel++;
    }
}

void Widget::drawWheel(QRect r, Wheel *wheel, float alpha)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(r.center().x(), r.center().y());
    // precompute dimensions and steps
    float outerRadius = (float)qMin(r.width(), r.height()) / 2.0;
    float innerRadius = outerRadius / 2.0;
    float phaseStep = 1.0 / (float)wheel->sampleCount;
    // set up a gradient to make the wheel contents
    painter.setPen(Qt::NoPen);
    jack_default_audio_sample_t *sample = wheel->sampleBuffer;
    float value;
    float phase = 0.0;
    QColor color;
    QConicalGradient gradient(QPointF(0, 0), 90);
    for (jack_nframes_t s = 0; s < wheel->sampleCount; s++) {
        value = qMin(1.0f, fabsf(*sample)) * alpha;
        if (*sample > 0.0) {
            color = QColor::fromHsvF(0.0, 0.0, 1.0, value);
        }
        else {
            color = QColor::fromHsvF(0.0, 0.0, 0.0, value);
        }
        gradient.setColorAt(phase, color);
        sample++;
        phase += phaseStep;
    }
    // draw the wheel contents
    QPainterPath path;
    path.addEllipse(QPointF(0.0, 0.0), outerRadius, outerRadius);
    QPainterPath hole;
    hole.addEllipse(QPointF(0.0, 0.0), innerRadius, innerRadius);
    path = path.subtracted(hole);
    painter.setPen(QPen(QApplication::palette().windowText(), 1));
    painter.setBrush(QBrush(gradient));
    painter.drawPath(path);
    // restore context
    painter.restore();
    // draw the label in the center
    if (wheel->label != NULL) {
        QFont font = painter.font();
        font.setPixelSize((int)floor(innerRadius * 0.75));
        painter.setFont(font);
        painter.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, wheel->label);
    }
}

Widget::~Widget()
{
    delete ui;
    delete updateTimer;
    destroyWheels();
}
