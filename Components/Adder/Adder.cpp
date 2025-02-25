/******************************************************************************
Adder DSPatch Component
Copyright (c) 2025, Marcus Tomlinson

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

#include <Adder.h>

#include <mutex>

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class Adder
{
public:
    std::mutex processMutex;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

Adder::Adder()
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::Adder() )
{
    // add 2 inputs
    SetInputCount_( 2, { "in1", "in2" } );

    // add 1 output
    SetOutputCount_( 1, { "out" } );
}

Adder::~Adder() = default;

void Adder::SetInputCount( unsigned int inputCount )
{
    std::lock_guard<std::mutex> lock( p->processMutex );
    SetInputCount_( inputCount );
}

void Adder::Process_( SignalBus& inputs, SignalBus& outputs )
{
    std::lock_guard<std::mutex> lock( p->processMutex );
    auto in = inputs.GetValue<std::vector<short>>( 0 );
    if ( !in )
    {
        return;
    }

    const int signalCount = inputs.GetSignalCount();
    for ( int i = 1; i < signalCount; ++i )
    {
        auto nextIn = inputs.GetValue<std::vector<short>>( i );
        if ( !nextIn || in->size() != nextIn->size() )
        {
            return;
        }

        const size_t bufferSize = in->size();
        for ( size_t s = 0; s < bufferSize; ++s )
        {
            ( *in )[s] += ( *nextIn )[s];  // perform addition sample-by-sample
        }
    }

    outputs.MoveSignal( 0, *inputs.GetSignal( 0 ) );  // move combined signal to output
}
