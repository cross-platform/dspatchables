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
#include <iostream>

static void hfn(mg_connection *c, int ev, void *ev_data, void * data) {
    auto buffer = (std::vector<short>*) data;
    auto hm = (mg_http_message *) ev_data;

    if (ev == MG_EV_HTTP_MSG)
    {
        mg_http_serve_opts opts;
        opts.root_dir = HTML_ROOT;
        mg_http_serve_dir(c, hm, &opts);
    }
    else if (ev == MG_EV_WS_MSG)
    {
        mg_ws_send(c, (const char*)&(*buffer)[0], buffer->size() * 2, WEBSOCKET_OP_BINARY);
        *buffer = std::vector<short>();
    }
}

static void fn(mg_connection *c, int ev, void *ev_data, void * data) {
    auto buffer = (std::vector<short>*) data;
    auto hm = (mg_http_message *) ev_data;

    if (ev == MG_EV_HTTP_MSG)
    {
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
        hc = mg_http_listen(&mgr, "localhost:8080", hfn, &buffer);
    }

    ~SocketOut()
    {
        mg_mgr_free(&mgr);
    }

    mg_mgr mgr;
    mg_connection *c, *hc;
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
        p->c = mg_http_listen(&p->mgr, (p->ip + ":8000").c_str(), fn, &p->buffer);
        p->hc = mg_http_listen(&p->mgr, (p->ip + ":8080").c_str(), hfn, &p->buffer);
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
