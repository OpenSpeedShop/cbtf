////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the LocalComponentNetwork class. */

#pragma once

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <vector>
#include <xercesc/dom/DOM.hpp>

#include "IncomingStreamMediator.hpp"
#include "MessageHandler.hpp"
#include "NamedStreams.hpp"
#include "OutgoingStreamMediator.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {

    /**
     * Container for a local component network and its associated mediators,
     * named streams, etc. One such local component network is found on each
     * of the backends, filters, and frontend.
     */
    class LocalComponentNetwork :
        private boost::noncopyable
    {

    public:

        /** Type of function for binding an incoming stream mediator. */
        typedef boost::function<
            void (boost::shared_ptr<IncomingStreamMediator>&)
            > IncomingBinder;

        /**
         * Construct a new container for a local component network. The actual
         * local component network and its associated mediators, named streams,
         * etc. aren't created until the initializeStepN() methods are invoked.
         */
        LocalComponentNetwork();

        /**
         * Perform step one of this local component network's initialization.
         * Saves a reference to the named streams for the distributed component
         * network containing this local component network.
         *
         * @param named_streams    Named streams for this network.
         *
         * @throw std::logic_error    The method initializeStepOne()
         *                            can't be invoked more than once.
         */
        void initializeStepOne(
            const boost::shared_ptr<NamedStreams>& named_streams
            );
        
        /**
         * Perform step two of this local component network's initialization.
         * Instantiates the actual local component network.
         *
         * @param document    Document containing the XML tree describing
         *                    the actual component network to be instantiated.
         * @param root        Root node of the XML tree describing the
         *                    actual component network to be instantiated.
         *
         * @throw std::logic_error    The method initializeStepTwo()
         *                            can't be invoked more than once.
         */
        void initializeStepTwo(
            const boost::shared_ptr<xercesc::DOMDocument>& document,
            const xercesc::DOMNode* root
            );
        
        /**
         * Perform step three of this local component network's initialization.
         * Creates this local component network's incoming and outgoing stream
         * mediators.
         *
         * @param incoming_upstream_binder       Incoming upstream binder
         *                                       for this network.
         * @param incoming_downstream_binder     Incoming downstream binder
         *                                       for this network.
         * @param outgoing_upstream_handler      Outgoing upstream handler
         *                                       for this network.
         * @param outgoing_downstream_handler    Outgoing downstream handler
         *                                       for this network.
         *
         * @throw std::logic_error    The method initializeStepThree()
         *                            can't be invoked more than once,
         *                            and can't be invoked before both
         *                            the initializeStepOne() and the
         *                            initializeStepTwo() methods have
         *                            been invoked.
         */
        void initializeStepThree(
            const IncomingBinder& incoming_upstream_binder,
            const IncomingBinder& incoming_downstream_binder,
            const MessageHandler& outgoing_upstream_handler,
            const MessageHandler& outgoing_downstream_handler
            );

        /**
         * Get the named streams for this network.
         *
         * @return    Named streams for this network.
         */
        const boost::shared_ptr<NamedStreams>& named_streams() const;
        
        /**
         * Get the actual local component network.
         *
         * @return    Actual local component network.
         */
        Component::Instance network();

    private:

        /** Parse the specified (MRNet) InputType node. */
        void parseIncomingStream(const xercesc::DOMNode* node,
                                 const IncomingBinder& binder);
        
        /** Parse the specified NetworkType node. */
        void parseNetwork(const xercesc::DOMNode* node);

        /** Parse the specified (MRNet) OutputType node. */
        void parseOutgoingStream(const xercesc::DOMNode* node,
                                 const MessageHandler& handler);
        
        /** Named streams for this network. */
        boost::shared_ptr<NamedStreams> dm_named_streams;

        /** XML document describing this network. */
        boost::shared_ptr<xercesc::DOMDocument> dm_document;

        /** XML tree describing this network. */
        const xercesc::DOMNode* dm_root;

        /** Actual local component network. */
        Component::Instance dm_network;
        
        /** Mediators for this network. */
        std::vector<Component::Instance> dm_mediators;

    }; // class LocalComponentNetwork

} } } // namespace KrellInstitute::CBTF::Impl
