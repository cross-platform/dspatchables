/******************************************************************************
InOut DSPatch Component
Copyright (c) 2022, Marcus Tomlinson

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

#include <InOut.h>

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{
class InOut
{
public:
    InOut( int inCount, int outCount )
        : inCount( inCount )
        , outCount( outCount )
    {
    }

    int inCount = 0;
    int outCount = 0;
};
}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

InOut::InOut( int inCount, int outCount )
    : p( new internal::InOut( inCount, outCount ) )
{
    _inputValues.resize( inCount );
    SetInputCount_( inCount );

    _outputValues.resize( outCount );
    SetOutputCount_( outCount );
}

InOut::~InOut()
{
}

void InOut::Process_( SignalBus& inputs, SignalBus& outputs )
{
    std::lock_guard<std::mutex> lock( _mutex );

    for ( int i = 0; i < p->inCount; ++i )
    {
        // put component inputs into _inputValues

        auto s = std::make_shared<fast_any::any>();
        s->swap( *inputs.GetSignal( i ) );
        _inputValues[i].push( s );
    }

    for ( int i = 0; i < p->outCount; ++i )
    {
        // put _outputValues into component outputs

        if ( !_outputValues[i].empty() )
        {
            outputs.MoveSignal( i, *_outputValues[i].front() );
            _outputValues[i].pop();
        }
    }
}
