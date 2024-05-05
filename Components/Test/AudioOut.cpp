#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT AudioOut final : public Component
{
public:
    AudioOut()
    {
        SetInputCount_( 2, { "leftIn", "rightIn" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
};

EXPORT_PLUGIN( AudioOut )

}  // namespace DSPatch
