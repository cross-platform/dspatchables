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

void InOut::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    std::lock_guard<std::mutex> lock( _mutex );

    for ( int i = 0; i < p->inCount; ++i )
    {
        // put component inputs into _inputValues

        auto s = std::make_shared<Signal>();
        s->MoveSignal( inputs.GetSignal( i ) );
        _inputValues[i].push( s );
    }

    for ( int i = 0; i < p->outCount; ++i )
    {
        // put _outputValues into component outputs

        if ( !_outputValues[i].empty() )
        {
            outputs.MoveSignal( i, _outputValues[i].front() );
            _outputValues[i].pop();
        }
    }
}
