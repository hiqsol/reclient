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

#include "DomainTrademark.h"

using namespace std;
using namespace domtools;
using namespace eppobject::epp;

epp_string DomainTrademark::toXML()
{
    domtools::xml_string_output xmltext;
    xmltext.setWhitespace(false);

    xmltext.beginTag("trademark:"+(*m_op));
    
//  setCommandSchemaAttributes(xmltext);
    xmltext.putAttribute("xmlns:trademark","urn:afilias:params:xml:ns:ext:info-trademark-1.0");
    xmltext.putAttribute("xsi:schemaLocation","urn:afilias:params:xml:ns:ext:info-trademark-1.0 info-trademark-1.0.xsd");

    if (m_name!=NULL)		xmltext.putTag("trademark:name",		*m_name);
    if (m_date!=NULL)		xmltext.putTag("trademark:date",		*m_date);
    if (m_owner_country!=NULL)	xmltext.putTag("trademark:ownerCountry",	*m_owner_country);
    if (m_country!=NULL)	xmltext.putTag("trademark:country",		*m_country);
    if (m_number!=NULL)		xmltext.putTag("trademark:number",		*m_number);

    xmltext.endTag("trademark:"+(*m_op));

    return xmltext.getString();
};
      
void DomainTrademark::fromXML(const epp_string & xml)
{
    DOM_Document doc = createDOM_Document(xml);

    DOM_Node trademarkNode = domtools::getSingleTag(doc, "trademark:infData");
    if(!trademarkNode.isNull())
    {
        string nodeValue;
        DOM_Node nameNode = domtools::getSingleTag(doc, "trademark:name");
        if(!nameNode.isNull())
        {
            getNodeData(nameNode, nodeValue);
            m_name.ref(new epp_string(nodeValue));
        }
        DOM_Node numberNode = domtools::getSingleTag(doc, "trademark:number");
        if(!nameNode.isNull())
        {
            getNodeData(numberNode, nodeValue);
            m_number.ref(new epp_string(nodeValue));
        }
        DOM_Node ownerCountryNode = domtools::getSingleTag(doc, "trademark:ownerCountry");
        if(!nameNode.isNull())
        {
            getNodeData(ownerCountryNode, nodeValue);
            m_owner_country.ref(new epp_string(nodeValue));
        }
        DOM_Node countryNode = domtools::getSingleTag(doc, "trademark:country");
        if(!nameNode.isNull())
        {
            getNodeData(countryNode, nodeValue);
            m_country.ref(new epp_string(nodeValue));
        }
        DOM_Node dateNode = domtools::getSingleTag(doc, "trademark:date");
        if(!nameNode.isNull())
        {
            getNodeData(dateNode, nodeValue);
            m_date.ref(new epp_string(nodeValue));
        }

    } // endif for trademark node

};


