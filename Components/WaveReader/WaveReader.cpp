/************************************************************************
DSPatchables - DSPatch Component Repository
Copyright (c) 2014-2019 Marcus Tomlinson

This file is part of DSPatchables.

GNU Lesser General Public License Usage
This file may be used under the terms of the GNU Lesser General Public
License version 3.0 as published by the Free Software Foundation and
appearing in the file LICENSE included in the packaging of this file.
Please review the following information to ensure the GNU Lesser
General Public License version 3.0 requirements will be met:
http://www.gnu.org/copyleft/lgpl.html.

Other Usage
Alternatively, this file may be used in accordance with the terms and
conditions contained in a signed written agreement between you and
Marcus Tomlinson.

DSPatch is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
************************************************************************/

#include <WaveReader.h>

#include <cstring>
#include <fstream>
#include <iostream>

const int c_bufferSize = 440;  // Process 10ms chunks of data @ 44100Hz

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
    WaveReader( std::string const& fileName )
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

void WaveReader::Process_( SignalBus const&, SignalBus& outputs )
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
