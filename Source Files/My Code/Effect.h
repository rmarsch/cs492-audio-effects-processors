#ifndef __EFFECT_H__
#define __EFFECT_H__

#include "Chorus.h"
#include "Delays.h"
#include "Reverb.h"

class Effect{
public:
    //Destructor
    ~Effect(void);
    //default Constructor
    Effect(void);

    static int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    //Wrapper for Wave File Output tick calls
    static StkFrames& tick(void* input, int nBufferFrames, StkFrames& frames);

    void setEffect(void);
    
    void chooseEffect(void);

private:
    static enum EFFECT_TYPE {SINGLE_DELAY = 1, DOUBLE_DELAY, FEEDBACK_DELAY, CHORUS, FLANGER, REVERB1, REVERB2, REVERB3} effectType; //flag for multi-stage chorus

    static SingleDelay sdelay;
    static DoubleDelay ddelay;
    static FeedbackDelay fdelay;
    static MultiChorus chorus;
    static FeedbackChorus flanger;
    static Reverb1 verb1;
    static Reverb2 verb2;
    static Reverb3 verb3;

    void setSingleDelay(void);
    void setDoubleDelay(void);
    void setFeedbackDelay(void);
    void setChorus(void);
    void setFlanger(void);
    void setReverb1(void);
    void setReverb2(void);
    void setReverb3(void);
};

#endif