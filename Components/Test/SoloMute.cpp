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
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
};

EXPORT_PLUGIN( SoloMute )

}  // namespace DSPatch
