#include <fft_kiss.h>

#include <kiss_fftr.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define FFT_FRAMESIZE 1024
#define MATRIX_COLUMNS 256
#define HOPSIZE 128

const double twopi = 8. * atan( 1. );

float* tmpOut;
bool ft = true;
bool bt = true;

bool kiss_ft = true;
kiss_fftr_cfg kiss_fftr_state;
kiss_fft_cpx sout[FFT_FRAMESIZE];

bool kiss_ift = true;
kiss_fftr_cfg kiss_ifftr_state;
kiss_fft_cpx specin[FFT_FRAMESIZE];

// needed for streaming stft
float previousInput[FFT_FRAMESIZE];
float nextOutput[FFT_FRAMESIZE];
float previousOutput[FFT_FRAMESIZE];

void initializeStft()
{
    for ( int i = 0; i < FFT_FRAMESIZE; i++ )
        previousInput[i] = 0.0f;
    for ( int i = 0; i < FFT_FRAMESIZE; i++ )
        nextOutput[i] = 0.0f;
    for ( int i = 0; i < FFT_FRAMESIZE; i++ )
        previousOutput[i] = 0.0f;
}

int stft( float* input, float* window, float* output, int input_size, int fftsize, int hopsize )
{

    int posin, posout, i, output_size;
    float *sigframe, *specframe;
    sigframe = new float[fftsize];
    specframe = new float[fftsize];
    output_size = input_size * fftsize / hopsize;

    for ( posin = posout = 0; posin < input_size; posin += hopsize )
    {
        // window a signal frame
        for ( i = 0; i < fftsize; i++ )
            if ( posin + i < input_size )
                sigframe[i] = input[posin + i] * window[i];
            // if we run out of input samples
            else
                sigframe[i] = 0;
        // transform it
        fft_kiss( sigframe, specframe, fftsize );

        // output it
        for ( i = 0; i < fftsize; i++, posout++ )
            output[posout] = specframe[i];
    }

    delete[] sigframe;
    delete[] specframe;

    return output_size;
}

// hopsize has to be smaller than fftsize!
void streamStft( float* input, float* window, float* output, int fftsize, int hopsize )
{
    int posin, posout, i;
    // to be optimized!!
    float *sigframe, *specframe;
    sigframe = new float[fftsize];
    specframe = new float[fftsize];

    for ( posout = posin = 0; posin < fftsize; posin += hopsize )
    {
        for ( i = 0; i < fftsize; i++ )
        {
            if ( ( i + posin ) < fftsize )
                sigframe[i] = previousInput[i + posin] * window[i];
            else
                sigframe[i] = input[( i + posin ) - fftsize] * window[i];
        }

        fft_kiss( sigframe, specframe, fftsize );

        // for(i=0; i < fftsize; i++, posout++) output[posout] = specframe[i];

        // convert to polar coordinates
        for ( i = 0; i < fftsize; i += 2, posout += 2 )
        {
            output[posout] = sqrt( ( specframe[i] * specframe[i] ) + ( specframe[i + 1] * specframe[i + 1] ) );
            output[posout + 1] = atan2f( specframe[i + 1], specframe[i] );
        }
    }

    for ( i = 0; i < fftsize; i++ )
        previousInput[i] = input[i];

    delete[] sigframe;
    delete[] specframe;
}

int istft( float* input, float* window, float* output, int input_size, int fftsize, int hopsize )
{

    int posin, posout, i, output_size;
    float *sigframe, *specframe;
    sigframe = new float[fftsize];
    specframe = new float[fftsize];
    output_size = input_size * hopsize / fftsize;

    for ( posout = posin = 0; posout < output_size; posout += hopsize )
    {
        // load in a spectral frame from input
        for ( i = 0; i < fftsize; i++, posin++ )
            specframe[i] = input[posin];
        // inverse-transform it
        ifft_kiss( specframe, sigframe, fftsize );
        // window it and overlap-add it
        for ( i = 0; i < fftsize; i++ )
            if ( posout + i < output_size )
                output[posout + i] += sigframe[i] * window[i];
    }
    delete[] sigframe;
    delete[] specframe;
    return output_size;
}

void streamIstft( float* input, float* window, float* output, int fftsize, int hopsize )
{
    int posin, posout, i;
    float *sigframe, *specframe;
    sigframe = new float[fftsize];
    specframe = new float[fftsize];

    // the second part of the previous framed calculation
    for ( i = 0; i < fftsize; i++ )
    {
        output[i] = nextOutput[i];
        nextOutput[i] = 0.0f;
    }

    for ( posout = posin = 0; posout < fftsize; posout += hopsize )
    {
        for ( i = 0; i < fftsize; i++, posin++ )
            specframe[i] = input[posin];

        ifft_kiss( specframe, sigframe, fftsize );

        for ( i = 0; i < fftsize; i++ )
        {
            if ( ( posout + i ) < fftsize )
                output[posout + i] += sigframe[i] * window[i];
            else
                // nextOutput is the second half of the two frame fft we need to add it to the output of the next frame
                nextOutput[( posout + i ) - fftsize] += sigframe[i] * window[i];
        }
    }

    delete[] sigframe;
    delete[] specframe;
}

void fft_kiss( float* in, float* out, int N )
{
    if ( kiss_ft )
    {
        kiss_fftr_state = kiss_fftr_alloc( N, 0, 0, 0 );
        kiss_ft = false;
    }

    kiss_fftr( kiss_fftr_state, (kiss_fft_scalar*)in, sout );

    int i, k;
    for ( i = 0, k = 0; i < N; i += 2, k++ )
    {
        out[i] = sout[k].r / (float)N;
        out[i + 1] = sout[k].i / (float)N;
    }
}

void ifft_kiss( float* in, float* out, int N )
{
    if ( kiss_ift )
    {
        kiss_ifftr_state = kiss_fftr_alloc( N, 1, 0, 0 );
        kiss_ift = false;
    }

    int i, k;
    for ( i = 0, k = 0; i < N; i += 2, k++ )
    {
        specin[k].r = in[i];
        specin[k].i = in[i + 1];
    }

    kiss_fftri( kiss_ifftr_state, specin, (kiss_fft_scalar*)out );
}

void fft_test( float* in, float* out, int N )
{
    for ( int i = 0, k = 0; k < N; i += 2, k++ )
    {
        out[i] = out[i + 1] = 0.f;
        for ( int n = 0; n < N; n++ )
        {
            out[i] += in[n] * cos( k * n * twopi / N );
            out[i + 1] -= in[n] * sin( k * n * twopi / N );
        }
        out[i] /= N;
        out[i + 1] /= N;
    }
}
