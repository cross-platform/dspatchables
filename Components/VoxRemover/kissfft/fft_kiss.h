#pragma once

int stft( float* input, float* window, float* output, int input_size, int fftsize, int hopsize );

int istft( float* input, float* window, float* output, int input_size, int fftsize, int hopsize );

void streamStft( float* input, float* window, float* output, int fftsize, int hopsize );

void streamIstft( float* input, float* window, float* output, int fftsize, int hopsize );

void initializeStft();

void fft_test( float* in, float* out, int N );

void fft_kiss( float* in, float* out, int N );

void ifft_kiss( float* in, float* out, int N );
