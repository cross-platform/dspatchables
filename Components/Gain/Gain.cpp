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

#include <Gain.h>

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
    Gain( float initGain )
    {
        gain = initGain;
    }

    float gain;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

Gain::Gain( float initGain )
    : p( new internal::Gain( initGain ) )
{
    SetInputCount_( 2, {"in", "gain"} );
    SetOutputCount_( 1, {"out"} );
}

void Gain::SetGain( float gain )
{
    p->gain = gain;
}

float Gain::GetGain() const
{
    return p->gain;
}

void Gain::Process_( SignalBus const& inputs, SignalBus& outputs )
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

    for ( size_t i = 0; i < in->size(); i++ )
    {
        ( *in )[i] *= p->gain;  // apply gain sample-by-sample
    }

    outputs.SetValue( 0, inputs, 0 );  // move gained input signal to output
}
