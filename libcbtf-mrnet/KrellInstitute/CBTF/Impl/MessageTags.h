////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010,2013 Krell Institute. All Rights Reserved.
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

/** @file Declaration of MRNet message tags. */

#pragma once

#if !defined(FirstApplicationTag)
#include <mrnet_lightweight/Types.h>
#endif



/** 
 * Sent by the frontend in order to establish the stream used by the backends
 * to send data to the frontend.
 */
#define KRELL_INSTITUTE_CBTF_IMPL_ESTABLISH_UPSTREAM \
    (FirstApplicationTag + 0)

/** Sent by the frontend to request shutdown of the backends. */
#define KRELL_INSTITUTE_CBTF_IMPL_REQUEST_SHUTDOWN \
    (FirstApplicationTag + 1)

/** Sent by a backend to acknowledge it is about to shutdown. */
#define KRELL_INSTITUTE_CBTF_IMPL_ACKNOWLEDGE_SHUTDOWN \
    (FirstApplicationTag + 2)

/**
 * Sent by the frontend in order to specify the named streams used by a given
 * distributed component network. These named streams are used for communication
 * between the local component networks on the backends, filters, and frontend.
 */
#define KRELL_INSTITUTE_CBTF_IMPL_SPECIFY_NAMED_STREAMS \
    (FirstApplicationTag + 3)

/**
 * Sent by the frontend in order to specify the local component network used by
 * the backends for a given distributed component network.
 */
#define KRELL_INSTITUTE_CBTF_IMPL_SPECIFY_BACKEND \
    (FirstApplicationTag + 4)
        
/**
 * Sent by the frontend in order to specify the local component network used by
 * filters at one or more levels within the MRNet tree for a given distributed
 * component network.
 */
#define KRELL_INSTITUTE_CBTF_IMPL_SPECIFY_FILTER \
    (FirstApplicationTag + 5)

/**
 * Sent by the frontend in order to request the destruction of all the local
 * component networks for a given distributed component network.
 */
#define KRELL_INSTITUTE_CBTF_IMPL_DESTROY_NETWORK \
    (FirstApplicationTag + 6)

/**
 * First message tag assigned to a named stream used for communication between
 * the local component networks on the backends, filters, and frontend.
 */
#define KRELL_INSTITUTE_CBTF_IMPL_FIRST_NAMED_STREAM_TAG \
    (FirstApplicationTag + 99)
