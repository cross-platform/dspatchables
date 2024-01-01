/******************************************************************************
WaveReader DSPatch Component
Copyright (c) 2024, Marcus Tomlinson

BSD 2-Clause License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#include <Constants.h>
#include <WaveReader.h>

#include <cstring>
#include <fstream>
#include <iostream>

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class WaveReader
{
public:
    explicit WaveReader( std::string const& fileName )
    {
        streamData.resize( bufferSize );

        std::ifstream inFile( fileName, std::ios::binary | std::ios::in );

        if ( inFile.bad() )
        {
            return;
        }

        int dwFileSize = 0, dwChunkSize = 0;
        char dwChunkId[5];
        char dwExtra[5];

        dwChunkId[4] = 0;
        dwExtra[4] = 0;

        // look for 'RIFF' chunk identifier
        inFile.seekg( 0, std::ios::beg );
        inFile.read( dwChunkId, 4 );
        if ( strcmp( dwChunkId, "RIFF" ) )
        {
            std::cerr << "'" << fileName.c_str() << "' not found." << std::endl;
            inFile.close();
            return;
        }
        inFile.seekg( 4, std::ios::beg );  // get file size
        inFile.read( reinterpret_cast<char*>( &dwFileSize ), 4 );
        if ( dwFileSize <= 16 )
        {
            inFile.close();
            return;
        }
        inFile.seekg( 8, std::ios::beg );  // get file format
        inFile.read( dwExtra, 4 );
        if ( strcmp( dwExtra, "WAVE" ) )
        {
            inFile.close();
            return;
        }

        // look for 'fmt ' chunk id
        bool bFilledFormat = false;
        for ( int i = 12; i < dwFileSize; )
        {
            inFile.seekg( i, std::ios::beg );
            inFile.read( dwChunkId, 4 );
            inFile.seekg( i + 4, std::ios::beg );
            inFile.read( reinterpret_cast<char*>( &dwChunkSize ), 4 );
            if ( !strcmp( dwChunkId, "fmt " ) )
            {
                inFile.seekg( i + 8, std::ios::beg );

                inFile.read( reinterpret_cast<char*>( &waveFormat.format ), 2 );
                inFile.read( reinterpret_cast<char*>( &waveFormat.channelCount ), 2 );
                inFile.read( reinterpret_cast<char*>( &waveFormat.sampleRate ), 4 );
                inFile.read( reinterpret_cast<char*>( &waveFormat.byteRate ), 4 );
                inFile.read( reinterpret_cast<char*>( &waveFormat.frameSize ), 2 );
                inFile.read( reinterpret_cast<char*>( &waveFormat.bitDepth ), 2 );
                inFile.read( reinterpret_cast<char*>( &waveFormat.extraDataSize ), 2 );

                bFilledFormat = true;
                break;
            }
            dwChunkSize += 8;  // add offsets of the chunk id, and chunk size data entries
            dwChunkSize += 1;
            dwChunkSize &= 0xfffffffe;  // guarantees WORD padding alignment
            i += dwChunkSize;
        }
        if ( !bFilledFormat )
        {
            inFile.close();
            return;
        }

        // look for 'data' chunk id
        bool bFilledData = false;
        for ( int i = 12; i < dwFileSize; )
        {
            inFile.seekg( i, std::ios::beg );
            inFile.read( dwChunkId, 4 );
            inFile.seekg( i + 4, std::ios::beg );
            inFile.read( reinterpret_cast<char*>( &dwChunkSize ), 4 );
            if ( !strcmp( dwChunkId, "data" ) )
            {
                waveData.resize( dwChunkSize / 2 );
                inFile.seekg( i + 8, std::ios::beg );
                inFile.read( reinterpret_cast<char*>( &waveData[0] ), dwChunkSize );
                bFilledData = true;
                break;
            }
            dwChunkSize += 8;  // add offsets of the chunk id, and chunk size data entries
            dwChunkSize += 1;
            dwChunkSize &= 0xfffffffe;  // guarantees WORD padding alignment
            i += dwChunkSize;
        }
        if ( !bFilledData )
        {
            inFile.close();
            return;
        }

        inFile.close();
    }

    struct WaveFormat
    {
        unsigned short format = 0;         // Integer identifier of the format
        unsigned short channelCount = 0;   // Number of audio channels
        unsigned long sampleRate = 0;      // Audio sample rate
        unsigned long byteRate = 0;        // Bytes per second (possibly approximate)
        unsigned short frameSize = 0;      // Size in bytes of a sample block (all channels)
        unsigned short bitDepth = 0;       // Size in bits of a single per-channel sample
        unsigned short extraDataSize = 0;  // Bytes of extra data appended to this struct
    };

    size_t bufferSize = c_bufferSize;
    int sampleIndex = 0;

    WaveFormat waveFormat;
    std::vector<short> waveData;
    std::vector<short> streamData;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

WaveReader::WaveReader( std::string const& fileName )
    : p( new internal::WaveReader( fileName ) )
{
    SetOutputCount_( p->waveFormat.channelCount );
}

void WaveReader::Process_( SignalBus&, SignalBus& outputs )
{
    if ( p->waveData.size() <= p->bufferSize )
    {
        return;
    }

    size_t waveBufferSize = p->bufferSize * p->waveFormat.channelCount;

    for ( auto ch = 0; ch < p->waveFormat.channelCount; ++ch )
    {
        int index = 0;
        for ( size_t i = ch; i < waveBufferSize; i += p->waveFormat.channelCount )
        {
            p->streamData[index++] = p->waveData[p->sampleIndex + i];
        }

        outputs.SetValue( ch, p->streamData );
    }

    p->sampleIndex += waveBufferSize;
    p->sampleIndex %= p->waveData.size() - waveBufferSize;
}
