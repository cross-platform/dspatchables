/******************************************************************************
Adder DSPatch Component
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

#include <Adder.h>

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

void Adder::Process_( SignalBus& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<std::vector<short>>( 0 );
    auto in2 = inputs.GetValue<std::vector<short>>( 1 );

    if ( !in1 || !in2 || in1->size() != in2->size() )
    {
        return;
    }

    for ( size_t i = 0; i < in1->size(); ++i )
    {
        ( *in1 )[i] += ( *in2 )[i];  // perform addition sample-by-sample
    }

    outputs.MoveSignal( 0, inputs.GetSignal( 0 ) );  // move combined signal to output
}
