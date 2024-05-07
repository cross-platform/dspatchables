#include <DSPatch.h>

#include "../Constants.h"

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

    virtual void Process_( SignalBus&, SignalBus& ouputs ) override
    {
        ouputs.SetValue(0, silentBuffer);
        ouputs.SetValue(1, silentBuffer);
    }

    std::vector<short> silentBuffer = std::vector<short>( c_bufferSize, 0 );
};

EXPORT_PLUGIN( Instrument )

}  // namespace DSPatch
