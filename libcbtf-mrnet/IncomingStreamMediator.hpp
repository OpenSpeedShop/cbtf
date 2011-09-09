////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010 Krell Institute. All Rights Reserved.
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

/** @file Declaration of the IncomingStreamMediator class. */

#pragma once

#include <KrellInstitute/CBTF/Component.hpp>
#include <mrnet/MRNet.h>
#include <xercesc/dom/DOM.hpp>

#include "NamedStreams.hpp"

namespace KrellInstitute { namespace CBTF { namespace Impl {
    
    /**
     * Mediator for an incoming stream. Provides the ability to forward
     * messages with a specific tag, from a MRNet network, to component
     * inputs. This class is itself a component, and thus its output can
     * be attached to inputs of other components via the usual mechanism
     * for connecting components.
     *
     * @note    Unlike most component types, this type has no factory
     *          function and thus is not registered with the Component
     *          class and must be instantiated via the create() method.
     *
     * @sa http://en.wikipedia.org/wiki/Mediator_pattern
     */
    class IncomingStreamMediator :
        public Component
    {

    public:

        /**
         * Create a new mediator for an incoming stream.
         *
         * @param node             Node of the XML tree describing the
         *                         incoming stream to be mediated.
         * @param network          Component network using that incoming stream.
         * @param named_streams    All of the possible named streams.
         */
        static Component::Instance create(const xercesc::DOMNode* node,
                                          Component::Instance network,
                                          const NamedStreams& named_streams);
        
        /**
         * Handler for new messages to be forwarded.
         *
         * @param packet    Packet containing a new mesage to be forwarded.
         */
        void handler(const MRN::PacketPtr& packet);

        /**
         * Get the MRNet message tag for the named stream being mediated.
         *
         * @return    MRNet message tag for the named stream being mediated.
         */
        int tag() const;

    private:

        /**
         * Create a new mediator for an incoming stream.
         *
         * @param tag    MRNet message tag for the named stream being mediated.
         */
        IncomingStreamMediator(const int& tag);

        /** Automatic type converter (if any) for this mediator. */
        Component::Instance& converter()
        {
            return dm_converter;
        }
        
        /** MRNet message tag for the named stream being mediated. */
        const int dm_tag;

        /** Automatic type converter (if any) for this mediator. */
        Component::Instance dm_converter;
                        
    }; // class IncomingStreamMediator

} } } // namespace KrellInstitute::CBTF::Impl
