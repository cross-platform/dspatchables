/******************************************************************************
SocketOut DSPatch Component
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

#include <SocketOut.h>

extern "C"
{
#include <mongoose.h>
}

static void fn(struct mg_connection *c, int ev, void *ev_data, void *) {
  if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    mg_ws_upgrade(c, hm, NULL);
  } else if (ev == MG_EV_WS_MSG) {
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    mg_ws_send(c, wm->data.ptr, wm->data.len, WEBSOCKET_OP_TEXT);
    mg_iobuf_delete(&c->recv, c->recv.len);
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

class SocketOut
{
public:
    SocketOut()
    {
        mg_mgr_init(&mgr);
        mg_http_listen(&mgr, "wp://localhost:8000", fn, NULL);
    }

    ~SocketOut()
    {
        mg_mgr_free(&mgr);
    }

    struct mg_mgr mgr;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

SocketOut::SocketOut()
    : p( new internal::SocketOut )
{
    SetInputCount_( 1, { "out" } );
}

void SocketOut::Process_( SignalBus const&, SignalBus& )
{
    mg_mgr_poll(&p->mgr, 0);
}
