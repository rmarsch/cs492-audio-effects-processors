/* 
Definitions for three types of Delay classes
SingleDelay
DoubleDelay
FeedbackDelay

Coded by Richard Marscher for use with the Synthesis ToolKit C++ Libraries
*/

#include "AudioHandler.h"
#include "Delays.h"

using std::cout;
using std::cin;
using std::endl;

//Global variable used
const double fsPerMs = (AudioHandler::fs / 1000.0);

//Define destructor of Delay
Delay::~Delay(){}

//***** Single Delay Definitions ***********************
int SingleDelay::MAX_MS_DELAY = 1000; //1000 ms
int SingleDelay::MAX_BUFFER_LENGTH = static_cast<int>((SingleDelay::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);

SingleDelay::~SingleDelay(){
    destroyDelayBuffer();
}

SingleDelay::SingleDelay(){
    dry = 0.9;
    wet = 0.5;
    delay = 200;
    bufferCell = 0;
    bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}

void SingleDelay::tick(void *output, void *input, int nBufferFrames){
    register StkFloat *out = (StkFloat *) output;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        if(bufferCell >= bufferLength)
            bufferCell %= bufferLength;

        delayBuffer[bufferCell++] = frames[i];
        //If delay milliseconds have passed since the delay buffer was initialized then add in delay
        if(delayCell >= 0){
            *out = (frames[i] * dry) + (delayBuffer[delayCell % bufferLength] * wet);  //Sum the current input and the correct delay buffer cell
        }
        else{
            *out = (frames[i] * dry);
        }

        //limit signal
        if(*out > 1.0)
            *out = 0.999;
        else if(frames[i] < -1.0)
            *out = -0.999;

        out++;
        delayCell++;
    }
}

StkFrames& SingleDelay::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        if(bufferCell >= bufferLength)
            bufferCell %= bufferLength;

        delayBuffer[bufferCell++] = frames[i];
        //If delay milliseconds have passed since the delay buffer was initialized then add in delay
        if(delayCell >= 0){
            frames[i] = (frames[i] * dry) + (delayBuffer[delayCell % bufferLength] * wet);  //Sum the current input and the correct delay buffer cell
        }
        else{
            frames[i] = (frames[i] * dry);
        }

        if(frames[i] > 1.0)
            frames[i] = 0.999;
        else if(frames[i] < -1.0)
            frames[i] = -0.999;

        delayCell++;
    }

    return frames;
}

int SingleDelay::callback(void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void SingleDelay::setSingleDelay(double tDry, double tWet, unsigned int tDelay){
    if((tDry >= 0.0) && (tDry <= 1.0) && (tWet >= 0.0) && (tWet <= 1.0) && (tDelay >= 0) && (tDelay <= 1000)){
        dry = tDry;
        wet = tWet;
        delay = tDelay;
        bufferCell = 0;
        delayCell = (int) (-1) * (fsPerMs * delay);
        bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

        initializeDelayBuffer();
    }
    //If for some case invalid parameters are passed, initialize to default
    else
        setSingleDelay();
}
void SingleDelay::setSingleDelay(){
    dry = 1.0;
    wet = 0.0;
    delay = 200;
    bufferCell = 0;
    delayCell = (int) (-1) * (fsPerMs * delay);
    bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}
void SingleDelay::setDry(double tDry){
    if(tDry >= 0.0 && tDry <= 1.0)
        dry = tDry;
}
void SingleDelay::setWet(double tWet){
    if(tWet >= 0.0 && tWet <= 1.0)
        wet = tWet;
}
void SingleDelay::setDelay(unsigned int tDelay){
    if(tDelay >= 0 && tDelay <= 1000)
        delay = tDelay;

    bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}

//initializes the delayBuffer into an array of proper size
void SingleDelay::initializeDelayBuffer(){
    destroyDelayBuffer();

    delayBuffer = new double[bufferLength];
}
//destroys the current delay buffer
void SingleDelay::destroyDelayBuffer(){
    delete[ ] delayBuffer;
    delayBuffer = 0; 
}


//*********** Double Delay Definitions ******************
int DoubleDelay::MAX_MS_DELAY = 1000; //1000 ms
int DoubleDelay::MAX_BUFFER_LENGTH = static_cast<int>((DoubleDelay::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);

DoubleDelay::~DoubleDelay(){
    destroyDelayBuffer();
}

DoubleDelay::DoubleDelay(){
    dry = 1.0;
    wet1 = 0.3;
    wet2 = 0.3;
    delay1 = 100;
    delay2 = 200;
    bufferCell = 0;
    bufferLength = 2 + (delay2 / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}

void DoubleDelay::tick(void *output, void *input, int nBufferFrames){
    register StkFloat *out = (StkFloat *) output;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        if(bufferCell >= bufferLength)
            bufferCell %= bufferLength;

        delayBuffer[bufferCell++] = frames[i];
        //If delay1 milliseconds have passed since the delay buffer was initialized then add in delay1
        if(delayCell1 >= 0){
            //If delay2 milliseconds have passed since the delay buffer was initialized then add in delay2
            if(delayCell2 >= 0){
                *out = (frames[i] * dry) + 
                    (delayBuffer[delayCell1 % bufferLength] * wet1) +
                    (delayBuffer[delayCell2 % bufferLength] * wet2);  //Sum the current input and the correct delay buffer cells

            }
            //Delay2 is not ready yet
            else{
                *out = (frames[i] * dry) + (delayBuffer[delayCell1 % bufferLength] * wet1);
            }
        }
        else{
            *out = (frames[i] * dry);
        }

        //limit signal
        if(*out > 1.0)
            *out = 0.999;
        else if(frames[i] < -1.0)
            *out = -0.999;

        out++;
        delayCell1++;
        delayCell2++;
    }
}

StkFrames& DoubleDelay::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){
        if(bufferCell >= bufferLength)
            bufferCell %= bufferLength;

        delayBuffer[bufferCell++] = frames[i];
        //If delay1 milliseconds have passed since the delay buffer was initialized then add in delay1
        if(delayCell1 >= 0){
            //If delay2 milliseconds have passed since the delay buffer was initialized then add in delay2
            if(delayCell2 >= 0){
                frames[i] = (frames[i] * dry) + 
                    (delayBuffer[delayCell1 % bufferLength] * wet1) +
                    (delayBuffer[delayCell2 % bufferLength] * wet2);  //Sum the current input and the correct delay buffer cells

            }
            //Delay2 is not ready yet
            else{
                frames[i] = (frames[i] * dry) + (delayBuffer[delayCell1 % bufferLength] * wet1);
            }
        }
        else{
            frames[i] = (frames[i] * dry);
        }

        if(frames[i] > 1.0)
            frames[i] = 0.999;
        else if(frames[i] < -1.0)
            frames[i] = -0.999;

        delayCell1++;
        delayCell2++;
    }

    return frames;
}

int DoubleDelay::callback(void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

void DoubleDelay::setDoubleDelay(){
    dry = 1.0;
    wet1 = 0.0;
    wet2 = 0.0;
    delay1 = 200;
    delay2 = 400;
    bufferCell = 0;
    delayCell1 = (int) (-1) * (fsPerMs * delay1);
    delayCell2 = (int) (-1) * (fsPerMs * delay2);

    bufferLength = 2 + (delay2 / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}
void DoubleDelay::setDoubleDelay(double tDry, double tWet1, double tWet2, unsigned int tDelay1, unsigned int tDelay2){
    if((tDry >= 0.0) && (tDry <= 1.0) && (tWet1 >= 0.0) && (tWet1 <= 1.0) 
        && (tWet2 >= 0.0) && (tWet2 <= 1.0) && (tDelay1 >= 0) && (tDelay1 <= 1000) 
        && (tDelay2 >= 0) && (tDelay2 <= 1000)){
        dry = tDry;
        wet1 = tWet1;

        //Enforce delay1 is shorter than delay2
        if(tDelay1 > tDelay2){
            delay1 = tDelay2;
            delay2 = tDelay1;
        }
        else{
            delay1 = tDelay1;
            delay2 = tDelay2;
        }
        
        bufferCell = 0;
        delayCell1 = (int) (-1) * (fsPerMs * delay1);
        delayCell2 = (int) (-1) * (fsPerMs * delay2);
        bufferLength = 2 + (delay2 / 1000.0) * MAX_BUFFER_LENGTH; 

        initializeDelayBuffer();
    }
    //If for some case invalid parameters are passed, initialize to default
    else
        DoubleDelay();
}

void DoubleDelay::setDry(double tDry){
    if(tDry >= 0.0 && tDry <= 1.0)
        dry = tDry;
}
void DoubleDelay::setWet1(double tWet1){
    if(tWet1 >= 0.0 && tWet1 <= 1.0)
        wet1 = tWet1;
}
void DoubleDelay::setDelay1(unsigned int tDelay1){
    if(tDelay1 >= 0 && tDelay1 <= 1000)
        delay1 = tDelay1;
}
void DoubleDelay::setWet2(double tWet2){
    if(tWet2 >= 0.0 && tWet2 <= 1.0)
        wet2 = tWet2;
}
void DoubleDelay::setDelay2(unsigned int tDelay2){
    if(tDelay2 >= 0 && tDelay2 <= 1000)
        delay2 = tDelay2;

    bufferLength = 2 + (delay2 / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}

//initializes the delayBuffer into an array of proper size
void DoubleDelay::initializeDelayBuffer(){
    destroyDelayBuffer();

    delayBuffer = new double[bufferLength];
}
//destroys the current delay buffer
void DoubleDelay::destroyDelayBuffer(){
    delete[ ] delayBuffer;
    delayBuffer = 0; 
}

//************ Feedback Delay Definitions ******************
int FeedbackDelay::MAX_MS_DELAY = 1000; //1000 ms
int FeedbackDelay::MAX_BUFFER_LENGTH = static_cast<int>((FeedbackDelay::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);

FeedbackDelay::~FeedbackDelay(){
    destroyDelayBuffer();
}
FeedbackDelay::FeedbackDelay(){
    gain = 1.0;
    decay = 60;
    delay = 100;
    bufferCell = 0;
    bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}

void FeedbackDelay::tick(void *output, void *input,int nBufferFrames){
    register StkFloat *out = (StkFloat *) output;
    FileWvIn *in = (FileWvIn *) input;
    
    StkFrames frames;
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){

        //If delay milliseconds have passed since the delay buffer was initialized then add in delay
        if(delayCell >= 0){
            double summedSignal = (frames[i] + (delayBuffer[delayCell % bufferLength] * (decay/100.0))); //Compute the signal at the sum point
            *out = (summedSignal * gain);  //Sum the current input and the correct delay buffer cell

            if(bufferCell >= bufferLength)
                bufferCell %= bufferLength;
            delayBuffer[bufferCell++] = summedSignal;
        }
        //Delay has not started yet. Sum is simplified to just *in.
        else{
            *out = (frames[i] * gain);
             
            if(bufferCell >= bufferLength)
                bufferCell %= bufferLength;
            delayBuffer[bufferCell++] = frames[i];
        }

        //limit signal
        if(*out > 1.0)
            *out = 0.999;
        else if(frames[i] < -1.0)
            *out = -0.999;

        out++;
        delayCell++;
    }
}

StkFrames& FeedbackDelay::tick(void *input, int nBufferFrames, StkFrames& frames){
    FileWvIn *in = (FileWvIn *) input;
    
    unsigned int channels = AudioHandler::nChannels;
    frames.resize( nBufferFrames, channels );
    
    in->tickFrame( frames );

    for(unsigned int i = 0; i<frames.size(); i++){

        //If delay milliseconds have passed since the delay buffer was initialized then add in delay
        if(delayCell >= 0){
            double summedSignal = (frames[i] + (delayBuffer[delayCell % bufferLength] * (decay/100.0))); //Compute the signal at the sum point
            frames[i] = (summedSignal * gain);  //Sum the current input and the correct delay buffer cell
           
            if(bufferCell >= bufferLength)
                bufferCell %= bufferLength;
            delayBuffer[bufferCell++] = summedSignal;
        }
        //Delay has not started yet. Sum is simplified to just *in.
        else{
            frames[i] = (frames[i] * gain);
             
            if(bufferCell >= bufferLength)
                bufferCell %= bufferLength;
            delayBuffer[bufferCell++] = frames[i];
        }

        if(frames[i] > 1.0)
            frames[i] = 0.999;
        else if(frames[i] < -1.0)
            frames[i] = -0.999;

        delayCell++;
    }

    return frames;
}


int FeedbackDelay::callback(void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData){
    FileWvIn *input = (FileWvIn *) userData;
        
    tick(outputBuffer, input, nBufferFrames); 

    if ( input->isFinished() ) {
        AudioHandler::done = true;
        return 1;
    }
    else
        return 0;
}

//Set functions
void FeedbackDelay::setFeedbackDelay(){
    gain = 1.0;
    decay = 10;
    delay = 200;
    bufferCell = 0;
    delayCell = (int) (-1) * (fsPerMs * delay);

    bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}

void FeedbackDelay::setFeedbackDelay(double tGain, double tDecay, unsigned int tDelay){
    if((tGain >= 0.0) && (tGain <= 2.0) && (tDecay >= 0) && (tDecay < 100) && (tDelay >= 0) && (tDelay <= 1000)){
        gain = tGain;
        decay = tDecay;
        delay = tDelay;
        bufferCell = 0;
        delayCell = (int) (-1) * (fsPerMs * delay);
        bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

        initializeDelayBuffer();
    }
    //If for some case invalid parameters are passed, initialize to default
    else
        FeedbackDelay();
}

void FeedbackDelay::setGain(double tGain){
    if(tGain >= 0.0 && tGain <= 2.0)
        gain = tGain;
}
void FeedbackDelay::setDecay(double tDecay){
    if(tDecay >= 0 && tDecay < 100)
        decay = tDecay;
}
void FeedbackDelay::setDelay(unsigned int tDelay){
    if(tDelay >= 0 && tDelay <= 1000)
        delay = tDelay;

    bufferLength = 2 + (delay / 1000.0) * MAX_BUFFER_LENGTH; 

    initializeDelayBuffer();
}


void FeedbackDelay::initializeDelayBuffer(){
    destroyDelayBuffer();

    delayBuffer = new double[bufferLength];
}
//destroys the current delay buffer
void FeedbackDelay::destroyDelayBuffer(){
    delete[ ] delayBuffer;
    delayBuffer = 0; 
}