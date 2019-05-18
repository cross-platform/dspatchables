/************************************************************************
DSPatchables - DSPatch Component Repository
Copyright (c) 2014-2019 Marcus Tomlinson

This file is part of DSPatchables.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LICENSE included in the packaging of this file.
Please review the following information to ensure the GNU Lesser
General Public License version 3.0 requirements will be met:
http://www.gnu.org/copyleft/lgpl.html.

Other Usage
Alternatively, this file may be used in accordance with the terms and
conditions contained in a signed written agreement between you and
Marcus Tomlinson.

DSPatch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
************************************************************************/

#include <Oscillator.h>

#include <cmath>
#include <mutex>

const int c_sampleRate = 44100;
const int c_bufferSize = 440;  // Process 10ms chunks of data @ 44100Hz

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
    SetInputCount_( 1, {"freq (x1000)"} );
    SetOutputCount_( 1, {"out"} );
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
