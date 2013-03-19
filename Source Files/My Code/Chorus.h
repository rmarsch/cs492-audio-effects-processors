#ifndef __CHORUS_H__
#define __CHORUS_H__

#include "RtAudio.h"
#include "Modulator.h"
#include "FileWvOut.h"

//Generic Base class for Chorus
class Chorus{
public:

        //Destructor
        virtual ~Chorus();

        //callback function
        virtual int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ) = 0;

        virtual void initializeDelayBuffer(void) = 0 ;

        virtual void destroyDelayBuffer(void) = 0;
};

class MultiChorus : public Chorus{
public:
    static int MAX_DELAYS; //Static variable for maximum number of stages to chorus
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay (chorus unit delays are short)

    //Destructor
    ~MultiChorus(void);
    
    //Default Constructor
    MultiChorus(void);


    //Main Setter
    void setMultiChorus(int tDry, int tWet, int tDelay1, int tDelay2, int tDelay3,
        int tNumDelays, int tNumModulators, bool bandlimited);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);

private:
int dry; //Dry signal % (0-100)
int wet; //Wet signal % (0-100)
int delay1; //First delay length 0-MAX_MS_DELAY ms
int delay2; //Second delay length 0-MAX_MS_DELAY ms
int delay3; //Third delay length 0-MAX_MS_DELAY ms
int numDelays; //Number of stages being used
Modulator mod1; //Modulator object 1
Modulator mod2; //Modulator object 2
Modulator mod3; //Modulator object 3
int numModulators; //Number of sample rate modulator objects
int bufferLength; //Actual buffer length
int writeCell; //Index of the cell to write input samples to
double delayCell1; //Index of the first delay in buffer
double delayCell2; //Index of the second delay in buffer
double delayCell3; //Index of the third delay in buffer
double* delayBuffer; //Pointer to the head of the delay buffer
bool isBandlimited; //Flags the type of interpolation to use
};

class FeedbackChorus : public Chorus{
public:
    static int MAX_MS_DELAY; //Maximum length of the delay
    static int MAX_BUFFER_LENGTH; //Maximum length the delay buffer can be

    //Destructor
    ~FeedbackChorus(void);

    //Default Constructor
    FeedbackChorus(void);

    //Main Setter
    void setFeedbackChorus(int tDecay, int tDelay, bool bandlimited);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);
    //destroys the current delay buffer
    void destroyDelayBuffer(void);

    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);

private:
    int decay; // % attenuation of feedback signal
    int delay; // delay time in ms
    Modulator mod; // Modulator object
    int bufferLength; //actual buffer length
    int writeCell; //Index of the cell to write feedback+input samples to
    double delayCell; //Index of the variable delay. Represented as a double because of variable nature of the value
    double* delayBuffer; //pointer to head of the delay buffer
    bool isBandlimited; //Flags the type of interpolation to use
};

#endif