#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT SoloMute final : public Component
{
public:
    SoloMute()
    {
        SetInputCount_( 2, { "leftIn", "rightIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override
    {
        outputs.MoveSignal( 0, *inputs.GetSignal( 0 ) );
        outputs.MoveSignal( 1, *inputs.GetSignal( 1 ) );
    }
};

EXPORT_PLUGIN( SoloMute )

}  // namespace DSPatch
