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

#include <mongoose.h>

static void fn(struct mg_connection *c, int ev, void *ev_data, void * data) {
    auto buffer = reinterpret_cast<std::vector<short>*>(data);

    if (ev == MG_EV_ERROR)
    {
        LOG(LL_ERROR, ("%p %s", c->fd, (char *) ev_data));
    }
    else if (ev == MG_EV_WS_OPEN)
    {
        buffer->push_back(0);
    }
    else if (ev == MG_EV_CLOSE)
    {
        *buffer = std::vector<short>();
    }
    else if (ev == MG_EV_WS_MSG)
    {
        struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;

        if (wm->data.len != 0)
        {
            auto short_data = reinterpret_cast<const short*>(wm->data.ptr);

            *buffer = std::vector<short>();
            for ( size_t i = 0; i < wm->data.len / 2; ++i )
            {
                buffer->push_back(short_data[i]);
            }
        }
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
    SocketIn()
    {
        mg_mgr_init(&mgr);
        c = mg_ws_connect(&mgr, "localhost:8000", fn, &buffer, nullptr);
    }

    ~SocketIn()
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

SocketIn::SocketIn()
    : p( new internal::SocketIn )
{
    SetInputCount_( 1, { "ip" } );
    SetOutputCount_( 1, { "in" } );
}

void SocketIn::SetIp(std::string const& newIp)
{
    if (newIp != p->ip)
    {
        p->ip = newIp;
        p->buffer = std::vector<short>();
        mg_mgr_free(&p->mgr);
        mg_mgr_init(&p->mgr);
        p->c = mg_ws_connect(&p->mgr, p->ip.c_str(), fn, &p->buffer, nullptr);
    }
}

void SocketIn::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto ip = inputs.GetValue<std::string>( 0 );
    if ( ip )
    {
        SetIp(*ip);
    }

    if (!p->buffer.empty())
    {
        mg_ws_send(p->c, nullptr, 0, WEBSOCKET_OP_BINARY);
    }

    mg_mgr_poll(&p->mgr, 50);

    if (p->buffer.size() > 1)
    {
        outputs.SetValue(0, p->buffer);
    }
}
