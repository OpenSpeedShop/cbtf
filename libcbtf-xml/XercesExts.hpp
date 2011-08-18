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

/** @file Declaration of extensions to the standard Xerces-C++ library. */

#pragma once

#include <boost/filesystem.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_BEGIN

    /**
     * Parse and validate the tree found in the specified file. Return the
     * resulting document to the caller.
     *
     * @param path    Path of the file to load.
     * @return        Document contained in the loaded file.
     *
     * @throw std::runtime_error    The specified file doesn't exist.
     */
    boost::shared_ptr<DOMDocument> loadFromFile(
        const boost::filesystem::path& path
        );

    /**
     * Parse the tree found in the specified string. Return the resulting
     * document to the caller.
     *
     * @param value    String containing the tree to load.
     * @return         Document contained in the loaded string.
     */
    boost::shared_ptr<DOMDocument> loadFromString(const std::string& value);

    /**
     * Save the tree rooted at the specified node to a string. Return the
     * resulting string to the caller.
     *
     * @param root    Root node of the tree to be saved.
     * @return        String containing that tree.
     */
    std::string saveToString(const DOMNode* root);

    /**
     * Select the set of nodes matching the given XPath expression from
     * the tree rooted at the specified node. Invoke the given function
     * for each of the selected nodes.
     *
     * @param root          Root node of the tree.
     * @param expression    XPath expression to be evaluated.
     * @param function      Function to be invoked for each selected node.
     *
     * @throw std::runtime_error    The specified XPath expression
     *                              couldn't be evaluated.
     *
     * @note    Because Xerces-C++ 2.8.x and 3.0.x do not support the use
     *          of XPath for querying DOM element trees, this function is
     *          implemented using a custom parser supporting only a basic
     *          subset of the full XPath language.
     *
     * @sa http://en.wikipedia.org/wiki/Xpath
     */
    void selectNodes(
        const DOMNode* root, const std::string& expression,
        const boost::function<void (const DOMNode*)>& function
        );

    /**
     * Select the first node or attribute value matching the given XPath
     * expression from the tree rooted at the specified node. Return the
     * value to the caller.
     *
     * @param root          Root node of the tree.
     * @param expression    XPath expression to be evaluated.
     * @return              First matching node or attribute value.
     *
     * @throw std::runtime_error    The specified XPath expression
     *                              couldn't be evaluated.
     *
     * @note    Because Xerces-C++ 2.8.x and 3.0.x do not support the use
     *          of XPath for querying DOM element trees, this function is
     *          implemented using a custom parser supporting only a basic
     *          subset of the full XPath language.
     *
     * @sa http://en.wikipedia.org/wiki/Xpath
     */
    std::string selectValue(const DOMNode* root, const std::string& expression);

XERCES_CPP_NAMESPACE_END
