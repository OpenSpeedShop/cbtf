////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011-2013 Krell Institute. All Rights Reserved.
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

/** @file Component for a basic MRNet launcher using backend-attach mode. */

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <mrnet/MRNet.h>
#include <stdexcept>
#include <typeinfo>
#include <unistd.h>

#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;


/** Forward definition of the MRNet event handler callback */
void handleBackendAdded(MRN::Event* event, void* data);


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

friend void handleBackendAdded(MRN::Event* event, void* data);

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
	    char const* connections_dir = getenv("PWD");
	    if (connections_dir) {
		mypath.replace(0, 1, connections_dir);
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
	// initialize backend added count whenever this handler is called.
	dm_backend_added_count = 0;
        
        // Create the MRNet network
        boost::shared_ptr<MRN::Network> network(MRN::Network::CreateNetworkFE(
            path.string().c_str(), NULL, NULL
            ));
        if (network->has_Error())
        {
            throw std::runtime_error("Unable to create the MRNet network.");
        }
        
        // Register a handler for backend-added events
        network->register_EventCallback(
            MRN::Event::TOPOLOGY_EVENT, MRN::TopologyEvent::TOPOL_ADD_BE,
            handleBackendAdded, this
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
        
	// Wait for the expected number of backends.
	boost::unique_lock<boost::mutex> backend_added_lock(dm_backend_added_count_mutex);
	while (dm_backend_added_count < dm_backend_attach_count)
	{
	    dm_backend_added_count_condition.wait(backend_added_lock);
	}

        // Emit the MRNet network on this component's "Network" output
        emitOutput("Network", network);
    }
    
    /** Number of backends connecting via MRNet's backend-attach mode. */
    unsigned int dm_backend_attach_count;
    /** Number of backends added by handleBackendAdded MRNet event handler callback. */
    unsigned int dm_backend_added_count;
    
    /** Path of the connection file for MRNet's backend-attach mode. */
    boost::filesystem::path dm_backend_attach_file;

    /** mutex and condition for backend added count */
    boost::mutex dm_backend_added_count_mutex;
    boost::condition_variable dm_backend_added_count_condition;


    
}; // class BasicMRNetLauncherUsingBackendAttach

/** Handler for MRNet backend-added topology events. */
void handleBackendAdded(MRN::Event* event, void* data)
{		
    BasicMRNetLauncherUsingBackendAttach* launcher =
		reinterpret_cast<BasicMRNetLauncherUsingBackendAttach*>(data);
    unsigned int* count = &launcher->dm_backend_added_count;
    BOOST_ASSERT(count != NULL);
    ++(*count);
    launcher->dm_backend_added_count_condition.notify_one();
}

KRELL_INSTITUTE_CBTF_REGISTER_FACTORY_FUNCTION(
    BasicMRNetLauncherUsingBackendAttach
    )
