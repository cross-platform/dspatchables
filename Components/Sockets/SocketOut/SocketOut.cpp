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

static void fn(struct mg_connection *c, int ev, void *ev_data, void * data) {
    auto buffer = reinterpret_cast<std::vector<short>*>(data);

    if (ev == MG_EV_HTTP_MSG)
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        mg_ws_upgrade(c, hm, nullptr);
    }
    else if (ev == MG_EV_WS_MSG)
    {
        mg_ws_send(c, (const char*)&(*buffer)[0], buffer->size() * 2, WEBSOCKET_OP_BINARY);
        *buffer = std::vector<short>();
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
        mg_http_listen(&mgr, "localhost:8000", fn, &buffer);
    }

    ~SocketOut()
    {
        mg_mgr_free(&mgr);
    }

    struct mg_mgr mgr;
    std::vector<short> buffer;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

SocketOut::SocketOut()
    : p( new internal::SocketOut )
{
    SetInputCount_( 1, { "out" } );
}

void SocketOut::Process_( SignalBus const& inputs, SignalBus& )
{
    auto in = inputs.GetValue<std::vector<short>>( 0 );
    if ( in )
    {
        p->buffer = *in;
    }

    while (!p->buffer.empty())
    {
        mg_mgr_poll(&p->mgr, 0);
    }
}
