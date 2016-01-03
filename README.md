# What Is It?

It's a simple strobe tuner that accepts audio from [JACK](http://jackaudio.org/) and helps you fine-tune musical instruments (or anything else that makes periodic audio waves). It uses the [Qt 5](http://doc.qt.io/qt-5/index.html) framework.

# Installing and Modifying

To install it you'll need the Qt 5 development headers, the JACK development headers, qmake, and a basic build environment. On Ubuntu (or similar), you should be able to do something like this in the console:
```
$ sudo apt-get install build-essential qt5-default libjack-jackd2-dev
```

Assuming you've got all that, this is how you would build and install jackstrobe:
```
$ git clone https://github.com/jessecrossen/jackstrobe.git jackstrobe
$ cd jackstrobe/project
$ qmake && make && sudo cp jackstrobe /usr/local/bin/
```

In theory jackstrobe should work on any OS supported by Qt and JACK (which is most of them), but I haven't tested it on anything but Linux. If anyone out there knows how to make packages, that would be extremely appreciated.

If you want to hack on jackstrobe, the project directory is also a QtCreator project.

# Using

If you've never used a strobe tuner before, you may be in for a treat. Strobe tuners have a faster response and greater accuracy than ordinary FFT-based tuners, and are able to tune in sub-cent intervals. Follow these steps for a quick start (you'll need to be familiar with using JACK first):

1. Start your JACK server and then start jackstrobe.
2. Connect audio from your instrument to the `in` port on the jackstrobe client.
3. Select an instrument from the dropdown. If your instrument isn't in there, go through the "Chromatic" options from lowest to highest to see what works best.
4. You will see a wheel for each possible note. If you're playing in the neighborhood of a given note, you should see a distinct pattern on that wheel. Sometimes the note may select a different wheel from the intended one, but as long as they're from the same pitch class (e.g. E4 and E3 or C6 and C2), it doesn't really matter.
5. If the pattern is rotating clockwise, you're sharp, and if it's rotating counterclockwise you're flat. Adjust your instrument until the pattern is still or moving very slowly.

If you want to get pickier, click the » at the top right to get advanced controls. You can select from a number of strange and wonderful temperament systems and change the reference note and reference frequency being used. The default settings are good for the majority of modern Western music.

# Shortcomings

* The list of instruments is just the commonest Western stringed instruments, since at the moment because being more comprehensive would take data-entry effort. If you want more instruments and tunings, please file an issue and I'll add what you need, or fork/pull and add it yourself. Eventually I would love to add [all these](https://en.wikipedia.org/wiki/Stringed_instrument_tunings) but it's a big job.
* I've only tested with a guitar so far. If it isn't behaving with your instrument, please file an issue and attach an audio file of you playing long single notes so I can try to improve the default settings.
* I haven't added any well temperaments yet. If you're tuning up to play some authentic Bach or something, please file an issue, preferrably with specific note ratios or formulas. I'm willing to add funkier temperaments too, as long as they can map onto a 12-tone scale.
