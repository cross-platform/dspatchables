/******************************************************************************
VoxRemover DSPatch Component
Copyright (c) 2021, Marcus Tomlinson

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

#pragma once

#include <DSPatch.h>

#include <mutex>

namespace DSPatch
{
namespace DSPatchables
{

class DLLEXPORT VoxRemover final : public Component
{
public:
    VoxRemover();
    virtual ~VoxRemover();

protected:
    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override;

private:
    std::vector<float> _hanningLookup = std::vector<float>( c_bufferSize * 2, 0.0f );

    std::vector<float> _inputBufL = std::vector<float>( c_bufferSize, 0.0f );
    std::vector<float> _inputBufR = std::vector<float>( c_bufferSize, 0.0f );
    std::vector<float> _prevInputBufL = std::vector<float>( c_bufferSize, 0.0f );
    std::vector<float> _prevInputBufR = std::vector<float>( c_bufferSize, 0.0f );
    std::vector<short> _outputBufL = std::vector<short>( c_bufferSize, 0 );
    std::vector<short> _outputBufR = std::vector<short>( c_bufferSize, 0 );

    std::vector<float> _sigBufL = std::vector<float>( c_bufferSize * 2, 0.0f );
    std::vector<float> _sigBufR = std::vector<float>( c_bufferSize * 2, 0.0f );
    std::vector<float> _specBufL = std::vector<float>( c_bufferSize * 2, 0.0f );
    std::vector<float> _specBufR = std::vector<float>( c_bufferSize * 2, 0.0f );

    std::vector<float> _mixBufL = std::vector<float>( c_bufferSize, 0.0f );
    std::vector<float> _mixBufR = std::vector<float>( c_bufferSize, 0.0f );

    void _BuildHanningLookup();

    void _BuildSignalBuffers();
    void _StoreCurrentBuffers();
    void _ApplyHanningWindows();
    void _FftSignalBuffers();
    void _ProcessSpectralBuffers();
    void _IfftSpectralBuffers();

    float _dphi = 0.05;
};

EXPORT_PLUGIN( VoxRemover )

}  // namespace DSPatchables
}  // namespace DSPatch
