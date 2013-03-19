#ifndef __MODULATOR_H__
#define __MODULATOR_H__

#include <math.h>
#include <cmath>

class Modulator{
public:
    static int STD_BUFFER_LENGTH; //Maximum length of the coefficient buffer
    static double MIN_HZ; //Minimum frequency of the modulator
    static double MAX_HZ; //Maximum frequency of the modulator
    static int MAX_MODS; //Maximum number of supported simultaneous Modulator objects for processing
    
    ~Modulator(void);

    Modulator(void);

    void setModulator(void);

    //Creates a buffer ring containing coefficient values calculated for
    //the given Wave shape over 1 period of the shape
    void initializeCoefficients(void);

    void destroyCoefficients(void);

    double nextCoefficient(void);

private:
    enum modShape {sine, saw, triangular, square} shape; //Shape of the modulator wave
    double freq; //Frequency of the modulator wave
    double depth; //Amplitude of the modulator wave
    int coefficientIndex; //Index of the currently considered coefficient
    double* coefficient; //Pointer to head of buffer of coefficient that is modulated
    int bufferLength; //Actual buffer length
};


#endif