////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010,2011 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the MRNet class. */

#pragma once

#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <mrnet/MRNet.h>
#include <vector>
#include <xercesc/dom/DOM.hpp>

#include "Frontend.hpp"
#include "IncomingStreamMediator.hpp"
#include "LocalComponentNetwork.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Container for a distributed (via MRNet) network of connected components.
     * The components to be instantiated, the connections between them, and the
     * inputs and outputs exposed outside the network, are specified by an XML
     * tree.
     */
    class MRNet :
        public Component
    {

    public:

        /**
         * Register a XML tree describing a network of connected components.
         *
         * @param document    Document containing the XML tree describing
         *                    the component network to be registered.
         * @param root        Root node of the XML tree describing the
         *                    component network to be registered.
         *
         * @note    The root node of the provided XML tree must conform
         *          to the MRNetType described in the "MRNet.xsd" schema.
         */
        static void registerXML(
            const boost::shared_ptr<xercesc::DOMDocument>& document,
            const xercesc::DOMNode* root
            );

        /**
         * Factory function for a component network.
         *
         * @param document    Document containing the XML tree describing
         *                    the component network to be instantiated.
         * @param root        Root node of the XML tree describing the
         *                    component network to be instantiated.
         * @return            A new instance of that component network.
         */
        static Component::Instance factoryFunction(
            const boost::shared_ptr<xercesc::DOMDocument>& document,
            const xercesc::DOMNode* root
            );

        /**
         * Destroy a component network. Releases any resources used by the
         * component network.
         */
        virtual ~MRNet();

    private:

        /**
         * Construct a new component network from the specified XML tree.
         *
         * @param type        Type of this component network.
         * @param version     Version of this component network.
         * @param document    Document containing the XML tree describing
         *                    the component network to be constructed.
         * @param root        Root node of the XML tree describing the
         *                    component network to be constructed.
         */
        MRNet(const Type& type, const Version& version,
              const boost::shared_ptr<xercesc::DOMDocument>& document,
              const xercesc::DOMNode* root);

        /** Bind the specified incoming upstream mediator. */
        void bindIncomingUpstream(
            boost::shared_ptr<IncomingStreamMediator>& mediator
            );
        
        /** Handler for the "Network" input. */
        void handleNetwork(const boost::shared_ptr<MRN::Network>& network);
        
        /** Parse the specified BackendType node. */
        void parseBackend(const xercesc::DOMNode* node);

        /** Parse the specified FilterType node. */
        void parseFilter(const xercesc::DOMNode* node, bool sendAllOther);

        /** Parse the specified FrontendType node. */
        void parseFrontend(const xercesc::DOMNode* node);

        /** Parse the specified (Network) InputType node. */
        void parseInput(const xercesc::DOMNode* node);

        /** Parse the specified (Network) OutputType node. */
        void parseOutput(const xercesc::DOMNode* node);
        
        /** XML document describing this network. */
        const boost::shared_ptr<xercesc::DOMDocument> dm_document;

        /** XML tree describing this network. */
        const xercesc::DOMNode* dm_root;

        /** Local component network. */
        LocalComponentNetwork dm_local_component_network;
        
        /** MRNet frontend of this network. */
        boost::shared_ptr<Frontend> dm_frontend;

        /** Mediators in this network. */
        std::vector<Component::Instance> dm_mediators;

    }; // class MRNet
            
} } } // namespace KrellInstitute::CBTF::Impl
