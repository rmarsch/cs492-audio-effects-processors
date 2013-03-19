/*
Filters.cpp

Definitions of the Filter classes
Allpass filter
Comb filter
Low-Passed Comb filter
*/

#include "Filters.h"
#include "AudioHandler.h"



//Global variable used
const double fsPerMs = (AudioHandler::fs / 1000.0);

//***************** Allpass Filter ***********************************************
int Allpass::MAX_MS_DELAY = 5000; //5 seconds
int Allpass::MAX_BUFFER_LENGTH = static_cast<int>((Allpass::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);

Allpass::~Allpass(){
    destroyDelayBuffer();
}
Allpass::Allpass(){
    delay = 3995;
    decay = 50;
    writePtr = 0;
    delayPtr = (int) (-1) * (fsPerMs * delay);
    bufferLength = 2 + ((delay / (1.0 * MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);

    initializeDelayBuffer();
}

//initializes the delayBuffer into an array of proper size
void Allpass::initializeDelayBuffer(){
    destroyDelayBuffer();

    delayBuffer = new double[bufferLength];
}

//destroys the current delay buffer
void Allpass::destroyDelayBuffer(){
    delete[ ] delayBuffer;
    delayBuffer = 0; //null cast
}

//Computation functions
int Allpass::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void Allpass::tick(void *outputBuffer, void *input,int nBufferFrames){
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

StkFrames& Allpass::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        computeSample(frames[i]); //Frames[i] is passed by reference
    }

    return frames;
}

double Allpass::computeSample(double& in){   
    if(delayPtr >= bufferLength)
            delayPtr %= bufferLength;
    if(writePtr >= bufferLength)
            writePtr %= bufferLength;
    
    double inputSample = in;

    //Compute outputs
    if(delayPtr >= 0){
        double delayComponent = delayBuffer[delayPtr];
        double feedbackComponent = delayComponent * (decay / 100.0);
        double firstSumComponent = in;
        firstSumComponent += feedbackComponent;
        
        //Feed firstSumComponent into the delay buffer
        delayBuffer[writePtr] = firstSumComponent;
        //scale sum to pass to second sum component
        firstSumComponent *= (-decay / 100.0);

        //Compute output sample
        in = firstSumComponent + delayComponent;
    }
    else{
        //fill initial delay buffer values
        delayBuffer[writePtr] = in;
        //compute output
        in *= (-decay / 100.0);
    }
    //Limiting
    if(in >= 1.0)
        in = 0.9999;
    else if(in <= -1.0)
        in = -0.9999;

    //Update pointers
    writePtr++;
    delayPtr++;

    return in;
}

void Allpass::setAllpass(int tDelay, int tDecay){
    delay = tDelay;
    decay = tDecay;

    if(delay > MAX_MS_DELAY || delay < 0)
        delay = MAX_MS_DELAY / 2;
    if(decay >= 100 || decay < 0)
        decay = 50;

    writePtr = 0;
    delayPtr = (int) (-1) * (fsPerMs * delay);
    bufferLength = 2 + ((delay / (1.0 * MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);

    initializeDelayBuffer();
}

//******************* Comb Filter ************************************************
int Comb::MAX_MS_DELAY = 50; //50 ms
int Comb::MAX_BUFFER_LENGTH = static_cast<int>((Comb::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);

Comb::~Comb(){
    destroyDelayBuffer();
}
Comb::Comb(){
    decay = 50;
    delay = 35;

    writePtr = 0;
    delayPtr = (int) (-1) * (fsPerMs * delay);
    bufferLength = 2 +  ((delay / (1.0 * MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);

    initializeDelayBuffer();
}

//initializes the delayBuffer into an array of proper size
void Comb::initializeDelayBuffer(){
    destroyDelayBuffer();

    delayBuffer = new double[bufferLength];
}

//destroys the current delay buffer
void Comb::destroyDelayBuffer(){
    delete[ ] delayBuffer;
    delayBuffer = 0; //null cast
}

//Computation functions
int Comb::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void Comb::tick(void *outputBuffer, void *input,int nBufferFrames){
    register StkFloat *out = (StkFloat *) outputBuffer;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        *out = computeSample(frames[i]);
        out++; //Move to next sample
    }
}

StkFrames& Comb::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        computeSample(frames[i]); //Passed by reference
    }

    return frames;
}

double Comb::computeSample(double& in){
    if(delayPtr >= bufferLength)
            delayPtr %= bufferLength;
    if(writePtr >= bufferLength)
            writePtr %= bufferLength;

    //Compute outputs
    if(delayPtr >= 0){
        //build sum point
        double sumComponent = in;
        double delayComponent = delayBuffer[delayPtr];
        delayComponent *= (decay / 100.0);
        sumComponent += delayComponent;

        //Pass in new value to delay
        delayBuffer[writePtr] = sumComponent;

        //compute output
        in = sumComponent * (-decay / 100.0); 
    }
    else{
        //fill delay
        delayBuffer[writePtr] = in;
        //delay unit has nothing to ouput
        in *= (-decay / 100.0);
    }

    //Limiting
    if(in >= 1.0)
        in = 0.9999;
    else if(in <= -1.0)
        in = -0.9999;

    //Update pointers
    writePtr++;
    delayPtr++;

    return in;
}


void Comb::setComb(int tDelay, int tDecay){
    decay = tDecay;
    delay = tDelay;

    if(delay > MAX_MS_DELAY || delay < 0)
        delay = MAX_MS_DELAY / 2;
    if(decay >= 100 || decay < 0)
        decay = 50;

    writePtr = 0;
    delayPtr = (int) (-1) * (fsPerMs * delay);
    bufferLength = 2 +  ((delay / (1.0 * MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);

    initializeDelayBuffer();
}

//************Low Pass Comb Filter ************************************************
int LPComb::MAX_MS_DELAY = 50; //50 ms
int LPComb::MAX_BUFFER_LENGTH = static_cast<int>((LPComb::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);

LPComb::~LPComb(){
    destroyDelayBuffer();
}
LPComb::LPComb(){
    decay1 = 45;
    decay2 = 55;
    delay = 35;

    writePtr = 0;
    delayPtr = (int) (-1) * (fsPerMs * delay);
    delayPtrN1 = delayPtr - 1; //one sample back
    bufferLength = 3 + ((delay / (1.0 * MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);

    initializeDelayBuffer();
}

//initializes the delayBuffer into an array of proper size
void LPComb::initializeDelayBuffer(){
    destroyDelayBuffer();

    delayBuffer = new double[bufferLength];
}

//destroys the current delay buffer
void LPComb::destroyDelayBuffer(){
    delete[ ] delayBuffer;
    delayBuffer = 0; //null cast
}

//Computation functions
int LPComb::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void LPComb::tick(void *outputBuffer, void *input,int nBufferFrames){
    register StkFloat *out = (StkFloat *) outputBuffer;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        *out = computeSample(frames[i]);
        out++; //move to next sample
    }
}

StkFrames& LPComb::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        computeSample(frames[i]); //passed by reference
    }

    return frames;
}

double LPComb::computeSample(double& in){
    if(delayPtr >= bufferLength)
            delayPtr %= bufferLength;
    if(delayPtrN1 >= bufferLength)
            delayPtrN1 %= bufferLength;
    if(writePtr >= bufferLength)
            writePtr %= bufferLength;

    //Compute outputs
    if(delayPtr >= 0){
        double sumComponent;
        
        //off-by-one when delayPtr = 0 and delayPtrN1 = -1
        if(delayPtrN1 >=0){
            //add input to sum
            sumComponent = in;
            //compute delay feedback into sum
            double delayComponent = delayBuffer[delayPtrN1] * (decay2 / 100.0);
            delayComponent += delayBuffer[delayPtr];
            delayComponent *= (decay1 / 100.0);
            sumComponent += delayComponent;

            //update delay buffer
            delayBuffer[writePtr] = sumComponent;

            //compute output
            in = sumComponent * (-decay1 / 100.0);
        }
        else{
            //add input to sum
            sumComponent = in;
            //compute delay feedback into sum
            double delayComponent = delayBuffer[delayPtr];
            delayComponent *= (decay1 / 100.0);
            sumComponent += delayComponent;

            //update delay buffer
            delayBuffer[writePtr] = sumComponent;

            //compute output
            in = sumComponent * (-decay1 / 100.0);
        }
    }
    else{
        //fill delay
        delayBuffer[writePtr] = in;
        //delay unit has nothing to ouput
        in *= (-decay1 / 100.0);
    }

    //Limiting
    if(in >= 1.0)
        in = 0.9999;
    else if(in <= -1.0)
        in = -0.9999;

    //Update pointers
    writePtr++;
    delayPtr++;
    delayPtrN1++;

    return in;
}

void LPComb::setLPComb(int tDelay, int tDecay1, int tDecay2){ //implicitly defines delay2
    decay1 = tDecay1;
    decay2 = tDecay2;
    delay = tDelay;

    if(delay > MAX_MS_DELAY || delay < 0)
        delay = MAX_MS_DELAY / 2;
    if(decay1 >= 100 || decay1 < 0)
        decay1 = 50;
    if(decay2 >= 100 || decay2 < 0)
        decay2 = 50;

    writePtr = 0;
    delayPtr = (int) (-1) * (fsPerMs * delay );
    delayPtrN1 = delayPtr - 1; //one sample back
    bufferLength = 3 + ((delay / (1.0 * MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);

    initializeDelayBuffer();
}