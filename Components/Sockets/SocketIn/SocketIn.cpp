/******************************************************************************
DSPatchables - DSPatch Component Repository
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

#include <SocketIn.h>
#include <thread>

extern "C"
{
#include <mongoose.h>
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *) {
  if (ev == MG_EV_ERROR) {
    LOG(LL_ERROR, ("%p %s", c->fd, (char *) ev_data));
  } else if (ev == MG_EV_WS_OPEN) {
    mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
  } else if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    printf("GOT ECHO REPLY: [%.*s]\n", (int) wm->data.len, wm->data.ptr);
    mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
  }

  if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE || ev == MG_EV_WS_MSG) {
      printf("DONE\n");
  }
}

using namespace DSPatch;
using namespace DSPatchables;

namespace DSPatch
{
namespace DSPatchables
{
namespace internal
{

class SocketIn
{
public:
    SocketIn( float initGain )
    {
        gain = initGain;

        mg_mgr_init(&mgr);
        c = mg_ws_connect(&mgr, "wp://localhost:8000/websocket", fn, NULL, NULL);
    }

    ~SocketIn()
    {
        mg_mgr_free(&mgr);
    }

    float gain;
    struct mg_mgr mgr;
    struct mg_connection *c;
    std::mutex m;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

SocketIn::SocketIn( float initGain )
    : Component( ProcessOrder::OutOfOrder )
    , p( new internal::SocketIn( initGain ) )
{
    SetInputCount_( 2, { "in", "gain" } );
    SetOutputCount_( 1, { "out" } );
}

void SocketIn::SetGain( float gain )
{
    p->gain = gain;
}

float SocketIn::GetGain() const
{
    return p->gain;
}

void SocketIn::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    p->m.lock();
    mg_mgr_poll(&p->mgr, 0);
    p->m.unlock();

    auto in = inputs.GetValue<std::vector<short>>( 0 );
    if ( !in )
    {
        return;
    }

    auto gain = inputs.GetValue<float>( 1 );
    if ( gain )
    {
        p->gain = *gain;
    }

    for ( auto& inSample : *in )
    {
        inSample *= p->gain;  // apply gain sample-by-sample
    }

    outputs.MoveSignal( 0, inputs.GetSignal( 0 ) );  // move gained input signal to output
}
