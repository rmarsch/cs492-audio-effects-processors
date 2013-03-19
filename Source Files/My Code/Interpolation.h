/**
Interpolation.h

Header file for the interpolation header file
created by Richard Marscher for a Directed Study
with Professor Stan Sclaroff in the Boston University
Undergraduate Computer Science Program in 2009.

Feel free to re-use, re-distribute, correct, or do
anything else to this file, as long as this opening
comment is kept 
*//**

Bandlimited interpolation is implemented using the ideal low-pass filter,
 the sinc function.

Linear interpolation is also implemented.

Use these functions...

sincBand(...)
linInterp(...)
*/

#ifndef __INTERPOLATION_H__
#define __INTERPOLATION_H__

/*Probably reduntant but VS2008 is problematic
with the math libary for some reason
so I tried hard to get the right functions in
Feel free to optimize this */
#include <cmath>
#include <math.h>

/*Wrapped in a namespace in case
sinc() or minimum() are already
defined.
To use method names without qualification
include the following lines of code when including this
header file:

using Interpolation::sincBand;
using Interpolation::linInterp; */
namespace Interpolation{

    //Declare M_PI for Weird VS2008 Error
    #ifndef M_PI
    #define M_PI 3.14159265358979323846
    #endif 

    /*********Bandlimited Interpolation***********/

    //Constant value for max # of interpolated points per left/right of the desired new sample
    const int INTERP_MAX = 256;

    //minimum helper function
    double minimum(double a, double b){
	    return a<b ? a : b;
    }

    //sinc function
    double sinc(double x){
	    if (x == 0.0)
		    return 1;
	    else
		    return sin(M_PI * x) / (M_PI * x);
    }

    /*  Bandlimited interpolation with sinc ideal high pass filter
        buffer: is a pointer to the head of a buffer that contains the samples to interpolate
        sampleDesired: is the "imaginary" index value of the sample you want. 
        i.e. 31.5 is a sample halfway between buffer[30] and buffer [31]
        stdFreq: is the normal sampling rate / frequency of the buffer you are passing
        newFreq: is the frequency / sampling rate you are converting to

        I'm not sure if it is necessary, but sampleDesired should properly reflect a point
        at which the newFreq would have an integer sample.
        i.e. sampleDesired = 31.5; stdFreq = 44100; Then newFreq = 96000.
    */
    double sincBand(double* buffer, int bufferLength, double sampleDesired, double stdFreq, double newFreq){
        double sum = 0;
        int sampleStart = static_cast<int>(sampleDesired); //left sample

        int FILT_SIZE = INTERP_MAX * 2; //size of filter table

        //*************************
        //BUILD FILTER TABLE
	    double* filter = new double[FILT_SIZE];

	    for(int i = 1 - INTERP_MAX; i <= INTERP_MAX; i++){
		    filter[i + INTERP_MAX] = minimum(1, newFreq/stdFreq) * sinc( minimum(stdFreq, newFreq) * (i) );
        }
        //END BUILDING FILTER TABLE
        //*************************

        //************************
        //COMPUTE x(sampleDesired)
	    for(int i = 1 + sampleStart - INTERP_MAX; i <= sampleStart+INTERP_MAX; i++){

		    //fix i if it would be out of index bounds
		    if(i < 0)
			    i += bufferLength;

            sum += buffer[i % bufferLength] * filter[i % FILT_SIZE];
        }
        //END INTERPOLATION
        //*************************

        //free filter table memory
        delete[ ] filter;

        return sum;
    }

    /*********Linear Interpolation**************/

    //Calculates linear interopolation given the smaller sample, the "fraction" away
    //from the smaller sample to the next sample that the interpolation is occuring,
    //and the "slope" or difference between the left and right samples
    double linInterp(double fraction, double slope, double leftSample){
        return slope*fraction + leftSample;
    }

}
#endif