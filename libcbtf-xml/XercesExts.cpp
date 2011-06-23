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

/** @file Definition of extensions to the standard Xerces-C++ library. */

#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/home/classic.hpp>
#include <deque>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <KrellInstitute/CBTF/Impl/XercesExts.hpp>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>

using namespace KrellInstitute::CBTF::Impl;
XERCES_CPP_NAMESPACE_USE



/** Anonymous namespace hiding implementation details. */
namespace {

    /**
     * Statically initialized C++ structure that automatically initializes
     * and terminates the Xerces-C++ library.
     */
    struct __attribute__ ((visibility("hidden"))) AutoInitializeXercesC
    {
        
        /** Default constructor. Initialize the Xerces-C++ library. */
        AutoInitializeXercesC()
        {
            XMLPlatformUtils::Initialize();
        }
        
        /** Destructor. Terminate the Xerces-C++ library. */
        ~AutoInitializeXercesC()
        {
            XMLPlatformUtils::Terminate();
        }
        
    } auto_initialize_xercesc;

    /**
     * Deleter for documents.
     *
     * @todo    The call to release() causes a segmentation fault, so
     *          it is commented out for now. This results in a memory
     *          leak that needs to be fixed.
     */
    void deleteDocument(DOMDocument* document)
    {
        // document->release();
    }
    
    /** Get the value of the specified node. */
    void getValue(const DOMNode* node, std::string& value)
    {
        if ((node->getNodeType() != DOMNode::ELEMENT_NODE) &&
            (node->getNodeType() != DOMNode::ATTRIBUTE_NODE))
        {
            return;
        }

        DOMNodeList* children = node->getChildNodes();
        for (XMLSize_t i = 0; i < children->getLength(); ++i)
        {
            DOMNode* child = children->item(i);
            if (child->getNodeType() == DOMNode::TEXT_NODE)
            {
                char* transcoded_value = XMLString::transcode(
                    reinterpret_cast<DOMText*>(child)->getWholeText()
                    );
                if (transcoded_value == NULL)
                {
                    raise<std::runtime_error>(
                        "Transcoding of the child node's text failed."
                        );
                }
                value = boost::algorithm::trim_copy(
                    std::string(transcoded_value)
                    );
                XMLString::release(&transcoded_value);
                break;
            }
        }
    }

    /**
     * Parse the specified XPath expression for the given node, applying
     * the specified function for each matching node.
     */
    void parse(const DOMNode* root, const std::string& expression,
               const boost::function<void (const DOMNode*)>& function)
    {
        using namespace boost::spirit::classic;

        std::vector<std::string> path;

        parse(expression.c_str(),
              list_p((*anychar_p)[push_back_a(path)], ch_p('/')),
              space_p);

        if (path.empty())
        {
            return;
        }

        const DOMNode* initial_node = root;
        std::vector<std::string>::size_type initial_index = 0;

        if (path[0].empty())
        {
            if (root->getNodeType() != DOMNode::DOCUMENT_NODE)
            {
                if (root->getOwnerDocument() == NULL)
                {
                    raise<std::runtime_error>(
                        "The root node has no owner document."
                        );
                }
                initial_node = root->getOwnerDocument()->getDocumentElement();
            }
            initial_index = 1;
        }
        
        std::set<const DOMNode*> matches;
        
        for (std::deque<
                 std::pair<const DOMNode*, std::vector<std::string>::size_type>
                 > queue(1, std::make_pair(initial_node, initial_index));
             !queue.empty();
             queue.pop_front())
        {
            const DOMNode* node = queue.front().first;
            std::vector<std::string>::size_type index = queue.front().second;

            if (index == path.size())
            {
                matches.insert(node);
            }

            else if (path[index].empty() /* "//" */)
            {
                queue.push_back(std::make_pair(node, index + 1));
                for (std::deque<const DOMNode*> all(1, node);
                     !all.empty();
                     all.pop_front())
                {
                    DOMNodeList* children = all.front()->getChildNodes();
                    for (XMLSize_t i = 0; i < children->getLength(); ++i)
                    {
                        DOMNode* child = children->item(i);
                        if (child->getNodeType() == DOMNode::ELEMENT_NODE)
                        {
                            queue.push_back(std::make_pair(child, index + 1));
                        }
                    }
                }                
            }
            
            else if (path[index] == "*")
            {
                DOMNodeList* children = node->getChildNodes();
                for (XMLSize_t i = 0; i < children->getLength(); ++i)
                {
                    DOMNode* child = children->item(i);
                    if (child->getNodeType() == DOMNode::ELEMENT_NODE)
                    {
                        queue.push_back(std::make_pair(child, index + 1));
                    }
                }
            }

            else if (path[index] == "..")
            {
                if (node->getParentNode() == NULL)
                {
                    raise<std::runtime_error>(
                        "The current node has no parent node."
                        );
                }
                queue.push_back(
                    std::make_pair(node->getParentNode(), index + 1)
                    );
            }

            else if (path[index] == ".")
            {
                queue.push_back(std::make_pair(node, index + 1));
            }

            else if (path[index][0] == '@')
            {
                if (node->getNodeType() == DOMNode::ELEMENT_NODE)
                {
                    XMLCh* value = XMLString::transcode(&(path[index][1]));
                    if (value == NULL)
                    {
                        raise<std::runtime_error>(
                            "Transcoding of the attribute's name failed."
                            );
                    }

                    DOMAttr* attribute = reinterpret_cast<const DOMElement*>(
                        node
                        )->getAttributeNode(value);

                    if (attribute != NULL)
                    {
                        queue.push_back(std::make_pair(attribute, index + 1));
                    }
                    
                    XMLString::release(&value);
                }
            }

            else
            {
                XMLCh* value = XMLString::transcode(path[index].c_str());
                if (value == NULL)
                {
                    raise<std::runtime_error>(
                        "Transcoding of the node's name failed."
                        );
                }

                DOMNodeList* children = node->getChildNodes();
                for (XMLSize_t i = 0; i < children->getLength(); ++i)
                {
                    DOMNode* child = children->item(i);
                    if ((child->getNodeType() == DOMNode::ELEMENT_NODE) &&
                        XMLString::equals(child->getNodeName(), value))
                    {
                        queue.push_back(std::make_pair(child, index + 1));
                    }
                }
                
                XMLString::release(&value);
            }
            
        }

        for (std::set<const DOMNode*>::const_iterator
                 i = matches.begin(); i != matches.end(); ++i)
        {
            function(*i);
        }
    }
    
} // namespace <anonymous>



//------------------------------------------------------------------------------
// TODO: Schema validation does not appear to be working currently. The schema
//       files are found and loaded properly. But documents don't appear to be
//       validated against the schema. This should be fixed in order to provide
//       better error reporting.
//------------------------------------------------------------------------------
boost::shared_ptr<DOMDocument> XERCES_CPP_NAMESPACE_QUALIFIER loadFromFile(
    const boost::filesystem::path& path
    )
{
    using namespace boost::filesystem;
    
    if (!is_regular_file(path))
    {
        raise<std::runtime_error>(
            "The specified file (%1%) doesn't exist.", path
            );
    }

    DOMDocument* document = NULL;
    XercesDOMParser* parser = new XercesDOMParser();
    
    try
    {
        boost::filesystem::path schema_directory = SCHEMA_DIR;
        
        if (is_directory(schema_directory))
        {
            bool found_schema = false;

            for (directory_iterator i(schema_directory);
                 i != directory_iterator();
                 ++i)
            {
                if (is_regular_file(*i) && (extension(*i) == ".xsd"))
                {
                    found_schema = true;
                    parser->loadGrammar(
                        i->string().c_str(), Grammar::SchemaGrammarType, true
                        );
                }
            }

            if (found_schema)
            {
                parser->setDoNamespaces(true);
                parser->setDoSchema(true);
                parser->setValidationScheme(XercesDOMParser::Val_Always);
                parser->useCachedGrammarInParse(true);
            }
        }

        parser->parse(path.string().c_str());
        document = parser->adoptDocument();
    }
    catch (...)
    {
        delete parser;
        throw;
    }

    delete parser;
    return boost::shared_ptr<DOMDocument>(document, deleteDocument);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
boost::shared_ptr<DOMDocument> XERCES_CPP_NAMESPACE_QUALIFIER loadFromString(
    const std::string& value
    )
{
    const XMLCh kBufferId[9] = { 
        chLatin_C, chLatin_B, chLatin_T, chLatin_F, chDash,
        chLatin_X, chLatin_M, chLatin_L, chNull
    };

    DOMDocument* document = NULL;
    XercesDOMParser* parser = new XercesDOMParser();
    
    try
    {
        MemBufInputSource source(
            reinterpret_cast<const XMLByte*>(&value[0]), value.size(), kBufferId
            );
        
        parser->parse(source);
        document = parser->adoptDocument();
    }
    catch (...)
    {
        delete parser;
        throw;
    }

    delete parser;
    return boost::shared_ptr<DOMDocument>(document, deleteDocument);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string XERCES_CPP_NAMESPACE_QUALIFIER saveToString(const DOMNode* root)
{
    const XMLCh kImplementationName[3] = { chLatin_L, chLatin_S, chNull };

    DOMImplementationLS* implementation = 
        reinterpret_cast<DOMImplementationLS*>(
            DOMImplementationRegistry::getDOMImplementation(kImplementationName)
            );
    if (implementation == NULL)
    {
        raise<std::runtime_error>(
            "Creation of a DOM load/save implementation failed."
            );
    }
    
    DOMLSSerializer* serializer = implementation->createLSSerializer();
    if (serializer == NULL)
    {
        raise<std::runtime_error>(
            "Creation of a DOM load/save serializer failed."
            );
    }

    XMLCh* value = serializer->writeToString(root);
    if (value == NULL)
    {
        raise<std::runtime_error>(
            "Serialization of the specified node failed."
            );
    }
    
    char* transcoded_value = XMLString::transcode(value);
    if (transcoded_value == NULL)
    {
        raise<std::runtime_error>(
            "Transcoding of the serialized text failed."
            );
    }
    
    std::string retval(transcoded_value);
    
    XMLString::release(&transcoded_value);
    XMLString::release(&value);

    serializer->release();

    return "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" + retval;
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void XERCES_CPP_NAMESPACE_QUALIFIER selectNodes(
    const DOMNode* root, const std::string& expression,
    const boost::function<void (const DOMNode*)>& function
    )
{
    parse(root, expression, function);
}



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::string XERCES_CPP_NAMESPACE_QUALIFIER selectValue(
    const DOMNode* root, const std::string& expression
    )
{
    std::string value;
    parse(root, expression, boost::bind(&getValue, _1, boost::ref(value)));
    return value;
}
