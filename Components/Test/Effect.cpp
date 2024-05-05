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

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( Effect )

}  // namespace DSPatch
