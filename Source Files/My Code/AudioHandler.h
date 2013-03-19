#ifndef __AUDIOHANDLER_H__
#define __AUDIOHANDLER_H__

#include "Effect.h"
#include "FileWvIn.h"
#include "FileWvOut.h"

class AudioHandler{

public:
    static unsigned int bufferFrames; //# of buffer frames
    static unsigned int nChannels; //# of Channels
    static double fs; //Sample rate
    static bool done;

    RtAudio rtout;
    FileWvIn in;
    FileWvOut flout;

    Effect effect;

    enum outputType {fileOutput, realtimeOutput} outType;

    //Member functions
    void selectOutput(void);
    void openOutput(void);
    void closeOutput(void);
    
    bool openInput(void);
    void closeInput(void);
    
    void selectEffect(void);
    void destroyEffect(void);
};

#endif