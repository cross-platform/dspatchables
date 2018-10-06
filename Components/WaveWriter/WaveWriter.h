/// WaveWriter v1.0.1

#pragma once

#include <DSPatch.h>

namespace DSPatch
{
namespace DSPatchables
{

namespace internal
{
class WaveWriter;
}

class DLLEXPORT WaveWriter final : public Component
{
public:
    WaveWriter( std::string const& fileName, int channelCount, int bitsPerSample, int sampleRate );
    virtual ~WaveWriter();

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

private:
    std::unique_ptr<internal::WaveWriter> p;
};

EXPORT_PLUGIN( WaveWriter, "WaveWriter.wav", 2, 16, 44100 )

}  // namespace DSPatchables
}  // namespace DSPatch
