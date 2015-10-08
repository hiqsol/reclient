#ifndef __PROSUPPLEMENTALDATA_H
#define __PROSUPPLEMENTALDATA_H

#include "epp-rtk-cpp/data/epp_eppXMLbase.h"

namespace eppobject { namespace epp {

    class ProSupplementalData : public epp_Extension {
    public:
        epp_string_ref m_op;
        epp_string_ref m_profession;
        epp_string_ref m_authorityName;
        epp_string_ref m_authorityUrl;
        epp_string_ref m_licenseNumber;

    public:
        ProSupplementalData() {};

        virtual ~ProSupplementalData() {};

        virtual epp_string toXML();
        virtual epp_string quoted (const epp_string & str);

        virtual void fromXML(const eppobject::epp::epp_string & xml);

    };  // end class definition

    typedef refcnt_ptr<ProSupplementalData> ProSupplementalData_ref;

}}  // end namespaces

#endif
