#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Recorder final : public Component
{
public:
    Recorder()
    {
        SetInputCount_( 2, { "leftIn", "rightIn" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( Recorder )

}  // namespace DSPatch
