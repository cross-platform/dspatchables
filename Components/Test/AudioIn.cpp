#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT AudioIn final : public Component
{
public:
    AudioIn()
    {
        SetInputCount_( 1, { "clockIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( AudioIn )

}  // namespace DSPatch
