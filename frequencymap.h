#ifndef FREQUENCYMAP_H
#define FREQUENCYMAP_H

#include <QString>
#include <QList>
#include <QHash>

typedef enum {
    EqualTemperament,
    SyntonicTemperament,
    JustTemperament
} TemperamentType;

typedef struct {
    QString name;
    TemperamentType type;
    float param;
} Temperament;

typedef struct {
    QString name;
    QList<QString> pitches;
} Scale;

class FrequencyMap
{
public:
    // initiliaze and update
    FrequencyMap();
    void updateFrequencies();
    // available temperaments
    QList<Temperament> temperaments;
    // available scales
    QList<Scale> scales;
    // a map from pitch name to MIDI note number
    QHash<QString, int> pitches;
    // a list of available pitch names
    QList<QString> pitchNames;
    // a mapping from MIDI note number to frequency
    float frequencies[128];
    // current temperament configuration
    int temperamentIndex;
    int refPitch;
    float refFreq;

protected:
    // a table of ratios for just intonations
    int justRatios[12][11][2] = {
        // 5-limit (Symmetric 1 Aug 4th)
        { {16,15}, {9,8}, {6,5}, {5,4}, {4,3}, {45,32}, {3,2}, {8,5}, {5,3}, {16,9}, {15,8} },
        // 5-limit (Symmetric 1 Dim 5th)
        { {16,15}, {9,8}, {6,5}, {5,4}, {4,3}, {64,45}, {3,2}, {8,5}, {5,3}, {16,9}, {15,8} },
        // 5-limit (Symmetric 2 Aug 4th)
        { {16,15}, {10,9}, {6,5}, {5,4}, {4,3}, {45,32}, {3,2}, {8,5}, {5,3}, {9,5}, {15,8} },
        // 5-limit (Symmetric 2 Dim 5th)
        { {16,15}, {10,9}, {6,5}, {5,4}, {4,3}, {64,45}, {3,2}, {8,5}, {5,3}, {9,5}, {15,8} },
        // 5-limit (Asymmetric Standard Aug 4th)
        { {16,15}, {9,8}, {6,5}, {5,4}, {4,3}, {45,32}, {3,2}, {8,5}, {5,3}, {9,5}, {15,8} },
        // 5-limit (Asymmetric Standard Dim 5th)
        { {16,15}, {9,8}, {6,5}, {5,4}, {4,3}, {64,45}, {3,2}, {8,5}, {5,3}, {9,5}, {15,8} },
        // 5-limit (Asymmetric Extended Aug 4th)
        { {16,15}, {9,8}, {6,5}, {5,4}, {4,3}, {25,18}, {3,2}, {8,5}, {5,3}, {9,5}, {15,8} },
        // 5-limit (Asymmetric Extended Dim 5th)
        { {16,15}, {9,8}, {6,5}, {5,4}, {4,3}, {36,25}, {3,2}, {8,5}, {5,3}, {9,5}, {15,8} },
        // 7-limit (Aug 4th)
        { {16,15}, {8,7}, {6,5}, {5,4}, {4,3}, {7,5}, {3,2}, {8,5}, {5,3}, {7,4}, {15,8} },
        // 7-limit (Dim 5th)
        { {16,15}, {8,7}, {6,5}, {5,4}, {4,3}, {10,7}, {3,2}, {8,5}, {5,3}, {7,4}, {15,8} },
        // 17-limit (Aug 4th)
        { {16,15}, {8,7}, {6,5}, {5,4}, {4,3}, {17,12}, {3,2}, {8,5}, {5,3}, {7,4}, {15,8} },
        // 17-limit (Dim 5th)
        { {16,15}, {8,7}, {6,5}, {5,4}, {4,3}, {24,17}, {3,2}, {8,5}, {5,3}, {7,4}, {15,8} }
    };
};

#endif // FREQUENCYMAP_H
