#ifndef __REVERB_H__
#define __REVERB_H__

#include "RtAudio.h"
#include "FileWvOut.h"
#include "Filters.h"

class Reverb1{
public:
    static int MAX_MS_DELAY; //Maximum length of the delay 

    ~Reverb1(void);
    Reverb1(void);

    //Computation functions
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);

    double computeSample(double& in);

    void setMix(int tMix);

    void setAP(int APnum, int tDelay, int tDecay);

private:
    int mix; //ratio of Wet / Dry signals
    Allpass AP1;
    Allpass AP2;
    Allpass AP3;
    Allpass AP4;
    Allpass AP5;
};

class Reverb2{
public:
    static int A_MAX_MS_DELAY; //Maximum length of the allpass delay 
    static int C_MAX_MS_DELAY; //Maximum length of the comb delay

    ~Reverb2(void);
    Reverb2(void);

    //Computation functions
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);

    double computeSample(double& in);

    void setMix(int tMix);

    void setAP(int APnum, int tDelay, int tDecay);

    void setComb(int Combnum, int tDelay, int tDecay);

private:
    int mix; //ratio of Wet / Dry signals
    Comb C1;
    Comb C2;
    Comb C3;
    Comb C4;
    Allpass AP1;
    Allpass AP2;
};

class Reverb3{
public:
    static int A_MAX_MS_DELAY; //Maximum length of the allpass delay 
    static int C_MAX_MS_DELAY; //Maximum length of the comb delay
    ~Reverb3(void);
    Reverb3(void);

    //Computation functions
    int callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData );

    void tick(void *outputBuffer, void *input,int nBufferFrames);
    
    StkFrames& tick(void *input, int nBufferFrames, StkFrames& frames);

    double computeSample(double& in);

    void setMix(int tMix);

    void setAP(int tDelay, int tDecay);

    void setLPComb(int Combnum, int tDelay, int tDecay1, int tDecay2);

private:
    int mix; //ratio of Wet / Dry signals
    LPComb LPC1;
    LPComb LPC2;
    LPComb LPC3;
    LPComb LPC4;
    LPComb LPC5;
    LPComb LPC6;
    Allpass AP;
};

#endif