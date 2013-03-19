/*
Reverb.cpp

Definitions for 3 Reverberation units
Reverb 1: 5 Allpass filters in series
Reverb 2: 4 Comb filters in parallel into two Allpass filters in series
Reverb 3: 6 Low-Passed Comb filters in parallel into one Allpass filter
*/

#include "Reverb.h"
#include "FileWvIn.h"
#include "AudioHandler.h"

//static variables
int Reverb1::MAX_MS_DELAY = 5000; //5 seconds
int Reverb2::A_MAX_MS_DELAY = 5000; //5 seconds
int Reverb2::C_MAX_MS_DELAY = 50; //50ms
int Reverb3::C_MAX_MS_DELAY = 50; //50ms
int Reverb3::A_MAX_MS_DELAY = 5000; //5 seconds


Reverb1::~Reverb1(){
    AP1.~Allpass();
    AP2.~Allpass();
    AP3.~Allpass();
    AP4.~Allpass();
    AP5.~Allpass();
}
Reverb1::Reverb1(){
    setAP(1, 3500, 68);
    setAP(2, 3495, 50);
    setAP(3, 3480, 40);
    setAP(4, 3505, 50);
    setAP(5, 3520, 40);
    setMix(50);
}

//Computation functions
int Reverb1::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void Reverb1::tick(void *outputBuffer, void *input,int nBufferFrames){
    register StkFloat *out = (StkFloat *) outputBuffer;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        *out = computeSample(frames[i]); //assign output sample
        out++; //move to next output sample
    }
}

StkFrames& Reverb1::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        computeSample(frames[i]); //Frames[i] is passed by reference
    }

    return frames;
}

double Reverb1::computeSample(double& in){
    double inComponent = ((100 - mix) / 100.0) * in; //store and adjust dry signal
    
    //Sequentially pass the input value
    AP1.computeSample(in);
    AP2.computeSample(in);
    AP3.computeSample(in);
    AP4.computeSample(in);
    AP5.computeSample(in);

    in *= (mix / 100.0); //adjust the wet signal

    in += inComponent; //compute final output

    //limit output
    if(in >= 1.0)
        in = 0.9999;
    else if(in <= -1.0)
        in = -0.9999;

    return in;
}

void Reverb1::setMix(int tMix){
    mix = tMix;

    if(mix > 100 || mix < 0)
        mix = 50;
}

void Reverb1::setAP(int APnum, int tDelay, int tDecay){
    switch(APnum){
        case 1:
            AP1.setAllpass(tDelay, tDecay);
            break;
        case 2:
            AP2.setAllpass(tDelay, tDecay);
            break;
        case 3:
            AP3.setAllpass(tDelay, tDecay);
            break;
        case 4:
            AP4.setAllpass(tDelay, tDecay);
            break;
        case 5:
            AP5.setAllpass(tDelay, tDecay);
            break;
        default:
            AP1.setAllpass(tDelay, tDecay);
    }
}


Reverb2::~Reverb2(){
    C1.~Comb();
    C2.~Comb();
    C3.~Comb();
    C4.~Comb();
    AP1.~Allpass();
    AP2.~Allpass();
}
Reverb2::Reverb2(){
    setComb(1, 32, 50);
    setComb(2, 33, 51);
    setComb(3, 34, 52);
    setComb(4, 35, 53);
    setAP(1, 4995, 65);
    setAP(2, 3995, 50);
    setMix(50);
}

//Computation functions
int Reverb2::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void Reverb2::tick(void *outputBuffer, void *input,int nBufferFrames){
    register StkFloat *out = (StkFloat *) outputBuffer;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        *out = computeSample(frames[i]); //assign output sample
        out++; //move to next output sample
    }
}

StkFrames& Reverb2::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        computeSample(frames[i]); //Frames[i] is passed by reference
    }

    return frames;
}

double Reverb2::computeSample(double& in){
    //Compute parallel values for comb filters
    
    double comb1 = in;

    C1.computeSample(comb1);

    double comb2 = in;

    C2.computeSample(comb2);

    double comb3 = in;

    C3.computeSample(comb3);

    double comb4 = in;

    C4.computeSample(comb4);

    //Compute input component
    double inComponent = in * ((100 - mix) / 100.0);

    //Sum the parallel comb values
    double combComponent = comb1 + comb2 + comb3 + comb4;

    //Sequentially pass parallel comb values through 2 allpass filters
    AP1.computeSample(combComponent);
    AP2.computeSample(combComponent);

    //Adjust wet signal by mix ratio
    combComponent *= (mix / 100.0);

    //designate output value
    in = inComponent + combComponent;

    //limit output
    if(in >= 1.0)
        in = 0.9999;
    else if(in <= -1.0)
        in = -0.9999;

    return in;
}

void Reverb2::setMix(int tMix){
    mix = tMix;

    if(mix > 100 || mix < 0)
        mix = 50;
}

void Reverb2::setAP(int APnum, int tDelay, int tDecay){
    switch(APnum){
        case 1:
            AP1.setAllpass(tDelay, tDecay);
            break;
        case 2:
            AP2.setAllpass(tDelay, tDecay);
            break;
        default:
            AP1.setAllpass(tDelay, tDecay);
    }
}

void Reverb2::setComb(int Combnum, int tDelay, int tDecay){
    switch(Combnum){
        case 1:
            C1.setComb(tDelay, tDecay);
            break;
        case 2:
            C2.setComb(tDelay, tDecay);
            break;
        case 3:
            C3.setComb(tDelay, tDecay);
            break;
        case 4:
            C4.setComb(tDelay, tDecay);
            break;
        default:
            C1.setComb(tDelay, tDecay);
    }
}

Reverb3::~Reverb3(){
    LPC1.~LPComb();
    LPC2.~LPComb();
    LPC3.~LPComb();
    LPC4.~LPComb();
    LPC5.~LPComb();
    LPC6.~LPComb();
    AP.~Allpass();
}
Reverb3::Reverb3(){
    setLPComb(1, 30, 40, 35);
    setLPComb(2, 31, 41, 36);
    setLPComb(3, 32, 42, 37);
    setLPComb(4, 33, 43, 38);
    setLPComb(5, 34, 44, 39);
    setLPComb(6, 35, 45, 40);
    setAP(3500, 30);
    setMix(50);
}

//Computation functions
int Reverb3::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void Reverb3::tick(void *outputBuffer, void *input,int nBufferFrames){
    register StkFloat *out = (StkFloat *) outputBuffer;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        *out = computeSample(frames[i]); //assign output sample
        out++; //move to next output sample
    }
}

StkFrames& Reverb3::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        computeSample(frames[i]); //Frames[i] is passed by reference
    }

    return frames;
}

double Reverb3::computeSample(double& in){
    //Compute parallel Low-Pass Comb filters

    double comb1 = in;
    
    LPC1.computeSample(comb1);

    double comb2 = in;

    LPC2.computeSample(comb2);

    double comb3 = in;

    LPC3.computeSample(comb3);

    double comb4 = in;

    LPC4.computeSample(comb4);

    double comb5 = in;

    LPC5.computeSample(comb5);

    double comb6 = in;

    LPC6.computeSample(comb6);

    //Sum the parallel comb values
    double combComponent = comb1 + comb2 + comb3 + comb4 + comb5 + comb6;

    //limit the combComponent to clean up sound
    if(combComponent >= 1.0)
        combComponent = 0.9999;
    else if(combComponent <= -1.0)
        combComponent = -0.9999;

    //Pass through Allpass filter
    AP.computeSample(combComponent);

    //Adjust wet signal by mix
    combComponent *= (mix / 100.0);

    //Compute in component
    double inComponent = in * ((100 - mix) / 100.0);

    in = inComponent + combComponent;

    //limit output
    if(in >= 1.0)
        in = 0.9999;
    else if(in <= -1.0)
        in = -0.9999;

    return in;
}

void Reverb3::setMix(int tMix){
    mix = tMix;

    if(mix > 100 || mix < 0)
        mix = 50;
}

void Reverb3::setAP(int tDelay, int tDecay){
    AP.setAllpass(tDelay, tDecay);
}

void Reverb3::setLPComb(int Combnum, int tDelay, int tDecay1, int tDecay2){
    switch(Combnum){
        case 1:
            LPC1.setLPComb(tDelay, tDecay1, tDecay2);
            break;
        case 2:
            LPC2.setLPComb(tDelay, tDecay1, tDecay2);
            break;
        case 3:
            LPC3.setLPComb(tDelay, tDecay1, tDecay2);
            break;
        case 4:
            LPC4.setLPComb(tDelay, tDecay1, tDecay2);
            break;
        case 5:
            LPC5.setLPComb(tDelay, tDecay1, tDecay2);
            break;
        case 6:
            LPC6.setLPComb(tDelay, tDecay1, tDecay2);
            break;
        default:
            LPC1.setLPComb(tDelay, tDecay1, tDecay2);
    }
}