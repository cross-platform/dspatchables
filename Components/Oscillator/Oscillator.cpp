/******************************************************************************
Oscillator DSPatch Component
Copyright (c) 2021, Marcus Tomlinson

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
#include <Oscillator.h>

#include <cmath>
#include <mutex>

using namespace DSPatch;
using namespace DSPatchables;

static const float TWOPI = 6.283185307179586476925286766559f;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class Oscillator
{
public:
    Oscillator( float startFreq, float startAmpl )
        : amplitude( startAmpl )
        , frequency( startFreq )
    {
        BuildLookup();
    }

    std::vector<short> signalLookup;
    std::vector<short> signal;

    int bufferSize = c_bufferSize;
    int sampleRate = c_sampleRate;
    float amplitude;
    float frequency;

    int lastPos = 0;
    int lookupLength = 0;

    std::mutex processMutex;

    void BuildLookup();
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

Oscillator::Oscillator( float startFreq, float startAmpl )
    : p( new internal::Oscillator( startFreq, startAmpl ) )
{
    SetInputCount_( 1, { "freq (x1000)" } );
    SetOutputCount_( 1, { "out" } );
}

void Oscillator::SetBufferSize( int bufferSize )
{
    std::lock_guard<std::mutex> lock( p->processMutex );

    p->bufferSize = bufferSize;
    p->BuildLookup();
}

void Oscillator::SetSampleRate( int sampleRate )
{
    std::lock_guard<std::mutex> lock( p->processMutex );

    p->sampleRate = sampleRate;
    p->BuildLookup();
}

void Oscillator::SetAmpl( float ampl )
{
    std::lock_guard<std::mutex> lock( p->processMutex );

    p->amplitude = ampl;
    p->BuildLookup();
}

void Oscillator::SetFreq( float freq )
{
    std::lock_guard<std::mutex> lock( p->processMutex );

    p->frequency = freq;
    p->BuildLookup();
}

int Oscillator::GetBufferSize() const
{
    return p->bufferSize;
}

int Oscillator::GetSampleRate() const
{
    return p->sampleRate;
}

float Oscillator::GetAmpl() const
{
    return p->amplitude;
}

float Oscillator::GetFreq() const
{
    return p->frequency;
}

void Oscillator::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto freq = inputs.GetValue<float>( 0 );
    if ( freq && *freq != 0.0f )
    {
        SetFreq( *freq * 1000 );
    }

    std::lock_guard<std::mutex> lock( p->processMutex );

    if ( !p->signalLookup.empty() )
    {
        for ( auto& sample : p->signal )
        {
            if ( p->lastPos >= p->lookupLength )
            {
                p->lastPos = 0;
            }
            sample = p->signalLookup[p->lastPos++];
        }

        outputs.SetValue( 0, p->signal );
    }
}

void DSPatchables::internal::Oscillator::BuildLookup()
{
    float posFrac = lookupLength <= 0 ? 0 : (float)lastPos / (float)lookupLength;
    float angleInc = TWOPI * frequency / sampleRate;

    lookupLength = (int)( (float)sampleRate / frequency );

    signal.resize( bufferSize );
    signalLookup.resize( lookupLength );

    for ( int i = 0; i < lookupLength; ++i )
    {
        signalLookup[i] = sin( angleInc * i ) * amplitude * 32767;
    }

    lastPos = (int)( posFrac * (float)lookupLength + 0.5f );  // calculate new position (round up)
}
