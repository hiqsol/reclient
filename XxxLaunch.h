#ifndef __XXX_LAUNCH_H
#define __XXX_LAUNCH_H

#include "epp-rtk-cpp/data/epp_eppXMLbase.h"

namespace eppobject { namespace epp {

    class XxxLaunch : public epp_Extension {
    public:
        epp_string_ref m_op;
        epp_string_ref m_phase;
        epp_string_ref m_name;
        epp_string_ref m_noticeID;
        epp_string_ref m_notAfter;
        epp_string_ref m_acceptedDate;

    public:
        XxxLaunch() {};

        virtual ~XxxLaunch() {};

        virtual epp_string toXML();

        virtual void fromXML(const eppobject::epp::epp_string & xml);

    };  // end class definition

    typedef refcnt_ptr<XxxLaunch> XxxLaunch_ref;

}}  // end namespaces

#endif
