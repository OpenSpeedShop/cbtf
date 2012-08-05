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

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/home/classic.hpp>
#include <boost/spirit/home/classic/phoenix.hpp>
#include <KrellInstitute/CBTF/Impl/MRNet.hpp>
#include <set>
#include <string>

#include "ParseDepth.hpp"
#include "XercesExts.hpp"

using namespace boost::spirit::classic;
using namespace KrellInstitute::CBTF::Impl;
using namespace phoenix;



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

    /** Parse the specified <AllOther> node. */
    void parseAllOther(const xercesc::DOMNode* node, bool currently_selected,
                       boost::tribool& selected)
    {
        selected = !currently_selected;
    }
    
    /**
     * Wrapper around the std::min<int>(). The only reason this exists is that
     * GCC 4.3.4 was having trouble compiling the boost::bind of std::min<int>
     * later in this file within the depth parser.
     */
    int minWrapper(int a, int b)
    {
        return std::min<int>(a, b);
    }

    /**
     * Wrapper around the std::max<int>(). The only reason this exists is that
     * GCC 4.3.4 was having trouble compiling the boost::bind of std::max<int>
     * later in this file within the depth parser.
     */
    int maxWrapper(int a, int b)
    {
        return std::max<int>(a, b);
    }

    /** Boost.Spirit (classic) closure containing a boolean value. */
    struct BooleanClosure :
        boost::spirit::classic::closure<BooleanClosure, bool, int>
    {
        member1 value;
        member2 temp;
    };
    
    /** Boost.Spirit (classic) closure containing an integer value. */
    struct IntegerClosure :
        boost::spirit::classic::closure<IntegerClosure, int>
    {
        member1 value;
    };

    /** Boost.Spirit (classic) grammar for parsing an <Expression> node. */
    struct ExpressionGrammar :
        public grammar<ExpressionGrammar, BooleanClosure::context_t>
    {
        bool IsOnLeafCP;
        
        template <typename ScannerType>
        struct definition
        {
            typedef rule<ScannerType, BooleanClosure::context_t> BooleanRule;
            typedef rule<ScannerType, IntegerClosure::context_t> IntegerRule;
            
            rule<ScannerType> expression;

            BooleanRule logical_or, logical_and, boolean, relational;
            IntegerRule additive, multiplicative, integer;
            
            definition(const ExpressionGrammar& self)
            {
                expression = logical_or[self.value = arg1];
                
                logical_or = logical_and[logical_or.value = arg1] >>
                    *(("||" >> logical_and
                       [logical_or.value = logical_or.value || arg1]));
                
                logical_and = boolean[logical_and.value = arg1] >>
                    *(("&&" >> boolean
                       [logical_and.value = logical_and.value && arg1]));

                boolean = relational[boolean.value = arg1] |
                    ('!' >> boolean[boolean.value = !arg1]) |
                    ('(' >> logical_or[boolean.value = arg1] >> ')') |
                    as_lower_d["fe"]
                        [boolean.value = TheTopologyInfo.IsFrontend] |
                    as_lower_d["cp:top"]
                        [boolean.value =
                         !TheTopologyInfo.IsFrontend &&
                         !TheTopologyInfo.IsBackend &&
                         (TheTopologyInfo.RootDistance == 1)] |
                    as_lower_d["cp:middle"]
                        [boolean.value = 
                         !TheTopologyInfo.IsFrontend &&
                         !TheTopologyInfo.IsBackend &&
                         (TheTopologyInfo.RootDistance != 0) &&
                         !self.IsOnLeafCP] |
                    as_lower_d["cp:bottom"]
                        [boolean.value =
                         !TheTopologyInfo.IsFrontend &&
                         !TheTopologyInfo.IsBackend &&
                         self.IsOnLeafCP] |
                    as_lower_d["cp"]
                        [boolean.value = !TheTopologyInfo.IsFrontend &&
                         !TheTopologyInfo.IsBackend] |
                    as_lower_d["be"][boolean.value = TheTopologyInfo.IsBackend];
                
                relational = additive[relational.temp = arg1] >>
                    (("==" >> additive
                          [relational.value = relational.temp == arg1]) |
                     ("!=" >> additive
                          [relational.value = relational.temp != arg1]) |
                     ('<' >> additive
                          [relational.value = relational.temp < arg1]) |
                     ("<=" >> additive
                          [relational.value = relational.temp <= arg1]) |
                     ('>' >> additive
                          [relational.value = relational.temp > arg1]) |
                     (">=" >> additive
                          [relational.value = relational.temp >= arg1]));
                    
                additive = multiplicative[additive.value = arg1] >>
                    *(('+' >> multiplicative[additive.value += arg1]) |
                      ('-' >> multiplicative[additive.value -= arg1]));

                multiplicative = integer[multiplicative.value = arg1] >>
                    *(('*' >> integer[multiplicative.value *= arg1]) |
                      ('/' >> integer[multiplicative.value /= arg1]) |
                      ('%' >> integer[multiplicative.value %= arg1]));

                integer = int_p[integer.value = arg1] |
                    ('+' >> integer[integer.value = arg1]) |
                    ('-' >> integer[integer.value = -arg1]) |
                    ('(' >> additive[integer.value = arg1] >> ')') |
                    as_lower_d["rank"][integer.value = 
                        static_cast<int>(TheTopologyInfo.Rank)] |
                    as_lower_d["numchildren"][integer.value = 
                        static_cast<int>(TheTopologyInfo.NumChildren)] |
                    as_lower_d["numsiblings"][integer.value = 
                        static_cast<int>(TheTopologyInfo.NumSiblings)] |
                    as_lower_d["numdescendants"][integer.value = 
                        static_cast<int>(TheTopologyInfo.NumDescendants)] |
                    as_lower_d["numleafdescendants"][integer.value = 
                        static_cast<int>(TheTopologyInfo.NumLeafDescendants)] |
                    as_lower_d["rootdistance"][integer.value = 
                        static_cast<int>(TheTopologyInfo.RootDistance)] |
                    as_lower_d["maxleafdistance"][integer.value = 
                        static_cast<int>(TheTopologyInfo.MaxLeafDistance)] |
                    (as_lower_d["min"] >> '(' >> 
                     additive[integer.value = arg1] >> ',' >>
                     additive[integer.value =
                              bind(&minWrapper)(integer.value, arg1)] >>
                     ')') ||
                    (as_lower_d["max"] >> '(' >>
                     additive[integer.value = arg1] >> ',' >>
                     additive[integer.value =
                              bind(&maxWrapper)(integer.value, arg1)] >>
                     ')');
            }
            
            const rule<ScannerType>& start() const
            {
                return expression;
            }
            
        }; // struct definition
    }; // struct ExpressionGrammar

    /** Parse the specified <Expression> node. */
    void parseExpression(const xercesc::DOMNode* node,
                         bool is_on_leaf_cp,
                         boost::tribool& selected)
    {
        std::string expression = xercesc::selectValue(node, ".");
        ExpressionGrammar grammar;
        grammar.IsOnLeafCP = is_on_leaf_cp;
        bool value = false;
        if (parse(expression.c_str(), grammar[assign_a(value)], space_p).full)
        {
            selected = value;
        }
    }

} // namespace <anonymous>



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void KrellInstitute::CBTF::Impl::parseDepth(const xercesc::DOMNode* root,
                                            bool is_on_leaf_cp,
                                            bool& selected)
{
    boost::tribool value = boost::indeterminate;

    xercesc::selectNodes(
        root, "./AllOther",
        boost::bind(&parseAllOther, _1, selected, boost::ref(value))
        );

    xercesc::selectNodes(
        root, "./Expression",
        boost::bind(&parseExpression, _1, is_on_leaf_cp, boost::ref(value))
        );

    xercesc::selectNodes(
        root, "./LeafRelative",
        boost::bind(&checkOffset, _1, TheTopologyInfo.MaxLeafDistance,
                    boost::ref(value))
        );
    
    xercesc::selectNodes(
        root, "./RootRelative",
        boost::bind(&checkOffset, _1, TheTopologyInfo.RootDistance,
                    boost::ref(value))
        );

    selected = boost::indeterminate(value) ? false : static_cast<bool>(value);
}
