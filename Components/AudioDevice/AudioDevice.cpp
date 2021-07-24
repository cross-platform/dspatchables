/******************************************************************************
AudioDevice DSPatch Component
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

#include <AudioDevice.h>
#include <Constants.h>

#include <RtAudio.h>

#include <algorithm>
#include <condition_variable>
#include <cstring>

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class AudioDevice
{
public:
    AudioDevice()
    {
        for ( unsigned int i = 0; i < audioStream.getDeviceCount(); ++i )
        {
            deviceList.emplace_back( audioStream.getDeviceInfo( i ) );
        }
    }

    void WaitForBuffer();
    void SyncBuffer();

    void StopStream();
    void StartStream();

    static int StaticCallback( void* outputBuffer,
                               void* inputBuffer,
                               unsigned int nBufferFrames,
                               double streamTime,
                               unsigned int status,
                               void* userData );

    int DynamicCallback( void* inputBuffer, void* outputBuffer, RtAudioStreamStatus status );

    std::vector<std::vector<short>> outputChannels;
    std::vector<std::vector<short>> inputChannels;

    std::mutex buffersMutex;
    std::mutex syncMutex;
    std::condition_variable waitCondt;
    std::condition_variable syncCondt;
    bool gotWaitReady = false;
    bool gotSyncReady = true;

    int currentDeviceIndex = -1;
    bool isStreaming = false;
    int bufferSize = c_bufferSize;
    int sampleRate = c_sampleRate;

    std::vector<RtAudio::DeviceInfo> deviceList;
    RtAudio::DeviceInfo currentDevice;

    RtAudio audioStream;
    RtAudio::StreamParameters outputParams;
    RtAudio::StreamParameters inputParams;

    std::mutex availableMutex;
    std::mutex processMutex;
    bool hasInputs = false;
    std::vector<std::string> nameHas;
    bool defaultIfNotFound = false;
    bool loopback = false;
    bool notFoundNotified = false;

    std::function<void( bool )> callback = []( bool ) {};
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

AudioDevice::AudioDevice( bool isOutputDevice,
                          std::vector<std::string> const& deviceNameHas,
                          bool defaultIfNotFound,
                          bool loopback )
    : p( new internal::AudioDevice() )
{
    SetDevice( isOutputDevice, deviceNameHas, defaultIfNotFound, loopback );

    p->currentDeviceIndex = -1;
}

AudioDevice::~AudioDevice()
{
    p->StopStream();
}

void AudioDevice::SetAvailableCallback( std::function<void( bool )> const& callback )
{
    std::lock_guard<std::mutex> lock( p->availableMutex );
    p->callback = callback;
}

bool AudioDevice::Available()
{
    std::lock_guard<std::mutex> lock( p->availableMutex );

    if ( p->nameHas.empty() )
    {
        p->notFoundNotified = false;
        return true;
    }

    ReloadDevices();

    for ( int i = 0; i < GetDeviceCount(); ++i )
    {
        auto name = GetDeviceName( i );

        std::transform( name.begin(), name.end(), name.begin(), ::tolower );

        // check if the device name contains p->nameHas
        bool found = true;
        for ( auto sub : p->nameHas )
        {
            std::transform( sub.begin(), sub.end(), sub.begin(), ::tolower );

            if ( !sub.empty() && name.find( sub ) == std::string::npos )
            {
                found = false;
                break;
            }
        }

        // if the device name contains p->nameHas
        if ( found )
        {
            if ( p->hasInputs && GetDeviceInputCount( i ) == 0 )
            {
                continue;
            }
            else if ( !p->hasInputs && GetDeviceInputCount( i ) != 0 )
            {
                continue;
            }
            else if ( p->currentDevice.name != GetDeviceName( i ) )
            {
                SetDevice( i, p->loopback );
                p->callback( true );
            }
            p->notFoundNotified = false;
            return true;
        }
    }

    if ( p->defaultIfNotFound )
    {
        if ( p->hasInputs && !p->notFoundNotified )
        {
            int defaultInputDevice = GetDefaultInputDevice();
            if ( GetCurrentDevice() != defaultInputDevice && GetDeviceInputCount( defaultInputDevice ) < 32 )
            {
                p->notFoundNotified = true;
                for ( auto const& sub : p->nameHas )
                {
                    std::cout << sub << " ";
                }
                std::cout << "input device not found." << std::endl;
                std::cout << "Switching to default input device: " << GetDeviceName( defaultInputDevice ) << std::endl;
                SetDevice( defaultInputDevice, p->loopback );
                p->callback( true );
                return true;
            }
        }
        else if ( !p->hasInputs && !p->notFoundNotified )
        {
            int defaultOutputDevice = GetDefaultOutputDevice();
            if ( GetCurrentDevice() != defaultOutputDevice && GetDeviceOutputCount( defaultOutputDevice ) < 32 )
            {
                p->notFoundNotified = true;
                for ( auto const& sub : p->nameHas )
                {
                    std::cout << sub << " ";
                }
                std::cout << "output device not found." << std::endl;
                std::cout << "Switching to default output device: " << GetDeviceName( defaultOutputDevice )
                          << std::endl;
                SetDevice( defaultOutputDevice, p->loopback );
                p->callback( true );
                return true;
            }
        }

        // no good default was found, try the first x:y device
        for ( int i = 0; i < GetDeviceCount(); ++i )
        {
            if ( i != GetDefaultOutputDevice() && i != GetDefaultInputDevice() &&
                 GetDeviceInputCount( i ) > 0 && GetDeviceOutputCount( i ) > 0 )
            {
                if ( GetCurrentDevice() != i )
                {
                    p->notFoundNotified = true;
                    for ( auto const& sub : p->nameHas )
                    {
                        std::cout << sub << " ";
                    }
                    std::cout << ( p->hasInputs ? "input device" : "output device" ) << " not found. " << std::endl;
                    std::cout << "Switching to first x:y device: " << GetDeviceName( i ) << std::endl;
                    SetDevice( i, p->loopback );
                    p->callback( true );
                    return true;
                }
            }
        }
    }

    if ( !p->notFoundNotified )
    {
        p->notFoundNotified = true;
        for ( auto const& sub : p->nameHas )
        {
            std::cout << sub << " ";
        }
        std::cout << ( p->hasInputs ? "input device" : "output device" ) << " not found. " << std::endl;

        SetDevice( -1, false );
        p->callback( false );
    }

    return false;
}

bool AudioDevice::SetDevice( int deviceIndex, bool loopback )
{
    if ( deviceIndex == -1 )
    {
        p->StopStream();
        p->currentDeviceIndex = deviceIndex;
        p->currentDevice = RtAudio::DeviceInfo();

        return true;
    }
    else if ( deviceIndex >= 0 && deviceIndex < GetDeviceCount() )
    {
        std::cout << "Initialising " << GetDeviceName( deviceIndex ) << "..." << std::endl;

        p->StopStream();

        p->currentDeviceIndex = deviceIndex;
        p->currentDevice = p->deviceList[deviceIndex];

        // configure outputParams
        p->outputParams.nChannels = p->deviceList[deviceIndex].outputChannels;
        p->outputParams.deviceId = deviceIndex;

        p->outputChannels.resize( p->outputParams.nChannels );
        SetInputCount_( p->outputParams.nChannels );

        // configure inputParams
        if ( loopback )
        {
            p->inputParams.nChannels = p->deviceList[deviceIndex].outputChannels;
            p->inputParams.deviceId = deviceIndex;

            p->inputChannels.resize( p->inputParams.nChannels );
            SetOutputCount_( p->inputParams.nChannels );
        }
        else
        {
            p->inputParams.nChannels = p->deviceList[deviceIndex].inputChannels;
            p->inputParams.deviceId = deviceIndex;

            p->inputChannels.resize( p->inputParams.nChannels );
            SetOutputCount_( p->inputParams.nChannels );
        }

        // the stream is started again via SetBufferSize()
        SetBufferSize( GetBufferSize() );

        return true;
    }

    return false;
}

bool AudioDevice::SetDevice( bool isOutputDevice,
                             std::vector<std::string> const& deviceNameHas,
                             bool defaultIfNotFound,
                             bool loopback )
{
    std::lock_guard<std::mutex> processLock( p->processMutex );

    {
        std::lock_guard<std::mutex> availableLock( p->availableMutex );

        if ( p->hasInputs == !isOutputDevice && p->nameHas == deviceNameHas &&
             p->defaultIfNotFound == defaultIfNotFound && p->loopback == loopback )
        {
            // Device already set, don't re-set unneccesarily
            return true;
        }

        p->hasInputs = !isOutputDevice;
        p->nameHas = deviceNameHas;
        p->defaultIfNotFound = defaultIfNotFound;
        p->loopback = loopback;
    }

    return Available();
}

bool AudioDevice::ReloadDevices()
{
    std::vector<RtAudio::DeviceInfo> newDeviceList;
    auto deviceCount = p->audioStream.getDeviceCount();

    bool devicesChanged = false;

    for ( unsigned int i = 0; i < deviceCount; ++i )
    {
        newDeviceList.emplace_back( p->audioStream.getDeviceInfo( i ) );

        if ( i >= p->deviceList.size() || newDeviceList[i].name != p->deviceList[i].name )
        {
            devicesChanged = true;
        }
    }

    if ( devicesChanged )
    {
        p->deviceList = newDeviceList;
        return true;
    }

    return false;
}

std::string AudioDevice::GetDeviceName( int deviceIndex ) const
{
    if ( deviceIndex >= 0 && deviceIndex < GetDeviceCount() )
    {
        return p->deviceList[deviceIndex].name;
    }

    return "";
}

int AudioDevice::GetDeviceInputCount( int deviceIndex ) const
{
    return p->deviceList[deviceIndex].inputChannels;
}

int AudioDevice::GetDeviceOutputCount( int deviceIndex ) const
{
    return p->deviceList[deviceIndex].outputChannels;
}

int AudioDevice::GetDefaultInputDevice() const
{
    return p->audioStream.getDefaultInputDevice();
}

int AudioDevice::GetDefaultOutputDevice() const
{
    return p->audioStream.getDefaultOutputDevice();
}

int AudioDevice::GetCurrentDevice() const
{
    return p->currentDeviceIndex;
}

int AudioDevice::GetDeviceCount() const
{
    return p->deviceList.size();
}

void AudioDevice::SetBufferSize( int bufferSize )
{
    p->StopStream();

    p->bufferSize = bufferSize;
    for ( auto& inputChannel : p->inputChannels )
    {
        inputChannel.resize( bufferSize );
    }

    p->StartStream();
}

void AudioDevice::SetSampleRate( int sampleRate )
{
    p->StopStream();
    p->sampleRate = sampleRate;
    p->StartStream();
}

bool AudioDevice::IsStreaming() const
{
    return p->isStreaming;
}

int AudioDevice::GetBufferSize() const
{
    return p->bufferSize;
}

int AudioDevice::GetSampleRate() const
{
    return p->sampleRate;
}

void AudioDevice::ShowWarnings( bool enabled )
{
    p->audioStream.showWarnings( enabled );
}

void AudioDevice::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    std::lock_guard<std::mutex> processLock( p->processMutex );

    // Wait until the sound card is ready for the next set of buffers
    // ==============================================================
    {
        std::unique_lock<std::mutex> lock( p->syncMutex );
        if ( !p->gotSyncReady )  // if haven't already got the release
        {
            if ( p->syncCondt.wait_for( lock, std::chrono::milliseconds( c_bufferWaitTimeoutMs ) ) ==
                 std::cv_status::timeout )
            {
                lock.unlock();
                if ( !Available() && IsStreaming() )
                {
                    std::cout << p->currentDevice.name << " disconnected." << std::endl;
                }
                return;
            }
        }
        p->gotSyncReady = false;  // reset the release flag
    }

    // Synchronise buffer size with the size of incoming buffers
    // =========================================================
    auto buffer = inputs.GetValue<std::vector<short>>( 0 );
    if ( buffer )
    {
        if ( GetBufferSize() != (int)buffer->size() && !buffer->empty() )
        {
            SetBufferSize( buffer->size() );
            return;
        }
    }

    // Retrieve incoming component buffers for the sound card to output
    // ================================================================
    for ( size_t i = 0; i < p->outputChannels.size(); ++i )
    {
        buffer = inputs.GetValue<std::vector<short>>( i );
        if ( buffer )
        {
            p->outputChannels[i] = *buffer;
        }
        else
        {
            p->outputChannels[i].assign( p->bufferSize, 0 );
        }
    }

    // Retrieve incoming sound card buffers for the component to output
    // ================================================================
    for ( size_t i = 0; i < p->inputChannels.size(); ++i )
    {
        outputs.SetValue( i, p->inputChannels[i] );
    }

    // Inform the sound card that buffers are now ready
    // ================================================
    std::lock_guard<std::mutex> lock( p->buffersMutex );
    p->gotWaitReady = true;     // set release flag
    p->waitCondt.notify_all();  // release sync
}

void DSPatchables::internal::AudioDevice::WaitForBuffer()
{
    std::unique_lock<std::mutex> lock( buffersMutex );
    if ( !gotWaitReady )  // if haven't already got the release
    {
        waitCondt.wait_for( lock, std::chrono::seconds( c_syncWaitTimeoutS ) );  // wait for sync
    }
    gotWaitReady = false;  // reset the release flag
}

void DSPatchables::internal::AudioDevice::SyncBuffer()
{
    std::lock_guard<std::mutex> lock( syncMutex );
    gotSyncReady = true;     // set release flag
    syncCondt.notify_all();  // release sync
}

void DSPatchables::internal::AudioDevice::StopStream()
{
    isStreaming = false;

    buffersMutex.lock();
    gotWaitReady = true;     // set release flag
    waitCondt.notify_all();  // release sync
    buffersMutex.unlock();

    if ( audioStream.isStreamOpen() )
    {
        std::lock_guard<std::mutex> lock( processMutex );  // wait for Process_() to exit
        audioStream.closeStream();
    }
}

void DSPatchables::internal::AudioDevice::StartStream()
{
    RtAudio::StreamParameters* inParams = nullptr;
    RtAudio::StreamParameters* outParams = nullptr;

    if ( inputParams.nChannels != 0 )
    {
        inParams = &inputParams;
    }

    if ( outputParams.nChannels != 0 )
    {
        outParams = &outputParams;
    }

    RtAudio::StreamOptions options;
#ifdef NDEBUG
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;
#endif
    options.flags |= RTAUDIO_NONINTERLEAVED;

    audioStream.openStream( outParams, inParams, RTAUDIO_SINT16, sampleRate, (unsigned int*)&bufferSize,
                            &StaticCallback, this, &options );

    isStreaming = true;

    audioStream.startStream();
}

int DSPatchables::internal::AudioDevice::StaticCallback(
    void* outputBuffer, void* inputBuffer, unsigned int, double, RtAudioStreamStatus status, void* userData )
{
    return ( reinterpret_cast<AudioDevice*>( userData ) )->DynamicCallback( inputBuffer, outputBuffer, status );
}

int DSPatchables::internal::AudioDevice::DynamicCallback( void* inputBuffer, void* outputBuffer, RtAudioStreamStatus )
{
    WaitForBuffer();

    if ( isStreaming )
    {
        short* shortOutput = (short*)outputBuffer;
        short* shortInput = (short*)inputBuffer;

        if ( outputBuffer != nullptr )
        {
            for ( auto& outputChannel : outputChannels )
            {
                if ( !outputChannel.empty() )
                {
                    memcpy( shortOutput, &outputChannel[0], outputChannel.size() * sizeof( short ) );
                    shortOutput += outputChannel.size();
                }
            }
        }

        if ( inputBuffer != nullptr )
        {
            for ( auto& inputChannel : inputChannels )
            {
                if ( !inputChannel.empty() )
                {
                    memcpy( &inputChannel[0], shortInput, inputChannel.size() * sizeof( short ) );
                    shortInput += inputChannel.size();
                }
            }
        }
    }
    else
    {
        SyncBuffer();
        return 1;
    }

    SyncBuffer();
    return 0;
}
