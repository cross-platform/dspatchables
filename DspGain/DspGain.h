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

#ifndef DSPGAIN_H
#define DSPGAIN_H

#include <DSPatch.h>

//=================================================================================================

class DspGain : public DspComponent
{
public:
    int pGain;  // Float

    DspGain();
    ~DspGain();

    void SetGain(float gain);
    float GetGain() const;

protected:
    virtual void Process_(DspSignalBus& inputs, DspSignalBus& outputs);
    virtual bool ParameterUpdating_(int index, DspParameter const& param);

private:
    std::vector<float> _stream;
};

//=================================================================================================

class DspGainPlugin : public DspPlugin
{
    DspComponent* Create(std::map<std::string, DspParameter>&) const
    {
        return new DspGain();
    }
};

EXPORT_DSPPLUGIN(DspGainPlugin)

//=================================================================================================

#endif  // DSPGAIN_H
