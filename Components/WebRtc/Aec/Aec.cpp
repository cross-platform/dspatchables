/******************************************************************************
Aec DSPatch Component
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

#include <Aec.h>
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

class Aec
{
public:
    Aec()
        : apm( webrtc::AudioProcessing::Create() )
    {
        apm->Initialize();
        apm->echo_cancellation()->Enable( true );

        apm->set_stream_delay_ms( c_initialStreamDelay );

        farFrame.sample_rate_hz_ = c_sampleRate;
        farFrame.num_channels_ = c_channelCount;

        nearFrame.sample_rate_hz_ = c_sampleRate;
        nearFrame.num_channels_ = c_channelCount;
    }

    int lastDelay = -1;
    int medianDelay = 0;
    int stdDelay = 0;

    std::shared_ptr<webrtc::AudioProcessing> apm;

    webrtc::AudioFrame farFrame;
    webrtc::AudioFrame nearFrame;
    std::vector<short> outputStream;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

Aec::Aec()
    : p( new internal::Aec() )
{
    // 2 Inputs (Near and Far inputs)
    SetInputCount_( 2, { "near", "far" } );

    // 1 Output (Echo-free output)
    SetOutputCount_( 1, { "out" } );
}

void Aec::Process_( SignalBus& inputs, SignalBus& outputs )
{
    auto nearSignal = inputs.GetValue<std::vector<short>>( 0 );
    auto farSignal = inputs.GetValue<std::vector<short>>( 1 );

    // if we only have a nearSignal...
    if ( nearSignal && !farSignal )
    {
        outputs.MoveSignal( 0, *inputs.GetSignal( 0 ) );  // ...move just the nearSignal to output
        return;
    }
    else if ( !nearSignal || !farSignal )
    {
        return;
    }

    p->farFrame.samples_per_channel_ = farSignal->size();
    memcpy( p->farFrame.data_, &( *farSignal )[0], farSignal->size() * sizeof( short ) );

    p->apm->ProcessReverseStream( &p->farFrame );

    p->nearFrame.samples_per_channel_ = nearSignal->size();
    memcpy( p->nearFrame.data_, &( *nearSignal )[0], nearSignal->size() * sizeof( short ) );

    p->apm->echo_cancellation()->GetDelayMetrics( &p->medianDelay, &p->stdDelay );
    if ( p->lastDelay != p->medianDelay )
    {
        p->lastDelay = p->medianDelay;
        int newDelay = p->apm->stream_delay_ms() + p->medianDelay;

        // set_stream_delay_ms() allows max 500ms
        newDelay %= 500;
        p->apm->set_stream_delay_ms( newDelay );
    }
    else
    {
        p->apm->set_stream_delay_ms( p->apm->stream_delay_ms() );
    }

    p->apm->ProcessStream( &p->nearFrame );

    p->outputStream.resize( nearSignal->size() );
    memcpy( &p->outputStream[0], p->nearFrame.data_, nearSignal->size() * sizeof( short ) );

    outputs.SetValue( 0, p->outputStream );
}
