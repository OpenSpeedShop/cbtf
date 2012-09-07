////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Declaration of the stream mediator connect function. */

#pragma once

#include <KrellInstitute/CBTF/Component.hpp>
#include <string>

namespace KrellInstitute { namespace CBTF { namespace Impl {
    
    /**
     * Connect a component's output to a component's input. If the types of the
     * specified output and input are of compatible types, simply connect them
     * directly. Otherwise search for, instantiate, and interpose an available
     * component that can provide the conversion.
     *
     * @param output_instance    Component with output being connected.
     * @param output_name        Name of output being connected.
     * @param input_instance     Component with input being connected.
     * @param input_name         Name of input being connected.
     * @return                   Component instance providing the automatic type
     *                           conversion, or null if no conversion was used.
     */
    Component::Instance connectWithAutomaticConversion(
        Component::Instance output_instance,
        const std::string& output_name,
        Component::Instance input_instance,
        const std::string& input_name
        );

} } } // namespace KrellInstitute::CBTF::Impl
