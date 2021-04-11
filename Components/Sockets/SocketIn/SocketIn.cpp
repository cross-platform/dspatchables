/******************************************************************************
SocketIn DSPatch Component
Copyright (c) 2021, Marcus Tomlinson

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
