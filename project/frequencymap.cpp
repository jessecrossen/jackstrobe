#include "frequencymap.h"

#include "math.h"

FrequencyMap::FrequencyMap()
{
    // temperaments
    temperaments.append({"12-edo (standard)", EqualTemperament, 12.0 });
    temperaments.append({"Syntonic (5-TET)", SyntonicTemperament, pow(2.0, 3.0 / 5.0) });
    temperaments.append({"Syntonic (22-TET)", SyntonicTemperament, pow(2.0, 13.0 / 22.0) });
    temperaments.append({"Syntonic (17-TET)", SyntonicTemperament, pow(2.0, 10.0 / 17.0) });
    temperaments.append({"Syntonic (Pythagorean)", SyntonicTemperament, 1.5 });
    temperaments.append({"Syntonic (12-TET)", SyntonicTemperament, pow(2.0, 7.0 / 12.0) });
    temperaments.append({"Syntonic (43-TET, 1/5 Comma)", SyntonicTemperament, pow(2.0, 25.0 / 43.0) });
    temperaments.append({"Syntonic (31-TET, 1/4 Comma)", SyntonicTemperament, pow(2.0, 18.0 / 31.0) });
    temperaments.append({"Syntonic (50-TET, 2/7 Comma)", SyntonicTemperament, pow(2.0, 29.0 / 50.0) });
    temperaments.append({"Syntonic (19-TET, 1/3 Comma)", SyntonicTemperament, pow(2.0, 11.0 / 19.0) });
    temperaments.append({"Syntonic (26-TET)", SyntonicTemperament, pow(2.0, 15.0 / 26.0) });
    temperaments.append({"Syntonic (7-TET)", SyntonicTemperament, pow(2.0, 4.0 / 7.0) });
    temperaments.append({"Just 5-limit (Symmetric 1 Aug 4th)", JustTemperament, 0 });
    temperaments.append({"Just 5-limit (Symmetric 1 Dim 5th)", JustTemperament, 1 });
    temperaments.append({"Just 5-limit (Symmetric 2 Aug 4th)", JustTemperament, 2 });
    temperaments.append({"Just 5-limit (Symmetric 2 Dim 5th)", JustTemperament, 3 });
    temperaments.append({"Just 5-limit (Asymmetric Standard Aug 4th)", JustTemperament, 4 });
    temperaments.append({"Just 5-limit (Asymmetric Standard Dim 5th)", JustTemperament, 5 });
    temperaments.append({"Just 5-limit (Asymmetric Extended Aug 4th)", JustTemperament, 6 });
    temperaments.append({"Just 5-limit (Asymmetric Extended Dim 5th)", JustTemperament, 7 });
    temperaments.append({"Just 7-limit (Aug 4th)", JustTemperament, 8 });
    temperaments.append({"Just 7-limit (Dim 5th)", JustTemperament, 9 });
    temperaments.append({"Just 17-limit (Aug 4th)", JustTemperament, 10 });
    temperaments.append({"Just 17-limit (Dim 5th)", JustTemperament, 11 });

    // scales
    scales.append({"Banjo (standard)", {"G4","D3","G3","B3","D4"}});
    scales.append({"Bass Guitar (standard)", {"E1","A1","D2","G2"}});
    scales.append({"Cello (standard)", {"C2","G2","D3","A3"}});
    scales.append({"Guitar (standard)", {"E2","A2","D3","G3","B3","E4"}});
    scales.append({"Mandolin", {"G3","D4","A4","E5"}});
    scales.append({"Ukelele (Concert)", {"G4","C4","E4","A4"}});
    scales.append({"Violin", {"G3","D4","A4","E5"}});
    scales.append({"Chromatic 1", {"C1","C#1","D1","D#1","E1","F1","G1","G#1","A1","A#1","B1"}});
    scales.append({"Chromatic 2", {"C2","C#2","D2","D#2","E2","F2","G2","G#2","A2","A#2","B2"}});
    scales.append({"Chromatic 3", {"C3","C#3","D3","D#3","E3","F3","G3","G#3","A3","A#3","B3"}});
    scales.append({"Chromatic 4", {"C4","C#4","D4","D#4","E4","F4","G4","G#4","A4","A#4","B4"}});
    scales.append({"Chromatic 5", {"C5","C#5","D5","D#5","E5","F5","G5","G#5","A5","A#5","B5"}});
    scales.append({"Chromatic 6", {"C6","C#6","D6","D#6","E6","F6","G6","G#6","A6","A#6","B6"}});

    // pitch names
    QList<QString> octaves({"0", "1", "2", "3", "4", "5", "6", "7", "8"});
    QList<QString> sharps({"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"});
    QList<QString> flats({"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"});
    QString name;
    int num = 12; // the MIDI note number for C0
    for (int oct = 0; oct < octaves.length(); oct++) {
        for (int i = 0; i < sharps.length(); i++) {
            if (sharps.at(i) == flats.at(i)) {
                name = sharps.at(i) + octaves.at(oct);
                pitchNames.append(name);
                pitches.insert(name, num);
            }
            else {
                name = sharps.at(i) + "/" + flats.at(i) + octaves.at(oct);
                pitchNames.append(name);
                pitches.insert(name, num);
                pitches.insert(sharps.at(i) + octaves.at(oct), num);
                pitches.insert(flats.at(i) + octaves.at(oct), num);
            }
            num++;
        }
    }

    // set defaults to 12-edo at A4 = 440.0 Hz
    temperamentIndex = 0;
    refPitch = 69;
    refFreq = 440.0;

    // populate frequencies
    updateFrequencies();
}

void FrequencyMap::updateFrequencies()
{
    int p;
    Temperament temp = temperaments[temperamentIndex];
    // set the reference pitch
    frequencies[refPitch] = refFreq;
    // build the octave above the reference pitch
    if (temp.type == EqualTemperament) {
        // in equal temperament the parameter is the number of divisions of the octave,
        //  which should generally be 12 since we have 12 pitch classes
        float divisions = temp.param;
        for (p = 1; p < 12; p++) {
            frequencies[refPitch + p] =
                refFreq * pow(2.0, (float)p / divisions);
        }
    }
    else if (temp.type == SyntonicTemperament) {
        // in syntonic temperaments the parameter is the ratio of a perfect fifth
        float Fr = temp.param;
        float Or = 2.0;
        frequencies[refPitch+1]  = refFreq * (pow(Fr, 7)  / pow(Or, 4));
        frequencies[refPitch+2]  = refFreq * (pow(Fr, 2)  / pow(Or, 2));
        frequencies[refPitch+3]  = refFreq * (pow(Fr, 9)  / pow(Or, 5));
        frequencies[refPitch+4]  = refFreq * (pow(Fr, 4)  / pow(Or, 2));
        frequencies[refPitch+5]  = refFreq * (pow(Fr, 11) / pow(Or, 6));
        frequencies[refPitch+6]  = refFreq * (pow(Fr, 6)  / pow(Or, 3));
        frequencies[refPitch+7]  = refFreq * Fr;
        frequencies[refPitch+8]  = refFreq * (pow(Fr, 8)  / pow(Or, 4));
        frequencies[refPitch+9]  = refFreq * (pow(Fr, 3)  / pow(Or, 2));
        frequencies[refPitch+10] = refFreq * (pow(Fr, 10) / pow(Or, 5));
        frequencies[refPitch+11] = refFreq * (pow(Fr, 5)  / pow(Or, 2));
    }
    else if (temp.type == JustTemperament) {
        // in just temperaments the parameter is an index into the ratio table
        int ri = (int)temp.param;
        for (p = 0; p < 11; p++) {
            frequencies[refPitch+p+1] = refFreq *
                ((float)justRatios[ri][p][0] / (float)justRatios[ri][p][1]);
        }
    }
    // fill in the other octaves by doubling and halving
    for (p = refPitch + 12; p < 128; p++) {
        frequencies[p] = frequencies[p - 12] * 2.0;
    }
    for (p = refPitch - 1; p >= 0; p--) {
        frequencies[p] = frequencies[p + 12] * 0.5;
    }
}
