/******************************************************************************
Agc DSPatch Component
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

#include <Agc.h>
#include <Constants.h>

#include <webrtc/modules/audio_processing/include/audio_processing.h>
#include <webrtc/modules/interface/module_common_types.h>

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class Agc
{
public:
    explicit Agc( bool enableNoiseSuppression )
        : apm( webrtc::AudioProcessing::Create() )
    {
        apm->Initialize();
        apm->gain_control()->Enable( true );
        apm->gain_control()->set_mode( webrtc::GainControl::kAdaptiveDigital );
        apm->gain_control()->set_target_level_dbfs( c_dbfsLevel );
        apm->gain_control()->set_compression_gain_db( c_compressorGain );
        apm->gain_control()->enable_limiter( true );

        if ( enableNoiseSuppression )
        {
            apm->noise_suppression()->Enable( true );
            apm->noise_suppression()->set_level( webrtc::NoiseSuppression::kLow );
        }

        frame.sample_rate_hz_ = c_sampleRate;
        frame.num_channels_ = c_channelCount;
    }

    std::shared_ptr<webrtc::AudioProcessing> apm;

    webrtc::AudioFrame frame;
    std::vector<short> outputStream;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

Agc::Agc( bool enableNoiseSuppression )
    : p( new internal::Agc( enableNoiseSuppression ) )
{
    SetInputCount_( 1 );
    SetOutputCount_( 1 );
}

void Agc::Process_( SignalBus& inputs, SignalBus& outputs )
{
    auto in = inputs.GetValue<std::vector<short>>( 0 );

    if ( !in )
    {
        return;
    }

    p->frame.samples_per_channel_ = in->size();
    memcpy( p->frame.data_, &( *in )[0], in->size() * sizeof( short ) );

    p->apm->ProcessStream( &p->frame );

    p->outputStream.resize( in->size() );
    memcpy( &p->outputStream[0], p->frame.data_, in->size() * sizeof( short ) );

    outputs.SetValue( 0, p->outputStream );
}
