#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Metronome final : public Component
{
public:
    Metronome()
    {
        SetInputCount_( 1, { "clockIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
};

EXPORT_PLUGIN( Metronome )

}  // namespace DSPatch
