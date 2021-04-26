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

#include <Constants.h>
#include <SocketOut.h>

extern "C"
{
#include <mongoose.h>
}

#include <thread>

static void fn( mg_connection* c, int ev, void* ev_data, void* data )
{
    auto buffer = (std::vector<short>*)data;
    auto hm = (mg_http_message*)ev_data;

    if ( ev == MG_EV_ERROR )
    {
        LOG( LL_ERROR, ( "%p %s", c->fd, (char*)ev_data ) );
    }
    else if ( ev == MG_EV_HTTP_MSG && std::string( hm->message.ptr ).find( "websocket" ) != std::string::npos )
    {
        mg_ws_upgrade( c, hm );
    }
    else if ( ev == MG_EV_HTTP_MSG )
    {
        mg_http_serve_dir( c, hm, HTML_ROOT );
    }
    else if ( ev == MG_EV_WS_MSG )
    {
        mg_ws_send( c, (const char*)&( *buffer )[0], buffer->size() * 2, WEBSOCKET_OP_BINARY );
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
        mg_mgr_init( &mgr );
        c = mg_http_listen( &mgr, "localhost:8000", fn, &buffer );
    }

    ~SocketOut()
    {
        mg_mgr_free( &mgr );
    }

    mg_mgr mgr;
    mg_connection *c, *hc;
    std::vector<short> buffer;
    std::string port;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

SocketOut::SocketOut()
    : p( new internal::SocketOut )
{
    SetInputCount_( 2, { "out", "port" } );
}

void SocketOut::SetPort( std::string const& newPort )
{
    if ( newPort != p->port )
    {
        p->port = newPort;
        p->buffer = std::vector<short>();
        mg_mgr_free( &p->mgr );
        mg_mgr_init( &p->mgr );
        p->c = mg_http_listen( &p->mgr, ( "localhost:" + p->port ).c_str(), fn, &p->buffer );
    }
}

void SocketOut::Process_( SignalBus const& inputs, SignalBus& )
{
    auto in = inputs.GetValue<std::vector<short>>( 0 );
    if ( in )
    {
        p->buffer = *in;
    }

    auto port = inputs.GetValue<std::string>( 1 );
    if ( port )
    {
        SetPort( *port );
    }

    auto begin = std::chrono::steady_clock::now();
    decltype( begin ) end;
    do
    {
        mg_mgr_poll( &p->mgr, c_doublePeriod );
        if ( p->buffer.empty() )
        {
            break;
        }
        end = std::chrono::steady_clock::now();
    } while ( std::chrono::duration_cast<std::chrono::milliseconds>( end - begin ).count() < c_doublePeriod );
}
