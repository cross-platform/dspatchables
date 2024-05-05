#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Transport final : public Component
{
public:
    Transport()
    {
        SetOutputCount_( 1, { "clockOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
};

EXPORT_PLUGIN( Transport )

}  // namespace DSPatch
