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

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( SoloMute )

}  // namespace DSPatch
