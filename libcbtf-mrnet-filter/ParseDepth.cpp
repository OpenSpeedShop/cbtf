////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2012 Krell Institute. All Rights Reserved.
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

/** @file Definition of the depth parsing function. */

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/home/classic.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <set>
#include <string>

#include "ParseDepth.hpp"
#include "XercesExts.hpp"

using namespace KrellInstitute::CBTF::Impl;



/** Anonymous namespace hiding implementation details. */
namespace {

    /** Insert the value of the specified node into the given integer set. */
    void insertInteger(const xercesc::DOMNode* node, std::set<int>& integers)
    {
        integers.insert(
            boost::lexical_cast<int>(xercesc::selectValue(node, "."))
            );
    }

    /** Compare the given offset against the ones in the specified node. */
    void checkOffset(const xercesc::DOMNode* node, int offset,
                     boost::tribool& selected)
    {
        std::set<int> offsets;

        xercesc::selectNodes(
            node, "./Offset",
            boost::bind(&insertInteger, _1, boost::ref(offsets))
            );
        
        std::string value = xercesc::selectValue(node, "./@offset");
        
        if (!value.empty())
        {
            offsets.insert(boost::lexical_cast<int>(value));
        }
        
        selected = (offsets.find(offset) != offsets.end());
    }
    
    /** Handle <AllOther> by selecting if not currently selected. */
    void allOther(const xercesc::DOMNode* node, bool currently_selected,
                  boost::tribool& selected)
    {
        selected = !currently_selected;
    }
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void KrellInstitute::CBTF::Impl::parseDepth(const xercesc::DOMNode* root,
                                            bool& selected)
{
    boost::tribool value = boost::indeterminate;
    
    if (boost::indeterminate(value))
    {
        xercesc::selectNodes(
            root, "./LeafRelative",
            boost::bind(&checkOffset, _1, TheTopologyInfo.MaxLeafDistance,
                        boost::ref(value))
            );        
    }

    if (boost::indeterminate(value))
    {
        xercesc::selectNodes(
            root, "./RootRelative",
            boost::bind(&checkOffset, _1, TheTopologyInfo.RootDistance,
                        boost::ref(value))
            );
    }

    if (boost::indeterminate(value))
    {
        xercesc::selectNodes(
            root, "./AllOther",
            boost::bind(&allOther, _1, selected, boost::ref(value))
            );
    }

    if (boost::indeterminate(value))
    {
        value = false;
    }
    
    selected = value;
}
