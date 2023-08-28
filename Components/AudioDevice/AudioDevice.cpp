/******************************************************************************
AudioDevice DSPatch Component
Copyright (c) 2022, Marcus Tomlinson

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
        deviceList = audioStream.getDeviceIds();
    }

    void WaitForBuffer();
    void SyncBuffer();

    void StopStream();
    void StartStream();

    static int StaticCallback(
        void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime, unsigned int status, void* userData );

    int DynamicCallback( void* inputBuffer, void* outputBuffer, RtAudioStreamStatus status );

    static void SanitizeDeviceName( std::string& name )
    {
        std::replace_if(
            name.begin(), name.end(), []( char c ) { return c == static_cast<char>( '\xd5' ); }, '\'' );

        std::replace_if(
            name.begin(), name.end(), []( char c ) { return !std::isprint( static_cast<unsigned char>( c ) ); }, '_' );
    }

    std::vector<std::vector<short>> outputChannels;
    std::vector<std::vector<short>> inputChannels;

    std::mutex buffersMutex;
    std::mutex syncMutex;
    std::condition_variable waitCondt;
    std::condition_variable syncCondt;
    bool gotWaitReady = false;
    bool gotSyncReady = true;

    unsigned int currentDeviceId = 0;
    bool isStreaming = false;
    unsigned int bufferSize = c_bufferSize;
    unsigned int sampleRate = c_sampleRate;

    std::vector<unsigned int> deviceList;
    RtAudio::DeviceInfo currentDevice;

    RtAudio audioStream;
    RtAudio::StreamParameters outputParams;
    RtAudio::StreamParameters inputParams;

    std::mutex availableMutex;
    std::mutex processMutex;
    bool isInputDevice = false;
    std::vector<std::string> nameHas;
    bool defaultIfNotFound = false;
    bool loopback = false;
    bool notFoundNotified = false;

    std::function<void( bool )> callback = []( bool ) {};
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

AudioDevice::AudioDevice( bool isOutputDevice, std::vector<std::string> const& deviceNameHas, bool defaultIfNotFound, bool loopback )
    : p( new internal::AudioDevice() )
{
    SetDevice( isOutputDevice, deviceNameHas, defaultIfNotFound, loopback );
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

    for ( const auto id : p->deviceList )
    {
        auto name = GetDeviceName( id );

        // check if the device name contains p->nameHas
        bool found = true;
        for ( const auto& sub : p->nameHas )
        {
            if ( !sub.empty() && name.find( sub ) == std::string::npos )
            {
                found = false;
                break;
            }
        }

        // if the device name contains p->nameHas
        if ( found )
        {
            if ( p->isInputDevice && GetDeviceInputCount( id ) == 0 )
            {
                continue;
            }
            else if ( !p->isInputDevice && GetDeviceOutputCount( id ) == 0 )
            {
                continue;
            }
            else if ( p->currentDevice.name != GetDeviceName( id ) )
            {
                SetDevice( id, p->loopback );
                p->callback( true );
            }
            p->notFoundNotified = false;
            return true;
        }
    }

    if ( p->defaultIfNotFound )
    {
        if ( p->isInputDevice && !p->notFoundNotified )
        {
            auto defaultInputDevice = GetDefaultInputDevice();
            if ( GetCurrentDevice() != defaultInputDevice )
            {
                p->notFoundNotified = true;

                std::stringstream log;
                for ( auto const& sub : p->nameHas )
                {
                    log << sub << " ";
                }
                log << "input device not found. Switching to default input device: " << GetDeviceName( defaultInputDevice );
                std::cout << log.str() << std::endl;

                SetDevice( defaultInputDevice, p->loopback );
            }
        }
        else if ( !p->isInputDevice && !p->notFoundNotified )
        {
            auto defaultOutputDevice = GetDefaultOutputDevice();
            if ( GetCurrentDevice() != defaultOutputDevice )
            {
                p->notFoundNotified = true;

                std::stringstream log;
                for ( auto const& sub : p->nameHas )
                {
                    log << sub << " ";
                }
                log << "output device not found. Switching to default output device: " << GetDeviceName( defaultOutputDevice );
                std::cout << log.str() << std::endl;

                SetDevice( defaultOutputDevice, p->loopback );
            }
        }
        return true;
    }

    if ( !p->notFoundNotified )
    {
        p->notFoundNotified = true;

        std::stringstream log;
        for ( auto const& sub : p->nameHas )
        {
            log << sub << " ";
        }
        log << ( p->isInputDevice ? "input device" : "output device" ) << " not found.";
        std::cout << log.str() << std::endl;

        SetDevice( 0, false );
        p->callback( false );
    }

    return false;
}

bool AudioDevice::SetDevice( unsigned int deviceId, bool loopback )
{
    if ( deviceId == 0 )
    {
        p->StopStream();
        p->currentDeviceId = deviceId;
        p->currentDevice = RtAudio::DeviceInfo();

        return true;
    }
    else if ( std::find( p->deviceList.begin(), p->deviceList.end(), deviceId ) != p->deviceList.end() )
    {
        auto sampleRates = p->audioStream.getDeviceInfo( deviceId ).sampleRates;
        if ( std::find( sampleRates.begin(), sampleRates.end(), p->sampleRate ) == sampleRates.end() )
        {
            std::cout << "Sample rate " << std::to_string( p->sampleRate ) << " not supported. Switching to "
                      << std::to_string( sampleRates.back() ) << std::endl;
            p->sampleRate = sampleRates.back();
        }

        std::cout << "Initialising " << GetDeviceName( deviceId ) << std::endl;

        p->StopStream();

        p->currentDeviceId = deviceId;
        p->currentDevice = p->audioStream.getDeviceInfo( deviceId );
        p->SanitizeDeviceName( p->currentDevice.name );

        // configure outputParams
        p->outputParams.nChannels = p->currentDevice.outputChannels;
        p->outputParams.deviceId = deviceId;

        p->outputChannels.resize( p->outputParams.nChannels );
        SetInputCount_( p->outputParams.nChannels );

        // configure inputParams
        if ( loopback )
        {
            p->inputParams.nChannels = p->currentDevice.outputChannels;
            p->inputParams.deviceId = deviceId;

            p->inputChannels.resize( p->inputParams.nChannels );
            SetOutputCount_( p->inputParams.nChannels );
        }
        else
        {
            p->inputParams.nChannels = p->currentDevice.inputChannels;
            p->inputParams.deviceId = deviceId;

            p->inputChannels.resize( p->inputParams.nChannels );
            SetOutputCount_( p->inputParams.nChannels );
        }

        // the stream is started again via SetBufferSize()
        SetBufferSize( GetBufferSize() );

        return true;
    }

    return false;
}

bool AudioDevice::SetDevice( bool isOutputDevice, std::vector<std::string> const& deviceNameHas, bool defaultIfNotFound, bool loopback )
{
    {
        std::lock_guard<std::mutex> processLock( p->processMutex );
        std::lock_guard<std::mutex> availableLock( p->availableMutex );

        if ( p->isInputDevice == !isOutputDevice && p->nameHas == deviceNameHas && p->defaultIfNotFound == defaultIfNotFound &&
             p->loopback == loopback )
        {
            // Device already set, don't re-set unneccesarily
            return true;
        }

        p->isInputDevice = !isOutputDevice;
        p->defaultIfNotFound = defaultIfNotFound;
        p->loopback = loopback;

        if ( !deviceNameHas.empty() )
        {
            p->nameHas = deviceNameHas;
        }
        else
        {
            if ( isOutputDevice )
            {
                p->nameHas = { GetDeviceName( GetDefaultOutputDevice() ) };
            }
            else
            {
                p->nameHas = { GetDeviceName( GetDefaultInputDevice() ) };
            }
        }
    }

    return Available();
}

bool AudioDevice::ReloadDevices()
{
    auto newDeviceList = p->audioStream.getDeviceIds();

    bool devicesChanged = false;
    for ( size_t i = 0; i < newDeviceList.size(); ++i )
    {
        if ( i >= p->deviceList.size() || newDeviceList[i] != p->deviceList[i] )
        {
            devicesChanged = true;
            break;
        }
    }

    if ( devicesChanged )
    {
        p->deviceList = newDeviceList;
        return true;
    }

    return false;
}

std::string AudioDevice::GetDeviceName( unsigned int deviceId ) const
{
    auto deviceName = p->audioStream.getDeviceInfo( deviceId ).name;
    p->SanitizeDeviceName( deviceName );
    return deviceName;
}

unsigned int AudioDevice::GetDeviceInputCount( unsigned int deviceId ) const
{
    return p->audioStream.getDeviceInfo( deviceId ).inputChannels;
}

unsigned int AudioDevice::GetDeviceOutputCount( unsigned int deviceId ) const
{
    return p->audioStream.getDeviceInfo( deviceId ).outputChannels;
}

std::vector<unsigned int> AudioDevice::GetSampleRates( unsigned int deviceId ) const
{
    return p->audioStream.getDeviceInfo( deviceId ).sampleRates;
}

std::vector<unsigned int> AudioDevice::GetDeviceIds() const
{
    return p->deviceList;
}

unsigned int AudioDevice::GetDefaultInputDevice() const
{
    return p->audioStream.getDefaultInputDevice();
}

unsigned int AudioDevice::GetDefaultOutputDevice() const
{
    return p->audioStream.getDefaultOutputDevice();
}

unsigned int AudioDevice::GetCurrentDevice() const
{
    return p->currentDeviceId;
}

unsigned int AudioDevice::SetBufferSize( unsigned int bufferSize )
{
    p->StopStream();

    p->bufferSize = bufferSize;
    for ( auto& inputChannel : p->inputChannels )
    {
        inputChannel.resize( bufferSize );
    }

    p->StartStream();
    return p->bufferSize;
}

bool AudioDevice::SetSampleRate( unsigned int sampleRate )
{
    if ( std::find( p->currentDevice.sampleRates.begin(), p->currentDevice.sampleRates.end(), sampleRate ) ==
         p->currentDevice.sampleRates.end() )
    {
        return false;
    }

    p->StopStream();
    p->sampleRate = sampleRate;
    p->StartStream();

    return true;
}

bool AudioDevice::IsStreaming() const
{
    return p->isStreaming;
}

unsigned int AudioDevice::GetBufferSize() const
{
    return p->bufferSize;
}

unsigned int AudioDevice::GetSampleRate() const
{
    return p->sampleRate;
}

void AudioDevice::ShowWarnings( bool enabled )
{
    p->audioStream.showWarnings( enabled );
}

void AudioDevice::Process_( SignalBus& inputs, SignalBus& outputs )
{
    std::unique_lock<std::mutex> processLock( p->processMutex );

    // Wait until the sound card is ready for the next set of buffers
    // ==============================================================
    {
        std::unique_lock<std::mutex> lock( p->syncMutex );
        if ( !p->gotSyncReady )  // if haven't already got the release
        {
            if ( p->syncCondt.wait_for( lock, std::chrono::milliseconds( c_bufferWaitTimeoutMs ) ) == std::cv_status::timeout )
            {
                lock.unlock();
                processLock.unlock();
                if ( !Available() && IsStreaming() )
                {
                    std::cout << p->currentDevice.name << " disconnected." << std::endl;
                }
                return;
            }
        }
        p->gotSyncReady = false;  // reset the release flag
    }

    // Retrieve incoming component buffers for the sound card to output
    // ================================================================
    const size_t outputCount = p->outputChannels.size();
    for ( size_t i = 0; i < outputCount; ++i )
    {
        auto buffer = inputs.GetValue<std::vector<short>>( (int)i );
        if ( buffer && buffer->size() == p->bufferSize )
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
    const size_t inputCount = p->inputChannels.size();
    for ( size_t i = 0; i < inputCount; ++i )
    {
        outputs.SetValue( (int)i, p->inputChannels[i] );
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
    options.flags = RTAUDIO_NONINTERLEAVED;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

    audioStream.openStream( outParams, inParams, RTAUDIO_SINT16, sampleRate, &bufferSize, &StaticCallback, this, &options );

    isStreaming = true;

    audioStream.startStream();
}

int DSPatchables::internal::AudioDevice::StaticCallback(
    void* outputBuffer, void* inputBuffer, unsigned int, double, RtAudioStreamStatus status, void* userData )
{
    return ( static_cast<AudioDevice*>( userData ) )->DynamicCallback( inputBuffer, outputBuffer, status );
}

int DSPatchables::internal::AudioDevice::DynamicCallback( void* inputBuffer, void* outputBuffer, RtAudioStreamStatus )
{
    WaitForBuffer();

    if ( isStreaming )
    {
        short* shortOutput = static_cast<short*>( outputBuffer );
        short* shortInput = static_cast<short*>( inputBuffer );

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

    SyncBuffer();
    return 0;
}
