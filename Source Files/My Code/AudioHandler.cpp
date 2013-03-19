/*
AudioHandler.cpp

AudioHandler
This module controls the input and output file streams used
for audio files. 
The module encapsulates file-handling, thus it will not matter
to other classes whether the streams are real-time or file-driven
At this point it will expect only files of
the type *.wav 

*/

#include "Stk.h"
#include "AudioHandler.h"
#include <cstring>
#include <iostream>
using std::strstr;
using std::cout;
using std::cin;
using std::endl;
using std::string;

typedef unsigned int uint;
  

//Initialize static variables
double AudioHandler::fs = 44100.0;
unsigned int AudioHandler::bufferFrames = 256;
unsigned int AudioHandler::nChannels = 2;
bool AudioHandler::done = false;


/*
selectOutput()

selects whether the output is
real-time or file-based
*/
void AudioHandler::selectOutput(){
    uint select = 0;

    cout << "Select (0) Real-Time Output or (1) File-based Output:";
    cin >> select;

    bool fileBased = static_cast<bool>(select);

    if(fileBased){
        outType = fileOutput;
    }
    else{
        outType = realtimeOutput;
    }
}

/*
openOutput()

opens the output stream according
to the output type currently
designated by outType in this instance
of AudioHandler
*/
void AudioHandler::openOutput(){
    //File-based output branch
    if(outType == fileOutput){
        Stk::StkFormat format = ( sizeof(StkFloat) == 8 ) ? Stk::STK_FLOAT64 : Stk::STK_FLOAT32;

        string file;
        cout << "Enter the name for the output file (do not include .wav): ";
        cin >> file;
        file += ".wav";

        flout.openFile(file, AudioHandler::nChannels, FileWrite::FILE_WAV, format);

        StkFrames frames;

        for ( unsigned int i=0; !in.isFinished(); i++ ){     
            flout.tickFrame( effect.tick(&in, AudioHandler::bufferFrames, frames) );
        }

        AudioHandler::done = true;

    }
    //Real-time output branch
    else{
        if(rtout.getDeviceCount() < 1){
        cout << "\nNo audio devices found!\n";
        exit ( 1 );
        }

        rtout.showWarnings( true );       

        RtAudio::StreamParameters oParams;
        oParams.deviceId = rtout.getDefaultOutputDevice();
        oParams.nChannels = AudioHandler::nChannels;
        RtAudioFormat format = ( sizeof(StkFloat) == 8 ) ? RTAUDIO_FLOAT64 : RTAUDIO_FLOAT32;

        RtAudio::StreamOptions options;
        options.flags = RTAUDIO_HOG_DEVICE;

        uint nonConstBufferFrames = AudioHandler::bufferFrames;

        try {
            rtout.openStream( &oParams, NULL, format, this->fs, &nonConstBufferFrames, &Effect::callback, (void *)&in, &options );
        }
        catch ( RtError& e ) {
            e.printMessage();
        }

        try{
            rtout.startStream();
        }
        catch (RtError &error ) {
            error.printMessage();
        }
    }
}

/*
closeOutput()

closes the output stream.
necessary for looping the main program
*/
void AudioHandler::closeOutput(){
    if(outType == fileOutput){
        flout.closeFile();
    }
    else{
        rtout.closeStream();
    }
}

/*
openInput()

opens a valid input file for processing.
*/
bool AudioHandler::openInput(){
    string file;
    int sampleRate = 44100;

    cout << "Enter the path of a valid (non 24-bit) .wav file:";
    cin >> file;

    //if .wav is not found to be the extension format, prompt until it is
    while( (file.find(".wav", 0) == string::npos) && (file.find(".WAV", 0) == string::npos) ){
        cout << "\nInvalid file type. Please try again:\n";
        cin >> file;
   }

   try{
       in.openFile(file);
   }
   catch( StkError & ) {
       cout << "\nInput file did not open correctly.\n";
       return false;
   }

   AudioHandler::fs = in.getFileRate();
  // Stk::setSampleRate( AudioHandler::fs );
   AudioHandler::nChannels = in.getChannels();

   return true;
}

/*
closeInput()

closes the input stream.
necessary for looping the main program
*/
void AudioHandler::closeInput(){
    in.closeFile();
}

/*
selectEffect()

selects the type of effect unit to
use as the callback function for the
output stream.
*/
void AudioHandler::selectEffect(){
    uint select = 0;

    effect.chooseEffect();
}

/*
destroyChorus()

makes a call to the destructor of
the ChorusUnit class. This is necessary
to loop the main program, allowing for
a new instantiation of the member object
for the chorus unit within the audioHandler
*/
void AudioHandler::destroyEffect(){
    effect.~Effect();
}