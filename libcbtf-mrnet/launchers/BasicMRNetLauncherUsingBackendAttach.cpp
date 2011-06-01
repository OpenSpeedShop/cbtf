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

/** @file Component for a basic MRNet launcher using backend-attach mode. */

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <mrnet/MRNet.h>
#include <typeinfo>
#include <unistd.h>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Handler for MRNet backend-added topology events. */
    void handleBackendAdded(MRN::Event* event, void* data)
    {
        int* count = reinterpret_cast<int*>(data);
        BOOST_ASSERT(count != NULL);
        ++(*count);
    }
    
} // namespace <anonymous>



/**
 * Basic MRNet launcher that instantiates the internal processes directly using
 * MRNet's built-in rsh/ssh based facilities for this purpose, but expects that
 * the backends will be independently started and will attach themselves to the
 * MRNet network.
 */
class __attribute__ ((visibility ("hidden"))) 
BasicMRNetLauncherUsingBackendAttach :
    public Component
{
    
public:

    /** Factory function for this component type. */
    static Component::Instance factoryFunction()
    {
        return Component::Instance(reinterpret_cast<Component*>(
            new BasicMRNetLauncherUsingBackendAttach()
            ));
    }

private:

    /** Default constructor. */
    BasicMRNetLauncherUsingBackendAttach() :
        Component(Type(typeid(BasicMRNetLauncherUsingBackendAttach)), 
                  Version(0, 0, 0)),
        dm_backend_attach_count(1),
        dm_backend_attach_file("~/.cbtf/attachBE_connections")
    {
        declareInput<unsigned int>(
            "BackendAttachCount",
            boost::bind(
                &BasicMRNetLauncherUsingBackendAttach::handleBackendAttachCount,
                this, _1
                )
            );
        declareInput<boost::filesystem::path>(
            "BackendAttachFile",
            boost::bind(
                &BasicMRNetLauncherUsingBackendAttach::handleBackendAttachFile,
                this, _1
                )
            );
        
        declareInput<boost::filesystem::path>(
            "TopologyFile",
            boost::bind(
                &BasicMRNetLauncherUsingBackendAttach::handleTopologyFile,
                this, _1
                )
            );
        
        declareOutput<boost::shared_ptr<MRN::Network> >("Network");
    }
    
    /** Handler for the "BackendAttachCount" input. */
    void handleBackendAttachCount(const unsigned int& count)
    {
        dm_backend_attach_count = count;
    }
    
    /** Handler for the "BackendAttachFile" input. */
    void handleBackendAttachFile(const boost::filesystem::path& path)
    {
	std::string mypath(path.string());

	if (not mypath.empty() and mypath[0] == '~') {
	    assert(mypath.size() == 1 or mypath[1] == '/');
	    char const* home = getenv("HOME");
	    if (home) {
		mypath.replace(0, 1, home);
	    }

            boost::filesystem::path newpath(mypath);
            dm_backend_attach_file = newpath;

	} else {

            dm_backend_attach_file = path;

	}
    }
    
    /** Handler for the "TopologyFile" input. */
    void handleTopologyFile(const boost::filesystem::path& path)
    {
        int added_backends = 0;
        
        // Create the MRNet network
        boost::shared_ptr<MRN::Network> network(MRN::Network::CreateNetworkFE(
            path.string().c_str(), NULL, NULL
            ));
        if (network->has_Error())
        {
            raise<std::runtime_error>("Unable to create the MRNet network.");
        }
        
        // Register a handler for backend-added events
        network->register_EventCallback(
            MRN::Event::TOPOLOGY_EVENT, MRN::TopologyEvent::TOPOL_ADD_BE,
            handleBackendAdded, &added_backends
            );
        
        // Access the topology of this network
        std::vector<MRN::NetworkTopology::Node*> leaves;
        network->get_NetworkTopology()->get_Leaves(leaves);

	std::string mypath(dm_backend_attach_file.string());

	if (not mypath.empty() and mypath[0] == '~') {
	    assert(mypath.size() == 1 or mypath[1] == '/');
	    char const* home = getenv("HOME");
	    if (home) {
		mypath.replace(0, 1, home);
	    }

            boost::filesystem::path newpath(mypath);
            dm_backend_attach_file = newpath;

	}

        // Write the backend-attach connection file
        boost::filesystem::ofstream stream(dm_backend_attach_file);

        for (int i = 0; i < dm_backend_attach_count; ++i)
        {
            const int l = i * leaves.size() / dm_backend_attach_count;
            stream << leaves[l]->get_HostName() << " "
                   << leaves[l]->get_Port() << " "
                   << leaves[l]->get_Rank() << " "
                   << i << std::endl;
        }
        stream.close();
        
        // Wait for the expected number of backends
        while (added_backends < dm_backend_attach_count)
        {
            sleep(1);
        }

        // Emit the MRNet network on this component's "Network" output
        emitOutput("Network", network);
    }
    
    /** Number of backends connecting via MRNet's backend-attach mode. */
    unsigned int dm_backend_attach_count;
    
    /** Path of the connection file for MRNet's backend-attach mode. */
    boost::filesystem::path dm_backend_attach_file;
    
}; // class BasicMRNetLauncherUsingBackendAttach

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(
    BasicMRNetLauncherUsingBackendAttach
    )
