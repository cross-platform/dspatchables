#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT AudioTrack final : public Component
{
public:
    AudioTrack()
    {
        SetInputCount_( 1, { "clockIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( AudioTrack )

}  // namespace DSPatch
