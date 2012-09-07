////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010-2012 Krell Institute. All Rights Reserved.
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

/** @file Definition of extensions to the standard Xerces-C++ library. */

#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/spirit/home/classic.hpp>
#include <deque>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "Raise.hpp"
#include "XercesExts.hpp"

using namespace KrellInstitute::CBTF::Impl;
XERCES_CPP_NAMESPACE_USE



/** Anonymous namespace hiding implementation details. */
namespace {

    /**
     * Statically initialized C++ structure that automatically initializes
     * and terminates the Xerces-C++ library.
     */
    struct AutoInitializeXercesC
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
     * Handle XML parsing warnings and errors by queueing them up and then
     * later throwing them as an exception.
     */
    class ParsingExceptionHandler :
        public ErrorHandler
    {

    public:

        /** Default constructor. */
        ParsingExceptionHandler() :
            ErrorHandler(),
            dm_exceptions()
        {
        }

        /** Destructor. */
        virtual ~ParsingExceptionHandler()
        {
        }
        
        /** Receive notification of a warning. */
        virtual void warning(const SAXParseException& exc)
        {
            queue("Warning", exc);
        }

        /** Receive notification of a recoverable error. */
        virtual void error(const SAXParseException& exc)
        {
            queue("Error", exc);
        }

        /** Receive notification of a non-recoverable error. */
        virtual void fatalError(const SAXParseException& exc)
        {
            queue("Fatal Error", exc);
        }

        /** Reset the error handler object on its reuse. */
        virtual void resetErrors()
        {
            dm_exceptions.clear();
        }

        /** Throw any parsing warnings and errors as a single exception. */
        void throwExceptions() const
        {
            std::string what;

            for (std::vector<std::string>::const_iterator
                     i = dm_exceptions.begin(); i != dm_exceptions.end(); ++i)
            {
                what += *i + "\n\n";
            }
            
            if (!what.empty())
            {
                throw std::runtime_error(what.c_str());
            }
        }
        
    private:

        /** Queue of any parsing warnings and errors. */
        std::vector<std::string> dm_exceptions;
        
        /** Queue a parsing warning or error. */
        void queue(const std::string& type, const SAXParseException& error)
        {
            char* file = XMLString::transcode(error.getSystemId());
            char* message = XMLString::transcode(error.getMessage());

            std::string what = boost::str(
                boost::format("%1% (%2%, Line %4%, Column %5%): %3%") %
                type % file % message %
                error.getLineNumber() % error.getColumnNumber()
                );
            
            XMLString::release(&file);
            XMLString::release(&message);
            
            dm_exceptions.push_back(what);
        }

    }; // class ParsingExceptionHandler

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
//------------------------------------------------------------------------------
boost::shared_ptr<DOMDocument> XERCES_CPP_NAMESPACE_QUALIFIER loadFromFile(
    const boost::filesystem::path& path,
    const std::vector<boost::filesystem::path>& schema_paths
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
        ParsingExceptionHandler handler;
        parser->setErrorHandler(&handler);
    
        if (!schema_paths.empty())
        {
            parser->setDoNamespaces(true);
            parser->setDoSchema(true);
            parser->setLoadSchema(false);
            //
            // Uncomment the line below to enable XML schema validation. The
            // actual validation appears to be working now. But unfortunately
            // the MRNet unit test's XML (test-mrnet.xml) won't validate. For
            // some unknown reason the parser thinks the <Network> tag usage
            // in that file is incorrect. The problem almost certainly has to
            // do with the fact that <Network> is in a different namespace,
            // but I can't figure out how to make it work out... Until then,
            // validation is still disabled.
            //
            // parser->setValidationScheme(AbstractDOMParser::Val_Always);
            parser->setValidationSchemaFullChecking(true);
            parser->useCachedGrammarInParse(true);
            
            for (std::vector<boost::filesystem::path>::const_iterator
                     i = schema_paths.begin(); i != schema_paths.end(); ++i)
            {
                if (!is_regular_file(*i))
                {
                    raise<std::runtime_error>(
                        "The specified schema file (%1%) doesn't exist.", *i
                        );
                }

                parser->loadGrammar(
                    i->string().c_str(), Grammar::SchemaGrammarType, true
                    );
            }
        }
        
        parser->parse(path.string().c_str());
        document = parser->adoptDocument();
        
        handler.throwExceptions();
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
