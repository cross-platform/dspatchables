/******************************************************************************
SocketIn DSPatch Component
Copyright (c) 2024, Marcus Tomlinson

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

#pragma once

#include <DSPatch.h>

namespace DSPatch
{
namespace DSPatchables
{

namespace internal
{
class SocketIn;
}

class DLLEXPORT SocketIn final : public Component
{
public:
    SocketIn();

    void SetUrl( std::string const& newUrl );

protected:
    virtual void Process_( SignalBus& inputs, SignalBus& outputs ) override;

private:
    std::unique_ptr<internal::SocketIn> p;
};

EXPORT_PLUGIN( SocketIn )

}  // namespace DSPatchables
}  // namespace DSPatch
