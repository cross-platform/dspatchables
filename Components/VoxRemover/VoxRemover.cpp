/******************************************************************************
VoxRemover DSPatch Component
Copyright (c) 2024, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <Constants.h>
#include <VoxRemover.h>

#include <fft_kiss.h>

#include <math.h>
#include <mutex>

using namespace DSPatch;
using namespace DSPatchables;

static std::mutex kissfftMutex;

VoxRemover::VoxRemover()
{
    SetInputCount_( 3, { "ch1", "ch2", "Δphi (/10)" } );
    SetOutputCount_( 2 );

    _BuildHanningLookup();
}

VoxRemover::~VoxRemover()
{
}

void VoxRemover::Process_( SignalBus& inputs, SignalBus& outputs )
{
    // 0. get current buffers (L & R)
    auto in0 = inputs.GetValue<std::vector<short>>( 0 );
    auto in1 = inputs.GetValue<std::vector<short>>( 1 );

    if ( !in0 || !in1 || ( *in0 ).size() != c_bufferSize || ( *in1 ).size() != c_bufferSize )
    {
        return;
    }

    auto dphi = inputs.GetValue<float>( 2 );
    if ( dphi )
    {
        _dphi = *dphi / 10;
    }

    for ( auto i = 0; i < c_bufferSize; i++ )
    {
        _inputBufL[i] = ( *in0 )[i] * c_s2fCoeff;
        _inputBufR[i] = ( *in1 )[i] * c_s2fCoeff;
    }

    // 1. build sigBufs: stored buffer + current buffer (L & R)
    _BuildSignalBuffers();

    // 2. replace stored buffer with current buffer (L & R)
    _StoreCurrentBuffers();

    // 3. apply window to sigbuf (L & R)
    _ApplyHanningWindows();

    // 4. do fft to sigbuf -> specbuf (L & R)
    _FftSignalBuffers();

    // 5. process specbuf (L & R)
    _ProcessSpectralBuffers();

    // 6. do ifft to specbuf -> sigbuf (L & R)
    _IfftSpectralBuffers();

    for ( auto i = 0; i < c_bufferSize; i++ )
    {
        // 7. add first half of sigbuf to last buffer (L & R)
        _outputBufL[i] = ( _mixBufL[i] + _sigBufL[i] ) * c_f2sCoeff;
        _outputBufR[i] = ( _mixBufR[i] + _sigBufR[i] ) * c_f2sCoeff;

        // 8. replace current buffer with second half of sigbuf (L & R)
        _mixBufL[i] = _sigBufL[c_bufferSize + i];
        _mixBufR[i] = _sigBufR[c_bufferSize + i];
    }

    // 9. output next buffer
    outputs.SetValue( 0, _outputBufL );
    outputs.SetValue( 1, _outputBufR );
}

void VoxRemover::_BuildHanningLookup()
{
    for ( auto i = 0; i < c_bufferSize; i++ )
    {
        // store 1st half window
        _hanningLookup[i] = 0.5f - (float)( 0.5f * cos( i * c_pi / ( c_bufferSize ) ) );

        // store 2nd half window
        _hanningLookup[c_bufferSize + i] = 0.5f - (float)( 0.5f * cos( ( i * c_pi / ( c_bufferSize ) ) + c_pi ) );
    }
}

void VoxRemover::_BuildSignalBuffers()
{
    for ( auto i = 0; i < c_bufferSize; i++ )
    {
        // build left channel
        _sigBufL[i] = _prevInputBufL[i];
        _sigBufL[c_bufferSize + i] = _inputBufL[i];

        // build right channel
        _sigBufR[i] = _prevInputBufR[i];
        _sigBufR[c_bufferSize + i] = _inputBufR[i];
    }
}

void VoxRemover::_StoreCurrentBuffers()
{
    _prevInputBufL = _inputBufL;
    _prevInputBufR = _inputBufR;
}

void VoxRemover::_ApplyHanningWindows()
{
    for ( size_t i = 0; i < _sigBufL.size(); i++ )
    {
        _sigBufL[i] *= _hanningLookup[i];  // apply hanning window to left channel
        _sigBufR[i] *= _hanningLookup[i];  // apply hanning window to right channel
    }
}

void VoxRemover::_FftSignalBuffers()
{
    std::lock_guard<std::mutex> lock( kissfftMutex );
    fft_kiss( &_sigBufL[0], &_specBufL[0], _sigBufL.size() );
    fft_kiss( &_sigBufR[0], &_specBufR[0], _sigBufR.size() );
}

void VoxRemover::_ProcessSpectralBuffers()
{
    float _frq = 0;  // frequency

    for ( size_t i = 2; i < _specBufL.size(); i += 2 )
    {
        // retrieve frequency (0Hz to Nyquist Frequency)
        _frq += (float)c_sampleRate / (float)_specBufL.size();

        // retrieve magnitudes (L & R)
        // [i] = real & [i+1] = imaginary
        // float _magL = 2.0f * sqrt( _specBufL[i] * _specBufL[i] + _specBufL[i + 1] * _specBufL[i + 1] );
        // float _magR = 2.0f * sqrt( _specBufR[i] * _specBufR[i] + _specBufR[i + 1] * _specBufR[i + 1] );

        // retrieve phases (L & R)
        // atan(i,r) gives (-pi to +pi), so add pi and divide by 2pi to get (0 to 1), then shift left 0.25 (90deg) to
        // convert cos phi to sin phi
        float _phiL = ( ( atan2( _specBufL[i + 1], _specBufL[i] ) + c_pi ) / c_twoPi ) - 0.25f;
        float _phiR = ( ( atan2( _specBufR[i + 1], _specBufR[i] ) + c_pi ) / c_twoPi ) - 0.25f;

        // convert phase to a value between 0 and 1 (L & R)
        if ( _phiL < 0 )
            _phiL += 1;
        if ( _phiL > 1 )
            _phiL -= 1;
        if ( _phiR < 0 )
            _phiR += 1;
        if ( _phiR > 1 )
            _phiR -= 1;

        // remove if phase diff < delta phi && frequency > 200
        if ( ( fabs( _phiL - _phiR ) < _dphi ) && _frq > 200 )
        {
            _specBufL[i] = 0.0f;
            _specBufL[i + 1] = 0.0f;
            _specBufR[i] = 0.0f;
            _specBufR[i + 1] = 0.0f;
        }
    }
}

void VoxRemover::_IfftSpectralBuffers()
{
    std::lock_guard<std::mutex> lock( kissfftMutex );
    ifft_kiss( &_specBufL[0], &_sigBufL[0], _sigBufL.size() );
    ifft_kiss( &_specBufR[0], &_sigBufR[0], _sigBufR.size() );
}
