/************************************************************************
DSPatchables - DSPatch Component Repository
Copyright (c) 2014-2015 Marcus Tomlinson

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

#include <DspAdder.h>

//=================================================================================================

DspAdder::DspAdder()
{
    // add 2 inputs
    AddInput_("Input1");
    AddInput_("Input2");

    // add 1 output
    AddOutput_("Output1");
}

//-------------------------------------------------------------------------------------------------

DspAdder::~DspAdder()
{
}

//=================================================================================================

void DspAdder::Process_(DspSignalBus& inputs, DspSignalBus& outputs)
{
    // get input values from inputs bus (GetValue() returns true if successful)
    if (!inputs.GetValue(0, _stream1))
    {
        _stream1.assign(_stream1.size(), 0);  // clear buffer if no input received
    }
    // do the same to the 2nd input buffer
    if (!inputs.GetValue(1, _stream2))
    {
        _stream2.assign(_stream2.size(), 0);
    }

    // ensure that the 2 input buffer sizes match
    if (_stream1.size() == _stream2.size())
    {
        for (size_t i = 0; i < _stream1.size(); i++)
        {
            _stream1[i] += _stream2[i];  // perform addition element-by-element
        }
        outputs.SetValue(0, _stream1);  // set output 1
    }
    // if input sizes don't match
    else
    {
        outputs.ClearValue(0);  // clear the output
    }
}

//=================================================================================================
