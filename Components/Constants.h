/******************************************************************************
DSPatchables - DSPatch Component Repository
Copyright (c) 2022, Marcus Tomlinson

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

#include <cmath>

// Common
const int c_sampleRate = 44100;
const int c_bufferSize = 440;  // Process 10ms chunks of data @ 44100Hz

// Aec
const int c_initialStreamDelay = 150;  // 150ms far signal delay has proven to be good place to start

// Agc
const int c_dbfsLevel = 3;        // -3db limiter
const int c_compressorGain = 90;  // +90db max gain

// AudioDevice
const int c_bufferWaitTimeoutMs = 500;  // Wait a max of 500ms for the sound card to respond
const int c_syncWaitTimeoutS = 2;       // Wait a max of 2s for the Process_() method to respond

// Sockets
const int c_period = ceil( ( float( c_bufferSize ) / float( c_sampleRate ) ) * 1000.0f );
const int c_doublePeriod = c_period * 2;

// VoxRemover
const float c_pi = 3.1415926535897932384626433832795f;
const float c_twoPi = c_pi * 2.0f;
const float c_s2fCoeff = 1.0f / 32767.0f;
const float c_f2sCoeff = 32767.0f * 0.85f;

// WaveWriter
const int c_channelCount = 2;
const int c_bitsPerSample = 16;
