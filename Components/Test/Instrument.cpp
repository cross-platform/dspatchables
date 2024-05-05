#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Instrument final : public Component
{
public:
    Instrument()
    {
        SetInputCount_( 2, { "midiIn", "clockIn" } );
        SetOutputCount_( 2, { "leftOut", "rightOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( Instrument )

}  // namespace DSPatch
