/************************************************************************
DSPatchables - DSPatch Component Repository
Copyright (c) 2014-2018 Marcus Tomlinson

This file is part of DSPatchables.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LGPLv3.txt included in the packaging of this
file. Please review the following information to ensure the GNU Lesser
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
    : p( new internal::Adder() )
{
    // add 2 inputs
    SetInputCount_( 2, {"in1", "in2"} );

    // add 1 output
    SetOutputCount_( 1, {"out"} );
}

void Adder::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto in1 = inputs.GetValue<std::vector<short>>( 0 );
    auto in2 = inputs.GetValue<std::vector<short>>( 1 );

    if ( !in1 || !in2 || in1->size() != in2->size() )
    {
        return;
    }

    for ( size_t i = 0; i < in1->size(); i++ )
    {
        ( *in1 )[i] += ( *in2 )[i];  // perform addition sample-by-sample
    }

    outputs.SetValue( 0, inputs, 0 );  // move combined signal to output
}
