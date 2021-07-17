#pragma once

#include <DSPatch.h>

#include <thread>

class VstLoader;
struct AEffect;
struct VstEvents;

namespace DSPatch
{
namespace DSPatchables
{

class DLLEXPORT VstHost final : public Component
{
public:
    VstHost();
    ~VstHost();

    bool LoadVst( const char* vstPath );
    void CloseVst();
    bool ShowVst();
    void HideVst();

    void ProcessMidi( VstEvents* events );

protected:
    virtual void Process_( SignalBus const& inputs, SignalBus& outputs ) override;

private:
    void _Run();

    std::thread _thread;

    float** _vstInputs;
    float** _vstOutputs;
    std::vector<std::vector<float>> _dspInputs;
    std::vector<std::vector<float>> _dspOutputs;
    unsigned short _bufferSize;

    VstLoader* _vstLoader;
    AEffect* _vstEffect;
};

EXPORT_PLUGIN( VstHost )

}  // namespace DSPatchables
}  // namespace DSPatch
