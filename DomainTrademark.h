/************************************************************************
* EPP RTK C++
* Copyright (C) 2001 Global Name Registry 
* 
* This library is free software; you can redistribute it and/or modify it
* under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation; either version 2.1 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Contact information:  epprtk@gnr.com
*
*                       EPP RTK
*                       GNR Ltd.
*                       125 High Holborn
*                       London WC1V 6QA
*                       United Kingdom
************************************************************************/

#ifndef __DOMAINTRADEMARK_H
#define __DOMAINTRADEMARK_H

#include "epp-rtk-cpp/config.h"
#include <domtools/dom_wrapper.h>
#include "epp-rtk-cpp/data/epp_eppXMLbase.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"

namespace eppobject { namespace epp {
   
   class DomainTrademark : public epp_Extension {
    public:
	epp_string_ref m_op;
        epp_string_ref m_name;
        epp_string_ref m_date;
        epp_string_ref m_number;
        epp_string_ref m_owner_country;
        epp_string_ref m_country;
      
    public:
        DomainTrademark() {};

        virtual ~DomainTrademark() {};
      
        virtual eppobject::epp::epp_string toXML();

        virtual void fromXML(const eppobject::epp::epp_string & xml);

    };  // end class definition

    typedef refcnt_ptr<DomainTrademark> DomainTrademark_ref;

    //void setCommandSchemaAttributes (eppobject::epp::EPP_output & output);
   
}}  // end namespaces

#endif
