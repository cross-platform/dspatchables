#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT MidiIn final : public Component
{
public:
    MidiIn()
    {
        SetInputCount_( 1, { "clockIn" } );
        SetOutputCount_( 1, { "midiOut" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
    }
};

EXPORT_PLUGIN( MidiIn )

}  // namespace DSPatch
