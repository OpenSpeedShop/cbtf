////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2.1 of the License, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
////////////////////////////////////////////////////////////////////////////////

/** @file Definition of the stream mediator connect function. */

#include <KrellInstitute/CBTF/Type.hpp>
#include <map>
#include <stdexcept>

#include "Raise.hpp"
#include "StreamMediator.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Component::Instance KrellInstitute::CBTF::Impl::connectWithAutomaticConversion(
    Component::Instance output_instance,
    const std::string& output_name,
    Component::Instance input_instance,
    const std::string& input_name
    )
{
    //
    // Determine the type of the specified output and input.
    //

    std::map<std::string, Type> outputs = output_instance->getOutputs();

    if (outputs.find(output_name) == outputs.end())
    {
        raise<std::runtime_error>(
            "The requested output (%1%) doesn't exist.", output_name
            );        
    }

    Type output_type = outputs.find(output_name)->second;

    std::map<std::string, Type> inputs = input_instance->getInputs();

    if (inputs.find(input_name) == inputs.end())
    {
        raise<std::runtime_error>(
            "The requested input (%1%) doesn't exist.", input_name
            );        
    }

    Type input_type = inputs.find(input_name)->second;

    //
    // There is no need for an automatic type converter when the output and
    // input are of compatible types. In this case simply defer to the usual
    // component connect method.
    //

    if (output_type == input_type)
    {
        Component::connect(
            output_instance, output_name, input_instance, input_name
            );

        return Component::Instance();
    }

    //
    // Otherwise search for a component to perform the type conversion. A
    // component type is deemed suitable for this purpose if its type name
    // contains the string "Convert", it has a single input of the same type
    // as the specified output, and it has a single output of the same type
    // as the specifed input. The first such component type is instantiated
    // and interposed between the two given components.
    //

    std::set<Type> available_types = Component::getAvailableTypes();

    for (std::set<Type>::const_iterator
             i = available_types.begin(); i != available_types.end(); ++i)
    {
        if (std::string(*i).find("Convert") == std::string::npos)
        {
            continue;
        }

        Component::Instance converter = Component::instantiate(*i);
        
        if ((converter->getInputs().size() != 1) ||
            (converter->getInputs().begin()->second != output_type) ||
            (converter->getOutputs().size() != 1) ||
            (converter->getOutputs().begin()->second != input_type))
        {
            continue;
        }
        
        Component::connect(
            output_instance, output_name,
            converter, converter->getInputs().begin()->first
            );

        Component::connect(
            converter, converter->getOutputs().begin()->first,
            input_instance, input_name
            );

        return converter;
    }

    //
    // Otherwise the specified output and input are of incompatible types and
    // a suitable automatic type converter could not be found. Throw the usual
    // exception to let the caller know.
    //

    raise<std::runtime_error>(
        "The requested output (%1%) and input (%2%) are not of compatible "
        "types and no suitable automatic type converter could be found.",
        output_name, input_name
        );
}
