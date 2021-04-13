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

#include <mongoose.h>

#include <thread>

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
        c = mg_http_listen(&mgr, "localhost:8000", fn, &buffer);
    }

    ~SocketOut()
    {
        mg_mgr_free(&mgr);
    }

    struct mg_mgr mgr;
    struct mg_connection *c;
    std::vector<short> buffer;
    std::string ip;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

SocketOut::SocketOut()
    : p( new internal::SocketOut )
{
    SetInputCount_( 2, { "out", "ip" } );
}

void SocketOut::SetIp(std::string const& newIp)
{
    if (newIp != p->ip)
    {
        p->ip = newIp;
        p->buffer = std::vector<short>();
        mg_mgr_free(&p->mgr);
        mg_mgr_init(&p->mgr);
        p->c = mg_http_listen(&p->mgr, p->ip.c_str(), fn, &p->buffer);
    }
}

void SocketOut::Process_( SignalBus const& inputs, SignalBus& )
{
    auto in = inputs.GetValue<std::vector<short>>( 0 );
    if ( in )
    {
        p->buffer = *in;
    }

    auto ip = inputs.GetValue<std::string>( 1 );
    if ( ip )
    {
        SetIp(*ip);
    }

    for (int i = 0; i < 50; ++i)
    {
        mg_mgr_poll(&p->mgr, 0);
        if (p->buffer.empty())
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}
