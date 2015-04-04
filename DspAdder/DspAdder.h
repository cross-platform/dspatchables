/************************************************************************
DSPatch - Cross-Platform, Object-Oriented, Flow-Based Programming Library
Copyright (c) 2012-2015 Marcus Tomlinson

This file is part of DSPatch.

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

#ifndef DSPADDER_H
#define DSPADDER_H

#include <DSPatch.h>

//=================================================================================================
/// Example DspComponent: Adder

/** This component has 2 inputs and 1 output. The component receives 2 floating-point buffers into
it's 2 inputs, adds each buffer element of the 1st buffer to the corresponding element of the 2nd
buffer, then stores the resultant buffer into a 3rd buffer. This resultant buffer is then passed to
output 1 of the component output bus. */

class DspAdder : public DspComponent
{
public:
    //! Component constructor
    /*! When a component is constructed, it's input and output buses must be configured. This is
    achieved by making calls to the base class protected methods: "AddInput_()" and "AddOutput_().
    These methods must be called once per input / output required. IO signal names are optional
    (Component IO can be referenced by either string ID or index) and can be assigned to each
    input / output by supplying the desired string ID as an argument to the respective AddInput_()
    / AddOutput_() method call.*/

    DspAdder();
    ~DspAdder();

protected:
    //! Virtual process method inherited from DspComponent
    /*! The Process_() method is called from the DSPatch engine when a new set of component input
    signals are ready for processing. The Process() method has 2 arguments: the input bus and the
    output bus. This method's purpose is to pull its required inputs out of the input bus, process
    these inputs, and populate the output bus with the results (see DspSignalBus). */

    virtual void Process_(DspSignalBus& inputs, DspSignalBus& outputs);

private:
    std::vector<float> _stream1;
    std::vector<float> _stream2;
};

//=================================================================================================

class DspAdderPlugin : public DspPlugin
{
    DspComponent* Create(std::map<std::string, DspParameter>&) const
    {
        return new DspAdder();
    }
};

EXPORT_DSPPLUGIN(DspAdderPlugin)

//=================================================================================================

#endif  // DSPADDER_H
