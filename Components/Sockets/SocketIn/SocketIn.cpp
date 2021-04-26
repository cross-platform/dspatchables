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

#include <Constants.h>
#include <SocketIn.h>

extern "C"
{
#include <mongoose.h>
}

static void fn( mg_connection* c, int ev, void* ev_data, void* data )
{
    auto buffer = (std::vector<short>*)data;
    auto wm = (mg_ws_message*)ev_data;

    if ( ev == MG_EV_ERROR )
    {
        LOG( LL_ERROR, ( "%p %s", c->fd, (char*)ev_data ) );
    }
    else if ( ev == MG_EV_WS_OPEN )
    {
        *buffer = { 0 };
    }
    else if ( ev == MG_EV_CLOSE )
    {
        *buffer = {};
    }
    else if ( ev == MG_EV_WS_MSG )
    {
        if ( wm->data.len != 0 )
        {
            auto short_data = (const short*)wm->data.ptr;

            *buffer = {};
            for ( size_t i = 0; i < wm->data.len / 2; ++i )
            {
                buffer->push_back( short_data[i] );
            }
        }
        else
        {
            *buffer = { 0 };
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
        mg_mgr_init( &mgr );
        c = mg_ws_connect( &mgr, "localhost:8000", fn, &buffer, nullptr );
    }

    ~SocketIn()
    {
        mg_mgr_free( &mgr );
    }

    mg_mgr mgr;
    mg_connection* c;
    std::vector<short> buffer;
    std::string url;
};

}  // namespace internal
}  // namespace DSPatchables
}  // namespace DSPatch

SocketIn::SocketIn()
    : p( new internal::SocketIn )
{
    SetInputCount_( 1, { "url" } );
    SetOutputCount_( 1, { "in" } );
}

void SocketIn::SetUrl( std::string const& newUrl )
{
    if ( newUrl != p->url )
    {
        p->url = newUrl;
        p->buffer = {};
        mg_mgr_free( &p->mgr );
        mg_mgr_init( &p->mgr );
        p->c = mg_ws_connect( &p->mgr, p->url.c_str(), fn, &p->buffer, nullptr );
    }
}

void SocketIn::Process_( SignalBus const& inputs, SignalBus& outputs )
{
    auto url = inputs.GetValue<std::string>( 0 );
    if ( url )
    {
        SetUrl( *url );
    }

    if ( !p->buffer.empty() )
    {
        mg_ws_send( p->c, nullptr, 0, WEBSOCKET_OP_BINARY );
    }

    mg_mgr_poll( &p->mgr, c_doublePeriod );

    if ( p->buffer.size() > 1 )
    {
        outputs.SetValue( 0, p->buffer );
    }
}
