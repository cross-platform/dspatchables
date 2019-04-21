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

#pragma once

#include <DSPatch.h>

#include <mutex>
#include <queue>

namespace DSPatch
{
namespace DSPatchables
{

namespace internal
{
class InOut;
}

class DLLEXPORT InOut : public Component
{
public:
    InOut( int inCount, int outCount );
    virtual ~InOut();

    template <class ValueType>
    bool PushOutput( int index, ValueType const& value );

    template <class ValueType>
    std::unique_ptr<ValueType> PopInput( int index );

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

private:
    std::mutex _mutex;

    std::vector<std::queue<Signal::SPtr>> _inputValues;
    std::vector<std::queue<Signal::SPtr>> _outputValues;

    std::unique_ptr<internal::InOut> p;
};

template <class ValueType>
bool InOut::PushOutput( int index, ValueType const& value )
{
    std::lock_guard<std::mutex> lock( _mutex );

    if ( index < _outputValues.size() )
    {
        auto signal = std::make_shared<Signal>();
        signal->SetValue( value );
        _outputValues[index].push( signal );

        return true;
    }

    return false;
}

template <class ValueType>
std::unique_ptr<ValueType> InOut::PopInput( int index )
{
    std::lock_guard<std::mutex> lock( _mutex );

    if ( index < _inputValues.size() && !_inputValues[index].empty() )
    {
        auto signal = _inputValues[index].front();
        _inputValues[index].pop();

        auto value = signal->GetValue<ValueType>();
        if ( value )
        {
            return std::unique_ptr<ValueType>( new ValueType( *value ) );
        }
    }

    return nullptr;
}

}  // namespace DSPatchables
}  // namespace DSPatch
