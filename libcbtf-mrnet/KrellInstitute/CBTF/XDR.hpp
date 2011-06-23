////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2011 Krell Institute. All Rights Reserved.
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

/** @file Declaration and definition of the XDR/MRNet conversion components. */

#pragma once

#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <KrellInstitute/CBTF/Component.hpp>
#include <KrellInstitute/CBTF/Impl/Raise.hpp>
#include <KrellInstitute/CBTF/Type.hpp>
#include <KrellInstitute/CBTF/Version.hpp>
#include <mrnet/Packet.h>
#include <rpc/rpc.h>
#include <stdexcept>
#include <typeinfo>

namespace KrellInstitute { namespace CBTF {
    
    /**
     * Component converting the template-specified XDR type into a MRNet packet.
     *
     * @tparam T    XDR type converted into a MRNet packet by this component.
     */
    template <typename T>
    class __attribute__ ((visibility ("hidden"))) ConvertXDRToMRNet :
        public Component
    {
        
    public:

        /**
         * Factory function for this component type.
         *
         * @xdr_proc    XDR procedure for the specified XDR type.
         * @return      A new instance of this componenttype.
         */
        static Component::Instance factoryFunction(const xdrproc_t& xdr_proc)
        {
            return Component::Instance(
                reinterpret_cast<Component*>(new ConvertXDRToMRNet(xdr_proc))
                );
        }
        
    private:

        /** Constructor from the XDR procedure for the specified XDR type. */
        ConvertXDRToMRNet(const xdrproc_t& xdr_proc) :
            Component(Type(typeid(ConvertXDRToMRNet)), Version(0, 0, 0)),
            _xdr_proc(xdr_proc)
        {
            if (_xdr_proc == NULL)
            {
                Impl::raise<std::runtime_error>(
                    "The XDR procedure for type \"%1%\" was null.",
                    typeid(T).name()
                    );
            }
            
            declareInput<boost::shared_ptr<T> >(
                "in", boost::bind(&ConvertXDRToMRNet::handler, this, _1)
                );
            declareOutput<MRN::PacketPtr>("out");
        }
        
        /** Handler for the "in" input. */
        void handler(const boost::shared_ptr<T>& in)
        {
            for (unsigned int size = 1024; size > 0; size *= 2)
            {
                boost::scoped_ptr<char> contents(new char[size]);

                XDR xdrs;
                xdrmem_create(&xdrs, contents.get(), size, XDR_ENCODE);

                if ((*_xdr_proc)(&xdrs, (void*)in.get()) == TRUE)
                {
                    size = xdr_getpos(&xdrs);

                    emitOutput<MRN::PacketPtr>(
                        "out",
                        MRN::PacketPtr(
                            new MRN::Packet(0, 0, "%auc", contents.get(), size)
                            )
                        );
                    
                    size = 0;
                }
                
                xdr_destroy(&xdrs);
            }            
        }
        
        /** XDR procedure for the specified XDR type. */
        const xdrproc_t _xdr_proc;

    }; // class ConvertXDRToMRNet<T>
   
    /**
     * Component converting a MRNet packet into the template-specified XDR type.
     *
     * @tparam T    XDR type converted from a MRNet packet by this component.
     */
    template <typename T>
    class __attribute__ ((visibility ("hidden"))) ConvertMRNetToXDR :
        public Component
    {
        
    public:

        /**
         * Factory function for this component type.
         *
         * @xdr_proc    XDR procedure for the specified XDR type.
         * @return      A new instance of this componenttype.
         */
        static Component::Instance factoryFunction(const xdrproc_t& xdr_proc)
        {
            return Component::Instance(
                reinterpret_cast<Component*>(new ConvertMRNetToXDR(xdr_proc))
                );
        }
        
    private:

        /** Constructor from the XDR procedure for the specified XDR type. */
        ConvertMRNetToXDR(const xdrproc_t& xdr_proc) :
            Component(Type(typeid(ConvertMRNetToXDR)), Version(0, 0, 0)),
            _xdr_proc(xdr_proc)
        {
            if (_xdr_proc == NULL)
            {
                Impl::raise<std::runtime_error>(
                    "The XDR procedure for type \"%1%\" was null.",
                    typeid(T).name()
                    );
            }

            declareInput<MRN::PacketPtr>(
                "in", boost::bind(&ConvertMRNetToXDR::handler, this, _1)
                );
            declareOutput<boost::shared_ptr<T> >("out");
        }
        
        /** Handler for the "in" input. */
        void handler(const MRN::PacketPtr& in)
        {
            void* contents = NULL;
            unsigned int size = 0;

            if ((in->unpack("%auc", &contents, &size) != 0) ||
                (contents == NULL) || (size == 0))
            {
                Impl::raise<std::runtime_error>(
                    "The incoming message could not be unpacked."
                    );
            }

            XDR xdrs;
            xdrmem_create(
                &xdrs, reinterpret_cast<char*>(contents), size, XDR_DECODE
                );

            boost::shared_ptr<T> value(new T());
            if ((*_xdr_proc)(&xdrs, value.get()) == FALSE)
            {
                Impl::raise<std::runtime_error>(
                    "The incoming message could not be decoded."
                    );                
            }
            
            xdr_destroy(&xdrs);
            free(contents);

            emitOutput<boost::shared_ptr<T> >("out", value);
        }
        
        /** XDR procedure for the specified XDR type. */
        const xdrproc_t _xdr_proc;
        
    }; // class ConvertMRNetToXDR<T>
        
} } // namespace KrellInstitute::CBTF

/**
 * Macro definition that generates a statically initialized C++ structure
 * registering the factory functions for converting the specified XDR type
 * to/from a MRNet packet.
 *
 * @param type    Type (XDR structure name) to be registered.
 */
#define KRELL_INSTITUTE_CBTF_REGISTER_XDR_CONVERTERS(type)                     \
    namespace KrellInstitute { namespace CBTF { namespace Impl {               \
        template <typename T> struct __attribute__ ((visibility ("hidden")))   \
        RegisterFactoryFunction;                                               \
        template <> struct __attribute__ ((visibility ("hidden")))             \
        RegisterFactoryFunction<type>                                          \
        {                                                                      \
            RegisterFactoryFunction()                                          \
            {                                                                  \
                KrellInstitute::CBTF::Component::registerFactoryFunction(      \
                    boost::bind(                                               \
                        &ConvertXDRToMRNet<type>::factoryFunction,             \
                        reinterpret_cast<xdrproc_t>(&xdr_ ## type)             \
                        )                                                      \
                    );                                                         \
                KrellInstitute::CBTF::Component::registerFactoryFunction(      \
                    boost::bind(                                               \
                        &ConvertMRNetToXDR<type>::factoryFunction,             \
                        reinterpret_cast<xdrproc_t>(&xdr_ ## type)             \
                        )                                                      \
                    );                                                         \
            }                                                                  \
            static RegisterFactoryFunction instance;                           \
        };                                                                     \
        RegisterFactoryFunction<type> RegisterFactoryFunction<type>::instance; \
    } } }
