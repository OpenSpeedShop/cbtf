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

/** @file Definition of the NamedStreams class. */

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <cstddef>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <KrellInstitute/CBTF/Impl/XercesExts.hpp>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <utility>

#include "MessageTags.hpp"
#include "NamedStreams.hpp"

using namespace KrellInstitute::CBTF;
using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Release (free) the memory for the given names and tags arrays. */
    void release(char** names, int* tags, std::size_t length)
    {
        if (names != NULL)
        {
            for (std::size_t n = 0; n < length; ++n)
            {
                if (names[n] != NULL)
                {
                    free(names[n]);
                }
            }
            free(names);            
        }
        
        if (tags != NULL)
        {
            free(tags);
        }
    }
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
NamedStreams::NamedStreams(const xercesc::DOMNode* root) :
    dm_uid(dm_uid_generator),
    dm_tags()
{
    xercesc::selectNodes(
        root, "./Stream",
        boost::bind(&NamedStreams::parseStreamDeclaration, this, _1)
        );
    xercesc::selectNodes(root, "./*/IncomingDownstream",
                         boost::bind(&NamedStreams::parseStream, this, _1));
    xercesc::selectNodes(root, "./*/OutgoingDownstream",
                         boost::bind(&NamedStreams::parseStream, this, _1));
    xercesc::selectNodes(root, "./*/IncomingUpstream",
                         boost::bind(&NamedStreams::parseStream, this, _1));
    xercesc::selectNodes(root, "./*/OutgoingUpstream",
                         boost::bind(&NamedStreams::parseStream, this, _1));
}



//------------------------------------------------------------------------------
// This code would be a LOT simpler if MRNet just used STL vectors and strings
// instead of raw C++ arrays.
//------------------------------------------------------------------------------
NamedStreams::NamedStreams(const MRN::PacketPtr& packet) :
    dm_uid(),
    dm_tags()
{
    char** names = NULL;
    int* tags = NULL;
    int names_length, tags_length = 0;
    
    try
    {
        packet->unpack(
            "%d %as %ad",
            &dm_uid, &names, &names_length, &tags, &tags_length
            );
        
        if ((names != NULL) && (names_length > 0) &&
            (tags != NULL) && (tags_length > 0) &&
            (names_length == tags_length))
        {
            for (int n = 0; n < names_length; ++n)
            {
                dm_tags.insert(
                    boost::bimap<std::string, int>::value_type(
                        names[n], tags[n]
                        )
                    );
            }
        }
    }
    catch (...)
    {
        release(names, tags, names_length);
        throw;
    }

    release(names, tags, names_length);
}



//------------------------------------------------------------------------------
// The packet's lifetime could extend beyond that of the NamedStreams. MRNet
// handles this by allowing us to specify that the packet should release the
// allocated arrays upon its destruction. Unfortunately it uses free() to do
// this. So we are forced to use malloc() instead of the new operator, and
// must jump through lots of hoops to handle possibly failed malloc() calls.
//------------------------------------------------------------------------------
NamedStreams::operator MRN::PacketPtr() const
{
    char** names = NULL;
    int* tags = NULL;

    try
    {
        names = reinterpret_cast<char**>(
            malloc(dm_tags.size() * sizeof(char*))
            );
        
        if (names == NULL)
        {
            throw std::bad_alloc();
        }

        memset(names, 0, dm_tags.size() * sizeof(char*));
        
        tags = reinterpret_cast<int*>(
            malloc(dm_tags.size() * sizeof(int))
            );

        if (tags == NULL)
        {
            throw std::bad_alloc();
        }

        int n = 0;
        for (boost::bimap<std::string, int>::const_iterator
                 i = dm_tags.begin(); i != dm_tags.end(); ++i, ++n)
        {
            names[n] = strdup(i->left.c_str());
            if (names[n] == NULL)
            {
                throw std::bad_alloc();
            }
            tags[n] = i->right;
        }

        MRN::PacketPtr packet(new MRN::Packet(
            0, MessageTags::SpecifyNamedStreams, "%d %as %ad",
            dm_uid, names, dm_tags.size(), tags, dm_tags.size()
            ));
        
        packet->set_DestroyData(true);
        
        return packet;
    }
    catch (...)
    {
        release(names, tags, dm_tags.size());
        throw;
    }
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int NamedStreams::uid() const
{
    return dm_uid;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int NamedStreams::tag(const std::string& name) const
{
    boost::bimap<std::string, int>::left_const_iterator i = 
        dm_tags.left.find(name);
    
    if (i == dm_tags.left.end())
    {
        raise<std::runtime_error>(
            "The requested named stream (%1%) doesn't exist.", name
            );
    }
    
    return i->second;
}



//------------------------------------------------------------------------------
// Assign the next available MRNet message tag to the named stream described
// by this <[Incoming|Outgoing][Downstream|Upstream]> XML node.
//------------------------------------------------------------------------------
void NamedStreams::parseStream(const xercesc::DOMNode* node)
{
    const std::string name = xercesc::selectValue(node, "./Name");

    if (dm_tags.left.find(name) == dm_tags.left.end())
    {
        int tag = MessageTags::FirstNamedStreamTag + dm_offset_generator;
        while (dm_tags.right.find(tag) != dm_tags.right.end())
        {
            tag = MessageTags::FirstNamedStreamTag + dm_offset_generator;
        }
        
        dm_tags.insert(boost::bimap<std::string, int>::value_type(name, tag));
    }
}



//------------------------------------------------------------------------------
// Assign the specified MRNet message tag to the named stream described by
// this <Stream> XML node.
//------------------------------------------------------------------------------
void NamedStreams::parseStreamDeclaration(const xercesc::DOMNode* node)
{
    const std::string name = xercesc::selectValue(node, "./Name");
    const int tag = boost::lexical_cast<int>(
        xercesc::selectValue(node, "./Tag")
        );

    if (dm_tags.left.find(name) != dm_tags.left.end())
    {
        raise<std::runtime_error>(
            "The named stream \"%1%\" is already declared.", name
            );
    }
    else if (dm_tags.right.find(tag) != dm_tags.right.end())
    {
        raise<std::runtime_error>(
            "The MRNet message tag \"%1%\" is already declared.", tag
            );
    }
    else if (tag < MessageTags::FirstNamedStreamTag)
    {
        raise<std::runtime_error>(
            "The MRNet message tag \"%1%\" is less than the minimum \"%2%\".",
            tag, MessageTags::FirstNamedStreamTag
            );
    }

    dm_tags.insert(boost::bimap<std::string, int>::value_type(name, tag));
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
AtomicCounter<int> NamedStreams::dm_uid_generator;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
AtomicCounter<int> NamedStreams::dm_offset_generator;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::ostream& KrellInstitute::CBTF::Impl::operator<<(
    std::ostream& stream,
    const NamedStreams& named_streams
    )
{
    stream << "UID " << named_streams.dm_uid << ": {";
    
    for (boost::bimap<std::string, int>::const_iterator
             i = named_streams.dm_tags.begin();
         i != named_streams.dm_tags.end();
         ++i)
    {
        if (i != named_streams.dm_tags.begin())
        {
            stream << ",";
        }
        stream << " \"" << i->left << "\" --> " << i->right;
    }

    stream << " }";
    
    return stream;
}
