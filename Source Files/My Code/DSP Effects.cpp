/*
DSP Effects.cpp

This file includes all related files
with code for I/O of audio and for
processing the audio streams through
various DSP effects.

To this point effects included are:
3 Delays
2 Freq. Modulators
3 Reverberations

The code of this file is the main() method
that actually executes all the included code
*/

//Includes
#include "AudioHandler.h"
#include <iostream>

//End Includes

//Using directives
using std::cout;
using std::endl;
using std::cin;
//End Using directives

int main(){
    AudioHandler audio;
    
    bool finished = false;
    char loop;

    while(!finished){
        audio.openInput();

        audio.selectOutput();

        audio.selectEffect();

        audio.openOutput();

        while(!AudioHandler::done){
            Stk::sleep(3000);
        }

        audio.closeOutput();
        audio.closeInput();

        cout << "Would you like to run the program again (y/n)?";
        cin >> loop;

        //terminate on n or N input
        if(loop == 'n' || loop == 'N')
            finished = true;
        else
            audio.done = false;
    }   
}