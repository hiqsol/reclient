#include "XxxLaunch.h"

using namespace std;
using namespace domtools;
using namespace eppobject::epp;

epp_string XxxLaunch::toXML()
{
    domtools::xml_string_output xmltext;
    xmltext.setWhitespace(false);

    xmltext.beginTag("launch:"+(*m_op));

    xmltext.putAttribute("xmlns:launch","urn:ietf:params:xml:ns:launch-1.0");
    if (*m_op == "check") {
        xmltext.putAttribute("type","avail");
    } else if (*m_op == "create") {
        xmltext.putAttribute("xsi:schemaLocation","urn:ietf:params:xml:ns:launch-1.0 launch-1.0.xsd");
    };

    xmltext.beginTag("launch:phase");
    if (m_name != NULL && *m_name != "") {
        xmltext.putAttribute("name",*m_name);
    };
    xmltext.putCDATA(*m_phase);
    xmltext.endTag("launch:phase");

    if (m_noticeID != NULL && *m_noticeID != "") {
        xmltext.beginTag("launch:notice");
            xmltext.putTag("launch:noticeID",       *m_noticeID);
            xmltext.putTag("launch:notAfter",       *m_notAfter);
            xmltext.putTag("launch:acceptedDate",   *m_acceptedDate);
        xmltext.endTag("launch:notice");
    };

    xmltext.endTag("launch:"+(*m_op));

printf("EXT XML: %s\n",xmltext.getString().c_str());
    return xmltext.getString();
};

void XxxLaunch::fromXML (const epp_string & xml) {
printf("fromXML: %s",xml.c_str());
    DOM_Document doc = createDOM_Document(xml);

    DOM_Node dataNode = domtools::getSingleTag(doc, "launch:infData");
    if (!dataNode.isNull()) {
        string nodeValue;
        DOM_Node valueNode = domtools::getSingleTag(doc, "launch:value");
        if (!valueNode.isNull()) {
            getNodeData(valueNode, nodeValue);
            m_phase.ref(new epp_string(nodeValue));
        };
    };

};


