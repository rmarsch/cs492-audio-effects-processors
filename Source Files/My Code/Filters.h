#ifndef __FILTERS_H__
#define __FILTERS_H__

#include "RtAudio.h"
#include "FileWvOut.h"

class Allpass{
public:
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay 

    ~Allpass(void);
    Allpass(void);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

    //Computation functions
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);
    
    double computeSample(double& in);

    void setAllpass(int tDelay, int tDecay);

private:
    int delay; //0-5000ms
    int decay; //0%-100%
    double* delayBuffer; //Delay buffer
    int bufferLength; //Buffer length
    int writePtr; //Write pointer for both buffers
    int delayPtr; //Pointer to the delay cell
};

class Comb{
public:
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay 
    ~Comb(void);
    Comb(void);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

    //Computation functions
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);
    
    double computeSample(double& in);

    void setComb(int tDelay, int tDecay);

private:
    int delay; //0-5000ms
    int decay; //0%-100%
    double* delayBuffer; //Delay buffer
    int bufferLength; //Buffer length
    int writePtr; //Write pointer for both buffers
    int delayPtr; //Pointer to the delay cell
};

class LPComb{
public:
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay 

    ~LPComb(void);
    LPComb(void);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

    //Computation functions
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);
    
    double computeSample(double& in);

    void setLPComb(int tDelay, int tDecay1, int tDecay2); //implicitly defines delay2

private:
    int delay; //0-5000ms
    int decay1; //0%-100%
    int decay2; //0%-100%
    double* delayBuffer; //Delay buffer
    int bufferLength; //Buffer length
    int writePtr; //Write pointer for both buffers
    int delayPtr; //Pointer to the delay cell
    int delayPtrN1; //Pointer to the feedback delay cell - 1
};

#endif