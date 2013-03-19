/*
Encapsulating class for a set of DSP effects. 
Manages functions that allow user to set parameters of a given
effect and to choose the current effect to use on the input stream.

Allows for abstraction of the exact effect to be used with the 
AudioHandler class
*/

#include "Effect.h"
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

//Destructor
Effect::~Effect(){
    sdelay.~SingleDelay();
    ddelay.~DoubleDelay();
    fdelay.~FeedbackDelay();
    chorus.~MultiChorus();
    flanger.~FeedbackChorus();
    verb1.~Reverb1();
    verb2.~Reverb2();
    verb3.~Reverb3();
}

Effect::Effect(){
effectType = SINGLE_DELAY;
}

//Main callback wrapper for all effects
int Effect::callback( void *outputBuffer, void *notUsed, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData ){
    if (effectType == SINGLE_DELAY)
        return sdelay.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if (effectType == DOUBLE_DELAY)
        return ddelay.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if (effectType == FEEDBACK_DELAY)
        return fdelay.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if (effectType == CHORUS)
        return chorus.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if (effectType == FLANGER)
        return flanger.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if(effectType == REVERB1)
        return verb1.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if(effectType == REVERB2)
        return verb2.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else if(effectType == REVERB3)
        return verb3.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
    else
        return sdelay.callback(outputBuffer, notUsed, nBufferFrames, streamTime, status, userData);
}

//Wrapper for Wave File Output tick calls
StkFrames& Effect::tick(void* input, int nBufferFrames, StkFrames& frames){
    if (effectType == SINGLE_DELAY)
        return sdelay.tick(input, nBufferFrames, frames);
    else if (effectType == DOUBLE_DELAY)
        return ddelay.tick(input, nBufferFrames, frames);
    else if (effectType == FEEDBACK_DELAY)
        return fdelay.tick(input, nBufferFrames, frames);
    else if (effectType == CHORUS)
        return chorus.tick(input, nBufferFrames, frames);
    else if (effectType == FLANGER)
        return flanger.tick(input, nBufferFrames, frames);
    else if(effectType == REVERB1)
        return verb1.tick(input, nBufferFrames, frames);
    else if(effectType == REVERB2)
        return verb2.tick(input, nBufferFrames, frames);
    else if(effectType == REVERB3)
        return verb3.tick(input, nBufferFrames, frames);
    else
        return sdelay.tick(input, nBufferFrames, frames);
}

void Effect::chooseEffect(){
    cout << "Choose the effect you wish to apply to the input stream:\n";
    cout << "   1) Single Delay\n";
    cout << "   2) Double Delay\n";
    cout << "   3) Feedback Delay \n";
    cout << "   4) Chorus\n";
    cout << "   5) Flanger\n";
    cout << "   6) Reverb 1 (5 Seq. Allpass Filters)\n";
    cout << "   7) Reverb 2 (4 Par. Comb Filters -> 2 Seq. Allpass Filters)\n";
    cout << "   8) Reverb 3 (6 Par. Low-Pass Comb Filters -> Allpass Filter)\n";
    cout << "<<<Enter Choice>>>:";
 
    int choice = 0;
    cin >> choice;

    switch(choice){
        case SINGLE_DELAY:
            setSingleDelay();
            break;
        case DOUBLE_DELAY:
            setDoubleDelay();
            break;
        case FEEDBACK_DELAY:
            setFeedbackDelay();
            break;
        case CHORUS:
            setChorus();
            break;
        case FLANGER:
            setFlanger();
            break;
        case REVERB1:
            setReverb1();
            break;
        case REVERB2:
            setReverb2();
            break;
        case REVERB3:
            setReverb3();
            break;
        default:
            setSingleDelay();
    }
}

void Effect::setEffect(){
//Nothing useful for this
}

void Effect::setSingleDelay(){
    int tDelay; 
    double tWet, tDry;

    effectType = SINGLE_DELAY;

    cout << "The parameters for the Single Delay unit must now be decided.";
    cout << endl;
    cout << "Enter the delay length (0-" << SingleDelay::MAX_MS_DELAY << "ms):";
    cin >> tDelay;
    cout << "Enter the % of the dry signal (0.0-1.0):";
    cin >> tDry;
    cout << "Enter the % of the wet signal (0.0-1.0):";
    cin >> tWet;

    sdelay.setSingleDelay(tDry, tWet, tDelay);
}
void Effect::setDoubleDelay(){
    int tDelay1, tDelay2; 
    double tWet1, tWet2, tDry;

    effectType = DOUBLE_DELAY;

    cout << "The parameters for the Double Delay unit must now be decided.";
    cout << endl;
    cout << "Enter the longest delay length (0-" << SingleDelay::MAX_MS_DELAY << "ms):";
    cin >> tDelay2;
    cout << "Enter the shortest delay length (0-" << SingleDelay::MAX_MS_DELAY << "ms):";
    cin >> tDelay1;
    cout << "Enter the % of the dry signal (0.0-1.0):";
    cin >> tDry;
    cout << "Enter the % of the wet signal of the longest delay (0.0-1.0):";
    cin >> tWet2;
    cout << "Enter the % of the wet signal of the shortest delay (0.0-1.0):";
    cin >> tWet1;

    ddelay.setDoubleDelay(tDry, tWet1, tWet2, tDelay1, tDelay2);
}
void Effect::setFeedbackDelay(){
    int tDecay, tDelay; 
    double tGain;

    effectType = FEEDBACK_DELAY;

    cout << "The parameters for the Feedback Delay unit must now be decided.";
    cout << endl;
    cout << "Enter the delay length (0-" << FeedbackDelay::MAX_MS_DELAY << "ms):";
    cin >> tDelay;
    cout << "WARNING: A DECAY OF 100 CAN OVERLOAD OUTPUT AND DAMAGE SPEAKERS\n";
    cout << "Enter the decay rate for the feedback signal (0 - 100):";
    cin >> tDecay;
    cout << "Enter the gain for the output signal (0.0-2.0):";
    cin >> tGain;

    fdelay.setFeedbackDelay(tGain, tDecay, tDelay);
}
void Effect::setChorus(){
    int tDry, tWet, tDelay1 = 0, tDelay2 = 0, tDelay3 = 0,
    tNumDelays, tNumModulators;
    bool bandlimited;

    //Set enumerator
    effectType = CHORUS;

    cout << "The parameters for the multi-staged chorus unit must now be decided.";
    cout << endl;
    cout << "Choose the % dry signal (0 - 100):";
    cin >> tDry;
    cout << "Choose the % wet signal (0 - 100):";
    cin >> tWet;
    cout << "Choose the number of stages/delays (1-3):";
    cin >> tNumDelays;
    cout << "\nPlease enter delays in order from longest to shortest.\n";

    switch(tNumDelays){
        case 3:
            cout << "Choose the third delay length (0-" << MultiChorus::MAX_MS_DELAY << "ms):";
            cin >> tDelay3;
        case 2:
            cout << "Choose the second delay length (0-" << MultiChorus::MAX_MS_DELAY << "ms):";
            cin >> tDelay2;
        case 1:
            cout << "Choose the first delay length (0-" << MultiChorus::MAX_MS_DELAY << "ms):";
            cin >> tDelay1;
            break;
        default:
            cout << "You entered and invalid number of stages, assuming 1.\n";
            cout << "Choose the delay length (0-" << MultiChorus::MAX_MS_DELAY << "ms):";
            cin >> tDelay1;
    }

    cout << "Select the number of seperate modulators. At most 1 per stage (1-3):";
    cin >> tNumModulators;

    if (tNumModulators > tNumDelays){
        cout << "You entered an invalid number of modulators. Defaulting to 1...\n";
        tNumModulators = 1;
    }

    int temp = 0;
    cout << "Select the type of interpolation: (0) Linear (1) Bandlimited:";
    cin >> temp;
    bandlimited = static_cast<bool>(temp);

    //call constructor
    chorus.setMultiChorus(tDry, tWet, tDelay1, tDelay2, tDelay3, tNumDelays, tNumModulators, bandlimited);
}

void Effect::setFlanger(){
    int tDecay, tDelay;
    bool bandlimited;

    //Set enumerator
    effectType = FLANGER;

    cout << "The parameters for the Flanger unit must now be decided.";
    cout << endl;
    cout << "Enter the delay length (0-" << FeedbackChorus::MAX_MS_DELAY << "ms):";
    cin >> tDelay;
    cout << "WARNING: A DECAY OF 100 CAN OVERLOAD OUTPUT AND DAMAGE SPEAKERS\n";
    cout << "Enter the decay rate for the feedback signal (0 - 100%):";
    cin >> tDecay;
    
    int temp = 0;
    cout << "Select the type of interpolation: (0) Linear (1) Bandlimited:";
    cin >> temp;
    bandlimited = static_cast<bool>(temp);

    //call constructor
    flanger.setFeedbackChorus(tDecay, tDelay, bandlimited);
}

void Effect::setReverb1(){
    int tDelay, tDecay, mix;

    effectType = REVERB1;

    cout << "The parameters for the Reverb 1 unit must now be decided:";
    cout << endl;
    cout << "Enter the mix ratio of wet to dry signal (0%-100%):";
    cin >> mix;
    verb1.setMix(mix);
    cout << "Enter the decay (0%-99%) for all filters:";
    cin >> tDecay;
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 1:";
    cin >> tDelay;
    verb1.setAP(1, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 2:";
    cin >> tDelay;
    verb1.setAP(2, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 3:";
    cin >> tDelay;
    verb1.setAP(3, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 4:";
    cin >> tDelay;
    verb1.setAP(4, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 5:";
    cin >> tDelay;
    verb1.setAP(5, tDelay, tDecay);
    /*
    cout << "Enter the decay (0%-99%) for Allpass 1:";
    cin >> tDecay;
    verb1.setAP(1, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 2:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass 2:";
    cin >> tDecay;
    verb1.setAP(2, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 3:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass 3:";
    cin >> tDecay;
    verb1.setAP(3, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 4:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass 4:";
    cin >> tDecay;
    verb1.setAP(4, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb1::MAX_MS_DELAY << "ms) for Allpass 5:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass 5:";
    cin >> tDecay; 
    verb1.setAP(5, tDelay, tDecay); */
}
void Effect::setReverb2(){
    int tDelay, tDecay, mix;

    effectType = REVERB2;

    cout << "The parameters for the Reverb 2 unit must now be decided:";
    cout << endl;
    cout << "Enter the mix ratio of wet to dry signal (0%-100%):";
    cin >> mix;
    verb2.setMix(mix);
    cout << "Enter the decay (0%-99%) for all filters:";
    cin >> tDecay;
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 1:";
    cin >> tDelay;
    verb2.setComb(1, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 2:";
    cin >> tDelay;
    verb2.setComb(2, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 3:";
    cin >> tDelay;
    verb2.setComb(3, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 4:";
    cin >> tDelay;
    verb2.setComb(4, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::A_MAX_MS_DELAY << "ms) for Allpass 1:";
    cin >> tDelay;
    verb2.setAP(1, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::A_MAX_MS_DELAY << "ms) for Allpass 2:";
    cin >> tDelay;
    verb2.setAP(2, tDelay, tDecay);

    /*
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 2:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Comb 2:";
    cin >> tDecay;
    verb2.setComb(2, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 3:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Comb 3:";
    cin >> tDecay;
    verb2.setComb(3, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::C_MAX_MS_DELAY << "ms) for Comb 4:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Comb 4:";
    cin >> tDecay;
    verb2.setComb(4, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::A_MAX_MS_DELAY << "ms) for Allpass 1:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass 1:";
    cin >> tDecay;
    verb2.setAP(1, tDelay, tDecay);
    cout << "Enter the delay length (0-" << Reverb2::A_MAX_MS_DELAY << "ms) for Allpass 2:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass 2:";
    cin >> tDecay;
    verb2.setAP(2, tDelay, tDecay);
    */
}
void Effect::setReverb3(){
    int tDelay, tDecay, tDecay2, mix;

    effectType = REVERB3;

    cout << "The parameters for the Reverb 3 unit must now be decided:";
    cout << endl;
    cout << "Enter the mix ratio of wet to dry signal (0%-100%):";
    cin >> mix;
    verb3.setMix(mix);
    cout << "Enter the first decay (0%-99%) for Low-Pass Combs:";
    cin >> tDecay;
    cout << "Enter the second decay (0%-99%) for Low-Pass Combs:";
    cin >> tDecay2;
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 1:";
    cin >> tDelay;
    verb3.setLPComb(1, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 2:";
    cin >> tDelay;
    verb3.setLPComb(2, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 3:";
    cin >> tDelay;
    verb3.setLPComb(3, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 4:";
    cin >> tDelay;
    verb3.setLPComb(4, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 5:";
    cin >> tDelay;
    verb3.setLPComb(5, tDelay, tDecay, tDecay2);
    cout << "Enter the first delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 6:";
    cin >> tDelay;
    verb3.setLPComb(6, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::A_MAX_MS_DELAY << "ms) for Allpass:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass:";
    cin >> tDecay;
    verb3.setAP(tDelay, tDecay);

    /*
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 2:";
    cin >> tDelay;
    cout << "Enter the first decay (0%-99%) for Low-Pass Comb 2:";
    cin >> tDecay;
    cout << "Enter the second decay (0%-99%) for Low-Pass Comb 2:";
    cin >> tDecay2;
    verb3.setLPComb(2, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 3:";
    cin >> tDelay;
    cout << "Enter the first decay (0%-99%) for Low-Pass Comb 3:";
    cin >> tDecay;
    cout << "Enter the second decay (0%-99%) for Low-Pass Comb 3:";
    cin >> tDecay2;
    verb3.setLPComb(3, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 4:";
    cin >> tDelay;
    cout << "Enter the first decay (0%-99%) for Low-Pass Comb 4:";
    cin >> tDecay;
    cout << "Enter the second decay (0%-99%) for Low-Pass Comb 4:";
    cin >> tDecay2;
    verb3.setLPComb(4, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 5:";
    cin >> tDelay;
    cout << "Enter the first decay (0%-99%) for Low-Pass Comb 5:";
    cin >> tDecay;
    cout << "Enter the second decay (0%-99%) for Low-Pass Comb 5:";
    cin >> tDecay2;
    verb3.setLPComb(5, tDelay, tDecay, tDecay2);
    cout << "Enter the first delay length (0-" << Reverb3::C_MAX_MS_DELAY << "ms) for Low-Pass Comb 6:";
    cin >> tDelay;
    cout << "Enter the first decay (0%-99%) for Low-Pass Comb 6:";
    cin >> tDecay;
    cout << "Enter the second decay (0%-99%) for Low-Pass Comb 6:";
    cin >> tDecay2;
    verb3.setLPComb(6, tDelay, tDecay, tDecay2);
    cout << "Enter the delay length (0-" << Reverb3::A_MAX_MS_DELAY << "ms) for Allpass:";
    cin >> tDelay;
    cout << "Enter the decay (0%-99%) for Allpass:";
    cin >> tDecay;
    verb3.setAP(tDelay, tDecay);
    */
}

//Static Variables
enum Effect::EFFECT_TYPE Effect::effectType;
MultiChorus Effect::chorus;
FeedbackChorus Effect::flanger;
SingleDelay Effect::sdelay;
DoubleDelay Effect::ddelay;
FeedbackDelay Effect::fdelay;
Reverb1 Effect::verb1;
Reverb2 Effect::verb2;
Reverb3 Effect::verb3;