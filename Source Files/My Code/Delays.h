#ifndef __DELAYS_H__
#define __DELAYS_H__

#include "RtAudio.h"
#include "FileWvOut.h"

//Generic Base class for Delay
class Delay{
public:
        //Destructor
        virtual ~Delay();

        //callback function
        virtual int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ) = 0;

        virtual void initializeDelayBuffer(void) = 0 ;

        virtual void destroyDelayBuffer(void) = 0;
};

//***************SINGLE DELAY *********************************************************************
class SingleDelay : public Delay{
public:
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay (chorus unit delays are short)

    //Destructor
    ~SingleDelay(void);
    
    //Default Constructor
    SingleDelay(void);

    //Tick functions
    void tick(void *output, void *input, int nBufferFrames);
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);
    //STK Wrapper fo Real-Time Audio
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    //Set functions
    void setSingleDelay(double tDry, double tWet, unsigned int tDelay);
    void setSingleDelay(void);
    
    void setDry(double tDry);
    void setWet(double tWet);
    void setDelay(unsigned int tDelay);
    
    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

private:
    double dry; //% of dry signal
    double wet; //% of wet signal
    unsigned int delay; //Delay in milliseconds
    int delayCell; //Index in the delayBuffer of the delay to add
    double *delayBuffer; //Delay buffer is 1 second long at sample rate SAMPLE_RATE
    unsigned int bufferCell; //Pointer to the current delay buffer cell
    int bufferLength; //Actual buffer length
};

//*******************DOUBLE DELAY ********************************************************************
class DoubleDelay : public Delay{
public:
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay (chorus unit delays are short)

    //Destructor
    ~DoubleDelay(void);
    
    //Default Constructor
    DoubleDelay(void);

    //Tick functions
    void tick(void *output, void *input, int nBufferFrames);
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);
    //STK Wrapper fo Real-Time Audio
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    //Set functions
    void setDoubleDelay(void);
    void setDoubleDelay(double tDry, double tWet1, double tWet2, unsigned int tDelay1, unsigned int tDelay2);
    void setDry(double tDry);
    void setWet1(double tWet1);
    void setDelay1(unsigned int tDelay1);
    void setWet2(double tWet2);   
    void setDelay2(unsigned int tDelay2);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

private:
    double dry; //% of dry signal
    double wet1; //% of wet signal of first delay
    double wet2; //% of wet signal of second delay
    unsigned int delay1; //millisecond delay time of first delay
    unsigned int delay2; //millisecond delay time of second delay
    int delayCell1; //Index in the delayBuffer of the delay1 to add
    int delayCell2; //Index in the delayBuffer of the delay2 to add
    unsigned int bufferCell; //Pointer to the current delay buffer cell
    double *delayBuffer; //Delay buffer is 1 second long at sample rate SAMPLE_RATE
    int bufferLength; //Actual buffer length
};

//*******************FEEDBACK DELAY ********************************************************
class FeedbackDelay : public Delay{
public:
    static int MAX_BUFFER_LENGTH; //Static variable for maximum buffer length
    static int MAX_MS_DELAY; //Maximum length of the delay (chorus unit delays are short)

    //Destructor
    ~FeedbackDelay(void);
    
    //Default Constructor
    FeedbackDelay(void);

    //Tick functions
    void tick(void *output, void *input, int nBufferFrames);
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);
    //STK Wrapper fo Real-Time Audio
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    //Set and Get functions
    void setFeedbackDelay(void);
    void setFeedbackDelay(double tGain, double tDecay, unsigned int tDelay);
    void setGain(double tGain);
    void setDecay(double tDecay);
    void setDelay(unsigned int tDelay);

    //initializes the delayBuffer into an array of proper size
    void initializeDelayBuffer(void);

    //destroys the current delay buffer
    void destroyDelayBuffer(void);

private:
    double gain; //% boost to signal from 0%-200%
    int decay; //attenuation of the feedback from 0% - 99%
    unsigned int delay; //Delay in milliseconds
    int delayCell; //Index in the delayBuffer of the delay to add
    unsigned int bufferCell; //Pointer to the current delay buffer cell
    double *delayBuffer; //Delay buffer is 1 second long at sample rate SAMPLE_RATE   
    int bufferLength; //Actual buffer length
};

#endif