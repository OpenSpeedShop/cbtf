////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the Network class. */

#pragma once

#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <map>
#include <string>
#include <vector>
#include <xercesc/dom/DOM.hpp>

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Container for a network of connected components. The components to be
     * instantiated, the connections between them, and the inputs and outputs
     * exposed outside the network, are specified by an XML tree.
     */
    class Network :
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
         * @note    The root node of the provided XML tree must conform to
         *          the NetworkType described in the "Network.xsd" schema.
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
        virtual ~Network();
        
    private:

        /**
         * Type of associative container used to map between the names of
         * component instances and the component instances themselves.
         */
        typedef std::map<std::string, Component::Instance> ComponentMap;

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
        Network(const Type& type, const Version& version,
                const boost::shared_ptr<xercesc::DOMDocument>& document,
                const xercesc::DOMNode* root);
        
        /** Parse the specified ComponentType node. */
        void parseComponent(const xercesc::DOMNode* node);
        
        /** Parse the specified ConnectionType node. */
        void parseConnection(const xercesc::DOMNode* node);
        
        /** Parse the specified InputType node. */
        void parseInput(const xercesc::DOMNode* node);
        
        /** Parse the specified OutputType node. */
        void parseOutput(const xercesc::DOMNode* node);

        /** Component instances in this network. */
        ComponentMap dm_components;

        /** Mediators in this network. */
        std::vector<Component::Instance> dm_mediators;
        
    }; // class Network
            
} } } // namespace KrellInstitute::CBTF::Impl
