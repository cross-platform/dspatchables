/******************************************************************************
InOut DSPatch Component
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

class DLLEXPORT InOut final : public Component
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

    if ( (size_t)index < _outputValues.size() )
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

    if ( (size_t)index < _inputValues.size() && !_inputValues[index].empty() )
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
