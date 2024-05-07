#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Effect final : public Component
{
public:
    Effect()
    {
        SetInputCount_( 3, { "leftIn", "rightIn", "clockIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override
    {
        outputs.MoveSignal( 0, *inputs.GetSignal( 0 ) );
        outputs.MoveSignal( 1, *inputs.GetSignal( 1 ) );
    }
};

EXPORT_PLUGIN( Effect )

}  // namespace DSPatch
