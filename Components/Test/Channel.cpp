#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Channel final : public Component
{
public:
    Channel()
    {
        SetInputCount_( 3, { "leftIn", "rightIn", "clockIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( Channel )

}  // namespace DSPatch
