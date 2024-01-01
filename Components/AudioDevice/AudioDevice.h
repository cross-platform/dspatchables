/******************************************************************************
AudioDevice DSPatch Component
Copyright (c) 2024, Marcus Tomlinson

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

#include <functional>

namespace DSPatch
{
namespace DSPatchables
{

namespace internal
{
class AudioDevice;
}

class DLLEXPORT AudioDevice final : public Component
{
public:
    explicit AudioDevice( bool isOutputDevice = true,
                          std::vector<std::string> const& deviceNameHas = std::vector<std::string>{},
                          bool defaultIfNotFound = true,
                          bool loopback = false );
    ~AudioDevice();

    bool Available();
    void SetAvailableCallback( std::function<void( bool )> const& callback );

    bool SetDevice( unsigned int deviceId, bool loopback );
    bool SetDevice( bool isOutputDevice, std::vector<std::string> const& deviceNameHas, bool defaultIfNotFound, bool loopback );

    bool ReloadDevices();

    std::string GetDeviceName( unsigned int deviceId ) const;
    unsigned int GetDeviceInputCount( unsigned int deviceId ) const;
    unsigned int GetDeviceOutputCount( unsigned int deviceId ) const;
    std::vector<unsigned int> GetSampleRates( unsigned int deviceId ) const;

    std::vector<unsigned int> GetDeviceIds() const;
    unsigned int GetDefaultInputDevice() const;
    unsigned int GetDefaultOutputDevice() const;
    unsigned int GetCurrentDevice() const;

    unsigned int SetBufferSize( unsigned int bufferSize );
    bool SetSampleRate( unsigned int sampleRate );

    bool IsStreaming() const;
    unsigned int GetBufferSize() const;
    unsigned int GetSampleRate() const;

    void ShowWarnings( bool enabled );

    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override;

private:
    std::unique_ptr<internal::AudioDevice> p;
};

}  // namespace DSPatchables
}  // namespace DSPatch
