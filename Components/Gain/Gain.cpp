/******************************************************************************
Gain DSPatch Component
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

#include <Gain.h>

#include <algorithm>
#include <atomic>

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class Gain
{
public:
    std::atomic<float> gain = 1.0f;
    std::atomic<bool> muted = false;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

Gain::Gain()
    : p( new internal::Gain() )
{
    SetInputCount_( 2, { "in", "gain" } );
    SetOutputCount_( 1, { "out" } );
}

Gain::~Gain() = default;

void Gain::SetGain( float gain )
{
    p->gain = gain;
}

float Gain::GetGain() const
{
    return p->gain;
}

void Gain::SetMute( bool muted )
{
    p->muted = muted;
}

bool Gain::GetMute() const
{
    return p->muted;
}

void Gain::Process_( SignalBus& inputs, SignalBus& outputs )
{
    auto in = inputs.GetValue<std::vector<short>>( 0 );
    if ( !in )
    {
        return;
    }

    auto gain = inputs.GetValue<float>( 1 );
    if ( gain )
    {
        p->gain = *gain;
    }

    // apply gain sample-by-sample
    if ( p->muted )
    {
        std::for_each( ( *in ).begin(), ( *in ).end(), []( short& sample ) { sample = 0.0f; } );
    }
    else
    {
        std::for_each( ( *in ).begin(), ( *in ).end(), [this]( short& sample ) { sample *= p->gain; } );
    }

    outputs.MoveSignal( 0, *inputs.GetSignal( 0 ) );  // move gained input signal to output
}
