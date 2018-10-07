#include <WaveWriter.h>

#include <cmath>
#include <fstream>
#include <iostream>

using namespace DSPatch;
using namespace DSPatchables;

template <typename Word>
std::ostream& write_word( std::ostream& outs, Word value, unsigned size = sizeof( Word ) )
{
    for ( ; size; --size, value >>= 8 )
    {
        outs.put( static_cast<char>( value & 0xFF ) );
    }
    return outs;
}

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class WaveWriter
{
public:
    WaveWriter( std::string const& fileName, int channelCount )
        : file( fileName, std::ios::binary )
        , channelCount( channelCount )
    {
    }

    std::ofstream file;
    size_t dataPos;
    int channelCount;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

WaveWriter::WaveWriter( std::string const& fileName, int channelCount, int bitsPerSample, int sampleRate )
    : p( new internal::WaveWriter( fileName, channelCount ) )
{
    SetInputCount_( channelCount );

    // Write the file headers
    p->file << "RIFF----WAVEfmt ";                                                // (chunk size to be filled in later)
    write_word( p->file, 16, 4 );                                                 // no extension data
    write_word( p->file, 1, 2 );                                                  // PCM - integer samples
    write_word( p->file, channelCount, 2 );                                       // channel count
    write_word( p->file, sampleRate, 4 );                                         // samples per second (Hz)
    write_word( p->file, ( sampleRate * bitsPerSample * channelCount ) / 8, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( p->file, 2 * channelCount, 2 );  // data block size (size of one integer sample per each channel, in bytes)
    write_word( p->file, bitsPerSample, 2 );     // number of bits per sample (use a multiple of 8)

    // Write the data chunk header
    p->dataPos = (size_t)p->file.tellp();
    p->file << "data----";  // (chunk size to be filled in later)
}

WaveWriter::~WaveWriter()
{
    // (We'll need the final file size to fix the chunk sizes above)
    size_t file_length = (size_t)p->file.tellp();

    // Fix the data chunk header to contain the data size
    p->file.seekp( p->dataPos + 4 );
    write_word( p->file, file_length - p->dataPos + 8 );

    // Fix the file header to contain the proper RIFF chunk size, which is (file size - 8) bytes
    p->file.seekp( 0 + 4 );
    write_word( p->file, file_length - 8, 4 );
}

void WaveWriter::Process_( SignalBus const& inputs, SignalBus& )
{
    std::vector<std::vector<short>*> ins;
    for ( int i = 0; i < p->channelCount; ++i )
    {
        auto in = inputs.GetValue<std::vector<short>>( i );

        if ( !in )
        {
            return;  // input buffer missing
        }

        ins.push_back( in );
    }

    for ( int i = 0; i < p->channelCount - 1; ++i )
    {
        if ( ins[i]->size() != ins[i + 1]->size() )
        {
            return;  // input buffers are not the same size
        }
    }

    // write input to file
    for ( size_t j = 0; j < ins[0]->size(); ++j )
    {
        for ( int i = 0; i < p->channelCount; ++i )
        {
            write_word( p->file, ( *ins[i] )[j], 2 );  ///! assumes 16bit
        }
    }
}
