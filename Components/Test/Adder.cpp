#include <DSPatch.h>

namespace DSPatch
{

class DLLEXPORT Adder final : public Component
{
public:
    Adder()
    {
        SetInputCount_( 2, { "in1", "in2" } );
        SetOutputCount_( 1, { "out" } );
    }

    virtual void Process_( SignalBus&, SignalBus& ) override
    {
        std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
    }
};

EXPORT_PLUGIN( Adder )

}  // namespace DSPatch
