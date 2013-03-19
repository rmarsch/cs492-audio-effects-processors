/*
Definitions for static variables and member functions of the classes:
MultiChorus, FeedbackChorus, ChorusUnit, and Modulator
*/

#include "AudioHandler.h"
#include "Chorus.h"
#include "Interpolation.h"
#include <math.h>
#include <cmath>

using std::cout;
using std::cin;
using std::endl;
using Interpolation::sincBand;
using Interpolation::linInterp;

//Declare M_PI for Bullshit VS2008 Error
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif 

//Global variable used
const double fsPerMs = (AudioHandler::fs / 1000.0);

//Definition for Destructor necessary
Chorus::~Chorus(){}

    MultiChorus::~MultiChorus(){
        destroyDelayBuffer();
        mod1.~Modulator();
        mod2.~Modulator();
        mod3.~Modulator();
    }
    
    //Default Constructor
    MultiChorus::MultiChorus(){
        dry = 50;
        wet = 50;
        delay1 = 100;
        delay2 = 200;
        delay3 = 400;
        numDelays = 3;
        //mod1.setModulator();
        numModulators = 1;
        bufferLength = 2 + (delay3 / 1000.0) * MAX_BUFFER_LENGTH;
        writeCell = 0;

        initializeDelayBuffer();
    }

    //Main Setter
    void MultiChorus::setMultiChorus(int tDry, int tWet, int tDelay1, int tDelay2, int tDelay3,
        int tNumDelays, int tNumModulators, bool bandlimited){
        if(tDry >= 0 && tDry <= 100)
            dry = tDry;
        else
            dry = 50;

        if(tWet >= 0 && tWet <= 100)
            wet = tWet;
        else
            wet = 50;

        if(tDelay1 > 0 && tDelay1 <= MAX_MS_DELAY)
            delay1 = tDelay1;
        else
            delay1 = 100;

        if(tDelay2 > 0 && tDelay2 <= MAX_MS_DELAY)
            delay2 = tDelay2;
        else
            delay2 = 200;

        if(tDelay3 > 0 && tDelay3 <= MAX_MS_DELAY)
            delay3 = tDelay3;
        else
            delay3 = 400;

        if(tNumDelays > 0 && tNumDelays <= MAX_DELAYS)
            numDelays = tNumDelays;
        else
            numDelays = MAX_DELAYS;

        if(tNumModulators > 0 && tNumModulators <= Modulator::MAX_MODS
            && tNumModulators <= numDelays)
            numModulators = tNumModulators;
        else
            numModulators = 1;

        //Initialize all the necessary modulators
        switch(numModulators){
            case 1:
                mod1.setModulator();
                break;
            case 2:
                mod1.setModulator();
                mod2.setModulator();
                break;
            case 3:
                mod1.setModulator();
                mod2.setModulator();
                mod3.setModulator();
                break;
            default:
                mod1.setModulator();
        }
    
        //Prioritize delay orderings
        if(delay1 > delay3 && delay3 != 0){
            int tDelay = delay1;
            delay1 = delay3;
            delay3 = tDelay; 
        }
        if(delay2 > delay3 && delay3 != 0){
            int tDelay = delay2;
            delay2 = delay3;
            delay3 = tDelay;
        }
        if(delay1 > delay2 && delay2 != 0){
            int tDelay = delay1;
            delay1 = delay2;
            delay2 = tDelay; 
        }


        //Define the buffer size
        switch(numDelays){
            case 1:
                bufferLength = 2 + static_cast<int>((delay1 / (1.0 *MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);
                break;
            case 2:
                bufferLength = 2 + static_cast<int>((delay2 / (1.0 *MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);
                break;
            case 3:
                bufferLength = 2 + static_cast<int>((delay3 / (1.0 *MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);
                break;
            default:
                bufferLength = 2 + static_cast<int>((delay1 / (1.0 *MAX_MS_DELAY)) * MAX_BUFFER_LENGTH);
        }

        initializeDelayBuffer();

        delayCell1 = (int) (-1) * (fsPerMs * delay1);
        delayCell2 = (int) (-1) * (fsPerMs * delay2);
        delayCell3 = (int) (-1) * (fsPerMs * delay3);
        writeCell = 0;

        if(bandlimited){
            isBandlimited = true;
        }
    }

    //initializes the delayBuffer into an array of proper size
    void MultiChorus::initializeDelayBuffer(){
        destroyDelayBuffer();

        delayBuffer = new double[bufferLength];
     }
    //destroys the current delay buffer
    void MultiChorus::destroyDelayBuffer(){
        delete[ ] delayBuffer;
        delayBuffer = 0; 
    }

    int MultiChorus::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
        FileWvIn *input = (FileWvIn *) userData;
        
        tick(outputBuffer, input, nBufferFrames); 

        if ( input->isFinished() ) {
            AudioHandler::done = true;
            return 1;
        }
        else
            return 0;
    }
    //real time overload
    void MultiChorus::tick(void *outputBuffer, void *input,int nBufferFrames){
        register StkFloat* out = (StkFloat *) outputBuffer;

        FileWvIn *in = (FileWvIn *) input;
        
        StkFrames frames;
        unsigned int channels = AudioHandler::nChannels;
        frames.resize( nBufferFrames, channels );
        
        in->tickFrame( frames );

        for(unsigned int i = 0; i<frames.size(); i++){

            //If delay milliseconds have passed since the delay buffer was initialized then add in delay
            if(delayCell1 >= 0){

                //********************VARY THE DELAY TIME ********************************
                //************************************************************************
                double factor1 = 1, factor2 = 1, factor3 = 1; //the factors by which to vary delays
    
                /*Modulate the delay length*/

                //***************CASE : ONE MODULATOR *************
                if(numModulators == 1){
                    factor1 = mod1.nextCoefficient();
                    factor2 = factor1;
                    factor3 = factor1;

                    double tmpDelay = delay1 * factor1;
                    delayCell1 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell1 < 0){
                        delayCell1 += bufferLength;
                    }
                    else if(delayCell1 > bufferLength){
                        while(delayCell1 > bufferLength){
                            delayCell1 -= bufferLength;
                        }
                    }

                    if(numDelays >= 2){
                        tmpDelay = delay2 * factor1;
                        delayCell2 = writeCell - tmpDelay;
    
                        //Conditionals to keep delayCell in bounds
                        if(delayCell2 < 0){
                            delayCell2 += bufferLength;
                        }
                        else if(delayCell2 > bufferLength){
                            while(delayCell2 > bufferLength){
                                delayCell2 -= bufferLength;
                            }
                        }
                    }
                    if(numDelays >= 3){
                        tmpDelay = delay3 * factor1;
                        delayCell3 = writeCell - tmpDelay;
    
                        //Conditionals to keep delayCell in bounds
                        if(delayCell3 < 0){
                            delayCell3 += bufferLength;
                        }
                        else if(delayCell3 > bufferLength){
                            while(delayCell3 > bufferLength){
                                delayCell3 -= bufferLength;
                            }
                        }
                    }
                }
                //*************CASE: TWO MODULATORS *********************
                else if(numModulators == 2){
                    factor1 = mod1.nextCoefficient();
                    factor3 = factor1;

                    double tmpDelay = delay1 * factor1;
                    delayCell1 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell1 < 0){
                        delayCell1 += bufferLength;
                    }
                    else if(delayCell1 > bufferLength){
                        while(delayCell1 > bufferLength){
                            delayCell1 -= bufferLength;
                        }
                    }

                    factor2 = mod2.nextCoefficient();
                    tmpDelay = delay2 * factor2;
                    delayCell2 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell2 < 0){
                        delayCell2 += bufferLength;
                    }
                    else if(delayCell2 > bufferLength){
                        while(delayCell2 > bufferLength){
                            delayCell2 -= bufferLength;
                        }
                    }

                    if(numDelays >= 3){
                        tmpDelay = delay3 * factor1;
                        delayCell3 = writeCell - tmpDelay;
    
                        //Conditionals to keep delayCell in bounds
                        if(delayCell3 < 0){
                            delayCell3 += bufferLength;
                        }
                        else if(delayCell3 > bufferLength){
                            while(delayCell3 > bufferLength){
                                delayCell3 -= bufferLength;
                            }
                        }
                    }
                }
                //*****************CASE: THREE MODULATORS *********
                else if(numModulators == 3){
                    factor1 = mod1.nextCoefficient();
                    double tmpDelay = delay1 * factor1;
                    delayCell1 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell1 < 0){
                        delayCell1 += bufferLength;
                    }
                    else if(delayCell1 > bufferLength){
                        while(delayCell1 > bufferLength){
                            delayCell1 -= bufferLength;
                        }
                    }

                    factor2 = mod2.nextCoefficient();
                    tmpDelay = delay2 * factor2;
                    delayCell2 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell2 < 0){
                        delayCell2 += bufferLength;
                    }
                    else if(delayCell2 > bufferLength){
                        while(delayCell2 > bufferLength){
                            delayCell2 -= bufferLength;
                        }
                    }

                    factor3 = mod3.nextCoefficient();
                    tmpDelay = delay3 * factor3;
                    delayCell3 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell3 < 0){
                        delayCell3 += bufferLength;
                    }
                    else if(delayCell3 > bufferLength){
                        while(delayCell3 > bufferLength){
                            delayCell3 -= bufferLength;
                        }
                    }
                }

                //*******************INTERPOLATE SAMPLE ***********************************
                //*************************************************************************

                double delayVal1 = 0; //Initialize value that is the signal from the delay line
                double delayVal2 = 0;
                double delayVal3 = 0;

                //*******************CASE: ONE DELAY **********************
                if(numDelays >= 1){
                    int delayCellInt1 = static_cast<int>(delayCell1);

                    //Take an already existing sample if we have one
                    if(delayCell1 - (delayCellInt1 < 0.00001)){
                      delayVal1 = delayBuffer[delayCellInt1];
                    }
                    else{//interpolate!
                        //bandlimited interpolation
                        if(isBandlimited){
                            delayVal1 = sincBand(delayBuffer, bufferLength, delayCell1, AudioHandler::fs, AudioHandler::fs*factor1);
                        }
                        //linear interpolation
                        else{
                            int delayCellPlus = delayCellInt1 + 1;
                            delayCellPlus %= bufferLength;
                            double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt1];

                            delayVal1 = linInterp(delayCell1 - delayCellInt1, slope, delayBuffer[delayCellInt1]);
                        }
                    }
                }
                //******************CASE: TWO DELAYS *******************
                if(numDelays >= 2){
                    int delayCellInt2 = static_cast<int>(delayCell2);

                    //Take an already existing sample if we have one
                    if(delayCell2 - (delayCellInt2 < 0.00001)){
                      delayVal2 = delayBuffer[delayCellInt2];
                    }
                    else{//interpolate!
                        //bandlimited interpolation
                        if(isBandlimited){
                            delayVal2 = sincBand(delayBuffer, bufferLength, delayCell2, AudioHandler::fs, AudioHandler::fs*factor2);
                        }
                        //linear interpolation
                        else{
                            int delayCellPlus = delayCellInt2 + 1;
                            delayCellPlus %= bufferLength;
                            double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt2];

                            delayVal2 = linInterp(delayCell2 - delayCellInt2, slope, delayBuffer[delayCellInt2]);
                        }
                    }
                }
                //*****************CASE: THREE DELAYS ****************
                if(numDelays >= 3){
                    int delayCellInt3 = static_cast<int>(delayCell3);

                    //Take an already existing sample if we have one
                    if(delayCell3 - (delayCellInt3 < 0.00001)){
                      delayVal3 = delayBuffer[delayCellInt3];
                    }
                    else{//interpolate!
                        //bandlimited interpolation
                        if(isBandlimited){
                            delayVal3 = sincBand(delayBuffer, bufferLength, delayCell3, AudioHandler::fs, AudioHandler::fs*factor3);
                        }
                        //linear interpolation
                        else{
                            int delayCellPlus = delayCellInt3 + 1;
                            delayCellPlus %= bufferLength;
                            double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt3];

                            delayVal3 = linInterp(delayCell3 - delayCellInt3, slope, delayBuffer[delayCellInt3]);
                        }
                    }
                }

                //******************COMPUTE OUTPUT ****************************************
                
                //Only need one case since delayVal2 and delayVal3 will be 0 if those delays don't "exist"
                *out = (frames[i] * (dry/100.0)) + (delayVal1 * (wet/100.0)) + (delayVal2 * (wet/100.0)) + (delayVal3 * (wet/100.0)); //Compute the signal at the sum point
                
                //limiter
                if(*out > 1)
                    *out = 1;
                else if(*out < -1)
                    *out = -1;
                
                //*******************UPDATE DELAY BUFFER ***********************************
                if(writeCell >= bufferLength) //Loop around the buffer if reached the end
                    writeCell %= bufferLength;

                delayBuffer[writeCell++] = frames[i]; //write input to delay buffer
            }

            //************************CASE: DELAY HAS NOT STARTED YET *********************
            else{
                *out = frames[i];
                 
                if(writeCell >= bufferLength)
                    writeCell %= bufferLength;
                delayBuffer[writeCell++] = frames[i];

                delayCell1++; //increment delayCell so it can get up to 0
                delayCell2++;
                delayCell3++;
            }

            out++;
        }
    }


    
    //file writing overload
    StkFrames& MultiChorus::tick(void *input, int nBufferFrames, StkFrames& frames){
        FileWvIn *in = (FileWvIn *) input;
        
        unsigned int channels = AudioHandler::nChannels;
        frames.resize( nBufferFrames, channels );
        
        in->tickFrame( frames );

        for(unsigned int i = 0; i<frames.size(); i++){

            //If delay milliseconds have passed since the delay buffer was initialized then add in delay
            if(delayCell1 >= 0){

                //********************VARY THE DELAY TIME ********************************
                //************************************************************************
                double factor1 = 1, factor2 = 1, factor3 = 1; //the factors by which to vary delays
    
                /*Modulate the delay length*/

                //***************CASE : ONE MODULATOR *************
                if(numModulators == 1){
                    factor1 = mod1.nextCoefficient();
                    factor2 = factor1;
                    factor3 = factor1;

                    double tmpDelay = delay1 * factor1;
                    delayCell1 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell1 < 0){
                        delayCell1 += bufferLength;
                    }
                    else if(delayCell1 > bufferLength){
                        while(delayCell1 > bufferLength){
                            delayCell1 -= bufferLength;
                        }
                    }

                    if(numDelays >= 2){
                        tmpDelay = delay2 * factor1;
                        delayCell2 = writeCell - tmpDelay;
    
                        //Conditionals to keep delayCell in bounds
                        if(delayCell2 < 0){
                            delayCell2 += bufferLength;
                        }
                        else if(delayCell2 > bufferLength){
                            while(delayCell2 > bufferLength){
                                delayCell2 -= bufferLength;
                            }
                        }
                    }
                    if(numDelays >= 3){
                        tmpDelay = delay3 * factor1;
                        delayCell3 = writeCell - tmpDelay;
    
                        //Conditionals to keep delayCell in bounds
                        if(delayCell3 < 0){
                            delayCell3 += bufferLength;
                        }
                        else if(delayCell3 > bufferLength){
                            while(delayCell3 > bufferLength){
                                delayCell3 -= bufferLength;
                            }
                        }
                    }
                }
                //*************CASE: TWO MODULATORS *********************
                else if(numModulators == 2){
                    factor1 = mod1.nextCoefficient();
                    factor3 = factor1;

                    double tmpDelay = delay1 * factor1;
                    delayCell1 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell1 < 0){
                        delayCell1 += bufferLength;
                    }
                    else if(delayCell1 > bufferLength){
                        while(delayCell1 > bufferLength){
                            delayCell1 -= bufferLength;
                        }
                    }

                    factor2 = mod2.nextCoefficient();
                    tmpDelay = delay2 * factor2;
                    delayCell2 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell2 < 0){
                        delayCell2 += bufferLength;
                    }
                    else if(delayCell2 > bufferLength){
                        while(delayCell2 > bufferLength){
                            delayCell2 -= bufferLength;
                        }
                    }

                    if(numDelays >= 3){
                        tmpDelay = delay3 * factor1;
                        delayCell3 = writeCell - tmpDelay;
    
                        //Conditionals to keep delayCell in bounds
                        if(delayCell3 < 0){
                            delayCell3 += bufferLength;
                        }
                        else if(delayCell3 > bufferLength){
                            while(delayCell3 > bufferLength){
                                delayCell3 -= bufferLength;
                            }
                        }
                    }
                }
                //*****************CASE: THREE MODULATORS *********
                else if(numModulators == 3){
                    factor1 = mod1.nextCoefficient();
                    double tmpDelay = delay1 * factor1;
                    delayCell1 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell1 < 0){
                        delayCell1 += bufferLength;
                    }
                    else if(delayCell1 > bufferLength){
                        while(delayCell1 > bufferLength){
                            delayCell1 -= bufferLength;
                        }
                    }

                    factor2 = mod2.nextCoefficient();
                    tmpDelay = delay2 * factor2;
                    delayCell2 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell2 < 0){
                        delayCell2 += bufferLength;
                    }
                    else if(delayCell2 > bufferLength){
                        while(delayCell2 > bufferLength){
                            delayCell2 -= bufferLength;
                        }
                    }

                    factor3 = mod3.nextCoefficient();
                    tmpDelay = delay3 * factor3;
                    delayCell3 = writeCell - tmpDelay;

                    //Conditionals to keep delayCell in bounds
                    if(delayCell3 < 0){
                        delayCell3 += bufferLength;
                    }
                    else if(delayCell3 > bufferLength){
                        while(delayCell3 > bufferLength){
                            delayCell3 -= bufferLength;
                        }
                    }
                }

                //*******************INTERPOLATE SAMPLE ***********************************
                //*************************************************************************

                double delayVal1 = 0; //Initialize value that is the signal from the delay line
                double delayVal2 = 0;
                double delayVal3 = 0;

                //*******************CASE: ONE DELAY **********************
                if(numDelays >= 1){
                    int delayCellInt1 = static_cast<int>(delayCell1);

                    //Take an already existing sample if we have one
                    if(delayCell1 - (delayCellInt1 < 0.00001)){
                      delayVal1 = delayBuffer[delayCellInt1];
                    }
                    else{//interpolate!
                        //bandlimited interpolation
                        if(isBandlimited){
                            delayVal1 = sincBand(delayBuffer, bufferLength, delayCell1, AudioHandler::fs, AudioHandler::fs*factor1);
                        }
                        //linear interpolation
                        else{
                            int delayCellPlus = delayCellInt1 + 1;
                            delayCellPlus %= bufferLength;
                            double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt1];

                            delayVal1 = linInterp(delayCell1 - delayCellInt1, slope, delayBuffer[delayCellInt1]);
                        }
                    }
                }
                //******************CASE: TWO DELAYS *******************
                if(numDelays >= 2){
                    int delayCellInt2 = static_cast<int>(delayCell2);

                    //Take an already existing sample if we have one
                    if(delayCell2 - (delayCellInt2 < 0.00001)){
                      delayVal2 = delayBuffer[delayCellInt2];
                    }
                    else{//interpolate!
                        //bandlimited interpolation
                        if(isBandlimited){
                            delayVal2 = sincBand(delayBuffer, bufferLength, delayCell2, AudioHandler::fs, AudioHandler::fs*factor2);
                        }
                        //linear interpolation
                        else{
                            int delayCellPlus = delayCellInt2 + 1;
                            delayCellPlus %= bufferLength;
                            double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt2];

                            delayVal2 = linInterp(delayCell2 - delayCellInt2, slope, delayBuffer[delayCellInt2]);
                        }
                    }
                }
                //*****************CASE: THREE DELAYS ****************
                if(numDelays >= 3){
                    int delayCellInt3 = static_cast<int>(delayCell3);

                    //Take an already existing sample if we have one
                    if(delayCell3 - (delayCellInt3 < 0.00001)){
                      delayVal3 = delayBuffer[delayCellInt3];
                    }
                    else{//interpolate!
                        //bandlimited interpolation
                        if(isBandlimited){
                            delayVal3 = sincBand(delayBuffer, bufferLength, delayCell3, AudioHandler::fs, AudioHandler::fs*factor3);
                        }
                        //linear interpolation
                        else{
                            int delayCellPlus = delayCellInt3 + 1;
                            delayCellPlus %= bufferLength;
                            double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt3];

                            delayVal3 = linInterp(delayCell3 - delayCellInt3, slope, delayBuffer[delayCellInt3]);
                        }
                    }
                }

                //******************COMPUTE OUTPUT ****************************************
                
                //Only need one case since delayVal2 and delayVal3 will be 0 if those delays don't "exist"
                frames[i] = (frames[i] * (dry/100.0)) + (delayVal1 * (wet/100.0)) + (delayVal2 * (wet/100.0)) + (delayVal3 * (wet/100.0)); //Compute the signal at the sum point
                
                //limiter
                if(frames[i] > 1)
                    frames[i] = 0.9999;
                else if(frames[i] < -1)
                    frames[i] = -0.9999;

               //*******************UPDATE DELAY BUFFER ***********************************
                if(writeCell >= bufferLength) //Loop around the buffer if reached the end
                    writeCell %= bufferLength;

                delayBuffer[writeCell++] = frames[i]; //write input to delay buffer
            }

            //************************CASE: DELAY HAS NOT STARTED YET *********************
            else{
                if(writeCell >= bufferLength)
                    writeCell %= bufferLength;
                delayBuffer[writeCell++] = frames[i];

                delayCell1++; //increment delayCell so it can get up to 0
                delayCell2++;
                delayCell3++;
            }
        }

        return frames;
    }

  
//Static variables
int MultiChorus::MAX_DELAYS = 3;
int MultiChorus::MAX_MS_DELAY = 100;
int MultiChorus::MAX_BUFFER_LENGTH = static_cast<int>((MultiChorus::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);


    //Destructor
    FeedbackChorus::~FeedbackChorus(){
        destroyDelayBuffer();
        mod.~Modulator();
    }

    
    //Default Constructor
    FeedbackChorus::FeedbackChorus(){
        decay = 70;
        delay = 20;
        //mod.setModulator();
        writeCell = 0;
    }

    
    //Main Setter
    void FeedbackChorus::setFeedbackChorus(int tDecay, int tDelay, bool bandlimited){
        if(tDecay <= 100 && tDecay >= 0)
            decay = tDecay;
        else
            decay = 70;

        if(tDelay >= 0 && tDelay <= MAX_MS_DELAY)
            delay = tDelay;
        else
            delay = 20;

        mod.setModulator();

        delayCell = (int) (-1) * (fsPerMs * delay);
        writeCell = 0;

        bufferLength = 2 + static_cast<int>((delay / (1.0 *MAX_MS_DELAY) ) * MAX_BUFFER_LENGTH);

        initializeDelayBuffer();

        if(bandlimited){
            isBandlimited = true;
        }
    }

    
    //initializes the delayBuffer into an array of proper size
    void FeedbackChorus::initializeDelayBuffer(){
        destroyDelayBuffer();

        delayBuffer = new double[bufferLength];
     }
    
    //destroys the current delay buffer
    void FeedbackChorus::destroyDelayBuffer(){
        delete[ ] delayBuffer;
        delayBuffer = 0;
     }

    
    int FeedbackChorus::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
        FileWvIn *input = (FileWvIn *) userData;
        
        tick(outputBuffer, input, nBufferFrames); 

        if ( input->isFinished() ) {
            AudioHandler::done = true;
            return 1;
        }
        else
            return 0;
    }

    
    //real-time overload
    void FeedbackChorus::tick(void *outputBuffer, void *input,int nBufferFrames){
        register StkFloat *out = (StkFloat *) outputBuffer;

        FileWvIn *in = (FileWvIn *) input;
        
        StkFrames frames;
        unsigned int channels = AudioHandler::nChannels;
        frames.resize( nBufferFrames, channels );
        
        in->tickFrame( frames );

        for(unsigned int i = 0; i<frames.size(); i++){

            //If delay milliseconds have passed since the delay buffer was initialized then add in delay
            if(delayCell >= 0){
                //********************VARY THE DELAY TIME ********************************
                
                /*Modulate the delay length*/
                double factor = mod.nextCoefficient(); //the factor by which to vary the delay
                double tmpDelay = delay * factor; //the varied delay time for this sample
            
                delayCell = writeCell - tmpDelay; //compute the current delayed sample needed

                //Conditionals to keep delayCell in bounds
                if(delayCell < 0){
                    delayCell += bufferLength;
                }
                else if(delayCell > bufferLength){
                    while(delayCell > bufferLength){
                        delayCell -= bufferLength;
                    }
                }

                //*******************INTERPOLATE SAMPLE ***********************************

                double delayVal = 0; //Initialize value that is the signal from the delay line

                int delayCellInt = static_cast<int>(delayCell);

                //Take an already existing sample if we have one
                if(delayCell - (delayCellInt < 0.00001)){
                    delayVal = delayBuffer[delayCellInt];
                }
                //else iterpolate!
                else{
                    //bandlimited interpolation
                    if(isBandlimited){
                        delayVal = sincBand(delayBuffer, bufferLength, delayCell, AudioHandler::fs, AudioHandler::fs*factor);
                    }
                    //linear interpolation
                    else{
                        int delayCellPlus = delayCellInt + 1;
                        delayCellPlus %= bufferLength;
                        double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt];

                        delayVal = linInterp(delayCell - delayCellInt, slope, delayBuffer[delayCellInt]);
                    }
                }

                //******************COMPUTE OUTPUT ****************************************
                *out = (frames[i] + (delayVal * (decay/100.0))); //Compute the signal at the sum point

                //limiter
                if(*out > 1)
                    *out = 1;
                else if(*out < -1)
                    *out = -1;
                
               //*******************UPDATE DELAY BUFFER ***********************************
                if(writeCell >= bufferLength) //Loop around the buffer if reached the end
                    writeCell %= bufferLength;

                delayBuffer[writeCell++] = *out;
            }

            //************************CASE: DELAY HAS NOT STARTED YET *********************
            else{
                *out = frames[i];
                 
                if(writeCell >= bufferLength)
                    writeCell %= bufferLength;
                delayBuffer[writeCell++] = frames[i];

                delayCell++; //increment delayCell so it can get up to 0
            }

            out++;
        }
    }


    //file writing overload
    StkFrames& FeedbackChorus::tick(void *input, int nBufferFrames, StkFrames& frames){
        FileWvIn *in = (FileWvIn *) input;
        
        unsigned int channels = AudioHandler::nChannels;
        frames.resize( nBufferFrames, channels );
        
        in->tickFrame( frames );

        for(unsigned int i = 0; i<frames.size(); i++){

            //If delay milliseconds have passed since the delay buffer was initialized then add in delay
            if(delayCell >= 0){
                //********************VARY THE DELAY TIME ********************************
                
                /*Modulate the delay length*/
                double factor = mod.nextCoefficient(); //the factor by which to vary the delay
                double tmpDelay = delay * factor; //the varied delay time for this sample
            
                delayCell = writeCell - tmpDelay; //compute the current delayed sample needed

                //Conditionals to keep delayCell in bounds
                if(delayCell < 0){
                    delayCell += bufferLength;
                }
                else if(delayCell > bufferLength){
                    while(delayCell > bufferLength){
                        delayCell -= bufferLength;
                    }
                }

                //*******************INTERPOLATE SAMPLE ***********************************

                double delayVal = 0; //Initialize value that is the signal from the delay line

                int delayCellInt = static_cast<int>(delayCell);

                //Take an already existing sample if we have one
                if(delayCell - (delayCellInt < 0.00001)){
                    delayVal = delayBuffer[delayCellInt];
                }
                //else iterpolate!
                else{
                    //bandlimited interpolation
                    if(isBandlimited){
                        delayVal = sincBand(delayBuffer, bufferLength, delayCell, AudioHandler::fs, AudioHandler::fs*factor);
                    }
                    //linear interpolation
                    else{
                        int delayCellPlus = delayCellInt + 1;
                        delayCellPlus %= bufferLength;
                        double slope = delayBuffer[delayCellPlus] - delayBuffer[delayCellInt];

                        delayVal = linInterp(delayCell - delayCellInt, slope, delayBuffer[delayCellInt]);
                    }
                }

                //******************COMPUTE OUTPUT ****************************************
                frames[i] += (delayVal * (decay/100.0)); //Compute the signal at the sum point

                if(frames[i] >= 1)
                    frames[i] = 0.9999;
                else if(frames[i] <= -1)
                    frames[i] = -0.9999;
               //*******************UPDATE DELAY BUFFER ***********************************
                if(writeCell >= bufferLength) //Loop around the buffer if reached the end
                    writeCell %= bufferLength;

                delayBuffer[writeCell++] = frames[i];
            }

            //************************CASE: DELAY HAS NOT STARTED YET *********************
            else{
                if(writeCell >= bufferLength)
                    writeCell %= bufferLength;
                delayBuffer[writeCell++] = frames[i];

                delayCell++; //increment delayCell so it can get up to 0
            }
        }

        return frames;
    }


//Static variables
int FeedbackChorus::MAX_MS_DELAY = 100;
int FeedbackChorus::MAX_BUFFER_LENGTH = static_cast<int>((MultiChorus::MAX_MS_DELAY / 1000.0) * AudioHandler::fs);
    

int Modulator::STD_BUFFER_LENGTH = static_cast<int>( AudioHandler::fs ); // Samples per cycle at 1Hz
double Modulator::MAX_HZ = 10; //10Hz maximum frequency
double Modulator::MIN_HZ = 0.5; //0.5Hz minimum frequency
int Modulator::MAX_MODS = 3; //3 mods at most supported

Modulator::~Modulator(){
    destroyCoefficients();
}

Modulator::Modulator(){
    shape = sine;
    depth = 20;
    freq = 2.0;
    
    bufferLength =  static_cast<int>( STD_BUFFER_LENGTH * (1 / freq) );

    initializeCoefficients();

    coefficientIndex = 0;
}

void Modulator::setModulator(){
    int temp;
    cout << "Enter the parameters for the modulator." << endl;
    cout << "Shape: (0) sine, (1) saw, (2) triangular, (3) square:";
    cin >> temp;
    shape = static_cast<modShape>(temp);
    cout << "Frequency: " << MIN_HZ << "Hz - " << MAX_HZ << "Hz:";
    cin >> freq;
    cout << "Depth: amplitude of wave from 0% - 99%:";
    cin >> depth;

    //adjust to a decimal percent
    depth /= 100;
    
    //Fix input errors
    while(depth > 0.99){
        depth -= 1.0;
    }
    while(freq > 10.0){
        freq -= 0.2;
    }
    while(freq < 0.5){
        freq += 0.2;
    }
    
    bufferLength =  static_cast<int>( STD_BUFFER_LENGTH * (1 / freq) );

    initializeCoefficients();
}

//Creates a buffer ring containing coefficient values calculated for
//the given Wave shape over 1 period of the shape
void Modulator::initializeCoefficients(){
    destroyCoefficients();

    coefficient = new double[bufferLength];

    //initialize loop variables
    int i = 0;
    double t = 0.0;

    //Fill coefficient table. Uses samples from 1 period of the specified wave shape
    //and gets freq samples of the one period at even intervals
    switch(shape){
        case sine:
            for(i = 0, t = 0.0; i < bufferLength; i++, t += 1.0/bufferLength){
                //Sine wave adjustable by depth parameters centered around 1
                coefficient[i] = (depth * sin( 2 * M_PI * t ) ) + 1;
                //^^ is continuous, not adjustable for integer i's, hence t

            }
            break;
        case saw:
            for(i = 0, t = 0.0; i < bufferLength; i++, t += 1.0/bufferLength){
                //Sawtooth wave adjustable by freq. and depth parameters centered around 1
                coefficient[i] = (1 - (2 * depth)) + ((t - floor( t + 0.5 ) + 1 ) * 2 * depth); 
                //^^ is continuous, not adjustable for integer i's, hence t
            }
            break;
        case triangular:
            //Note, t increments 2/bufferLength because triangular wave has period spanning [0,2)
            for(i = 0, t = -0.5; i < bufferLength; i++, t += 2.0/bufferLength){
                //Triangular wave adjustable by freq. and depth parameter centered around 1
                coefficient[i] = (2 * depth) * fabs( (t ) - 2 * floor( t / 2.0 ) - 1 ) + (1 - depth);
                //^^ is continuous, not adjustable for integer i's, hence t
            }
        case square:
            for(i = 0; i < bufferLength; i++){
                //Low part of square
                if(i < static_cast<int>(bufferLength / 2) )
                    coefficient[i] = 1.0 - depth;
                //High part of square
                else
                    coefficient[i] = 1.0 + depth;
            }
            break;
    }
}

void Modulator::destroyCoefficients(){
    delete[ ] coefficient;
    coefficient = 0;
}


double Modulator::nextCoefficient(){
    if(coefficientIndex >= bufferLength)
        coefficientIndex %= bufferLength;

    return coefficient[coefficientIndex++];
}

