#include "ProSupplementalData.h"

using namespace std;
using namespace domtools;
using namespace eppobject::epp;

epp_string ProSupplementalData::toXML()
{
    domtools::xml_string_output xmltext;
    xmltext.setWhitespace(false);

    xmltext.beginTag("supplementalData:"+(*m_op));

    xmltext.putAttribute("xmlns:supplementalData","urn:afilias:params:xml:ns:supplementalData-1.0");
    //xmltext.putAttribute("xsi:schemaLocation","urn:afilias:params:xml:ns:supplementalData-1.0 supplementalData-1.0.xsd");

    epp_string json = "{"+
        quoted("profession")    + ": " + quoted(*m_profession)    + ", " +
        quoted("authorityName") + ": " + quoted(*m_authorityName) + ", " +
        quoted("authorityUrl")  + ": " + quoted(*m_authorityUrl)  + ", " +
        quoted("licenseNumber") + ": " + quoted(*m_licenseNumber) +
    "}";
    xmltext.beginTag("supplementalData:value");
    xmltext.putCDATA(json); /// XXX
    xmltext.endTag("supplementalData:value");

    xmltext.endTag("supplementalData:"+(*m_op));

printf("EXT XML: %s\n",xmltext.getString().c_str());
    return xmltext.getString();
};

epp_string ProSupplementalData::quoted (const epp_string & str) {
    return "\""+str+"\"";
}

void ProSupplementalData::fromXML (const epp_string & xml) {
printf("fromXML: %s",xml.c_str());
    DOM_Document doc = createDOM_Document(xml);

    DOM_Node dataNode = domtools::getSingleTag(doc, "supplementalData:infData");
    if (!dataNode.isNull()) {
        string nodeValue;
        DOM_Node valueNode = domtools::getSingleTag(doc, "supplementalData:value");
        if (!valueNode.isNull()) {
            getNodeData(valueNode, nodeValue);
            m_profession.ref(new epp_string(nodeValue));
        };
    };

};


