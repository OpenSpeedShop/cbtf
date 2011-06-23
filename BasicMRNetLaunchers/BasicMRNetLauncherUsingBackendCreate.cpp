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

/** @file Component for a basic MRNet launcher using backend-create mode. */

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <mrnet/MRNet.h>
#include <typeinfo>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/**
 * Basic MRNet launcher that instantiates the internal and backend processes
 * directly using MRNet's built-in rsh/ssh based facilities for this purpose.
 */
class __attribute__ ((visibility ("hidden"))) 
BasicMRNetLauncherUsingBackendCreate :
    public Component
{

public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(reinterpret_cast<Component*>(
            new BasicMRNetLauncherUsingBackendCreate()
            ));
    }

private:

    /** Default constructor. */
    BasicMRNetLauncherUsingBackendCreate() :
        Component(Type(typeid(BasicMRNetLauncherUsingBackendCreate)), 
                  Version(0, 0, 0))
    {
        declareInput<boost::filesystem::path>(
            "TopologyFile",
            boost::bind(
                &BasicMRNetLauncherUsingBackendCreate::handleTopologyFile,
                this, _1
                )
            );

        declareOutput<boost::shared_ptr<MRN::Network> >("Network");
    }
    
    /** Handler for the "TopologyFile" input. */
    void handleTopologyFile(const boost::filesystem::path& path)
    {
        // Get the path of, and arguments for, the MRNet backend executable
        boost::filesystem::path backend_path = getMRNetBackendPath();
        std::vector<std::string> backend_arguments = getMRNetBackendArguments();

        // Translate the arguments into an argv-style argument list
        const char** argv = new const char*[backend_arguments.size() + 1];
        for (std::vector<std::string>::size_type 
                 i = 0; i < backend_arguments.size(); ++i)
        {
            argv[i] = backend_arguments[i].c_str();
        }
        argv[backend_arguments.size()] = NULL;
        
        // Create the MRNet network
        boost::shared_ptr<MRN::Network> network(MRN::Network::CreateNetworkFE(
            path.string().c_str(), backend_path.string().c_str(), argv
            ));
        if (network->has_Error())
        {
            raise<std::runtime_error>("Unable to create the MRNet network.");
        }
        
        // Destroy the argv-style argument list
        delete [] argv;
        
        // Emit the MRNet network on this component's "Network" output
        emitOutput("Network", network);
    }

}; // class BasicMRNetLauncherUsingBackendCreate

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(
    BasicMRNetLauncherUsingBackendCreate
    )
