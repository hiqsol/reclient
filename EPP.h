// (c) Andrii Vasyliev
// EPP - EPP interface

#ifndef __RE_EPP_H__
#define __RE_EPP_H__

#include "rebase/Types/Hash.h"
#include "rebase/Types/Data.h"
#include "epp-rtk-cpp/epp_Session.h"
#include "epp-rtk-cpp/data/epp_Command.h"
#include "epp-rtk-cpp/data/epp_AuthInfo.h"
#include "epp-rtk-cpp/data/epp_domaindata.h"
#include "nameaddon/data/epp_emailFwddata.h"
#include "epp-rtk-cpp/data/epp_hostdata.h"
#include "epp-rtk-cpp/data/epp_ContactNameAddress.h"
#include "epp-rtk-cpp/data/epp_ContactPhone.h"
#include "epp-rtk-cpp/data/epp_PollResData.h"
#include "epp-rtk-cpp/data/epp_Response.h"
#include "epp-rtk-cpp/transport/transports.h"
#include "comnetaddon/epp_NamestoreExt.h"

/// extensions
#include "liberty-org-extensions/SecDNSDsData.h"

using namespace epptransport;
using namespace eppobject::epp;
using namespace eppobject::domain;
using namespace eppobject::emailFwd;
using namespace eppobject::host;
using namespace eppobject::contact;
using namespace eppobject::namestoreExt;

namespace re {

class EPP {
public:
	EPP () {};
	EPP (line_cref h,size_type p,line_cref c,line_cref f="",line_cref d="")
		: host(h),port(p),certificate(c),cacertfile(f),cacertdir(d),serialNo("0"),batchNo(1),commandNo(1)
	{
		transport = new epp_TransportSSL(certificate,cacertfile,cacertdir);
		//printf("certificate:%s cacertfile:%s cacertdir:%s\n",certificate.c_str(),cacertfile.c_str(),cacertdir.c_str());
		transport->setServerName(host);
		transport->setServerPort(port);
		refcnt_ptr<epp_TransportSSL> tref(dynamic_cast<epp_TransportSSL*>(transport));
		session.setTransport(tref);
	};

	void_type setCredentials		(line_cref u,line_cref p) { username = u;password = p; };
	void_type setNamestoreExtension		(line_cref ext,line_cref data);
	void_type setSerialNo			(line_cref s) { serialNo = s; };
	size_type incBatchNo			() { return ++batchNo; };
	void_type dump				() const { printf("EPP: %p\n",this);extensions.dump(); };

// NATIVE EPP COMMANDS
public:
	data_type poll				(data_cref a=data_null);
	data_type hello				(data_cref a=data_null);
	data_type login				(data_cref a=data_null);
	data_type logout			(data_cref a=data_null);

	data_type domainInfo			(data_cref a);
	data_type domainSync			(data_cref a);
	data_type domainCheck			(data_cref a);
	data_type domainSimpleCheck		(data_cref a);
	data_type domainRenew			(data_cref a);
	data_type domainCreate			(data_cref a);
	data_type domainUpdate			(data_cref a);
	data_type domainDelete			(data_cref a);
	data_type domainTransfer		(data_cref a);

	data_type emailFwdInfo			(data_cref a);
	data_type emailFwdCheck			(data_cref a);
	data_type emailFwdRenew			(data_cref a);
	data_type emailFwdCreate		(data_cref a);
	data_type emailFwdUpdate		(data_cref a);
	data_type emailFwdDelete		(data_cref a);
	data_type emailFwdTransfer		(data_cref a);

	data_type hostInfo			(data_cref a);
	data_type hostCheck			(data_cref a);
	data_type hostCreate			(data_cref a);
	data_type hostUpdate			(data_cref a);
	data_type hostDelete			(data_cref a);

	data_type contactCheck			(data_cref a);
	data_type contactCreate			(data_cref a);
	data_type contactInfo			(data_cref a);
	data_type contactTransfer		(data_cref a);
	data_type contactUpdate			(data_cref a);
	data_type contactDelete			(data_cref a);

// SMART COMMANDS 
	data_type pollOne			(data_cref a);
	data_type pollAll			(data_cref a);

	data_type domainAllowUpdate		(data_cref a) { return domainUpdate(a+allowUpdate()); };
	data_type domainProhibitUpdate		(data_cref a) { return domainUpdate(a+prohibitUpdate()); };
	data_type domainMassCheck		(data_cref a);
	data_type domainSmartCheck		(data_cref a);
	data_type domainSmartUpdate		(data_cref a,data_cref info);
	data_type domainSmartUpdate		(data_cref a) { return domainSmartUpdate(a,domainInfo(a)); };
	data_type domainSmartDelete		(data_cref a,data_cref info);
	data_type domainSmartDelete		(data_cref a);
	data_type domainSmartLock		(data_cref a,data_cref info);
	data_type domainSmartLock		(data_cref a) { return domainSmartLock(a,domainInfo(a)); };
	data_type domainSmartUnlock		(data_cref a,data_cref info);
	data_type domainSmartUnlock		(data_cref a) { return domainSmartUnlock(a,domainInfo(a)); };
	data_type domainSmartHold		(data_cref a,data_cref info);
	data_type domainSmartHold		(data_cref a) { return domainSmartHold(a,domainInfo(a)); };
	data_type domainSmartUnhold		(data_cref a,data_cref info);
	data_type domainSmartUnhold		(data_cref a) { return domainSmartUnhold(a,domainInfo(a)); };
	data_type domainSmartRenew		(data_cref a);

	data_type emailFwdSmartRenew		(data_cref a);

	data_type hostAllowUpdate		(data_cref a) { return hostUpdate(a+allowUpdate()); };
	data_type hostProhibitUpdate		(data_cref a) { return hostUpdate(a+prohibitUpdate()); };
	data_type hostRename			(line_cref oldn,line_cref newn);
	data_type hostSmartRename		(line_cref oldn,line_cref pfix=".public-ns.com");
	data_type hostSmartCheck		(data_cref a);
	data_type hostSmartUpdate		(data_cref a,data_cref info);
	data_type hostSmartUpdate		(data_cref a) { return hostSmartUpdate(a,hostInfo(a)); };
	data_type hostSmartDelete		(data_cref a);
	data_type hostSmartSet			(data_cref a,data_type info);
	data_type hostSmartSet			(data_cref a) { return hostSmartSet(a,hostInfo(a)); };

	data_type contactAllowUpdate		(data_cref a) { return contactUpdate(a+allowUpdate()); };
	data_type contactProhibitUpdate		(data_cref a) { return contactUpdate(a+prohibitUpdate()); };
	data_type contactSmartCheck		(data_cref a);
	data_type contactSmartCreate		(data_cref a) { return contactCreate(a.has("password") ? a : data_type(a,"password",genPass())); };
	data_type contactSmartUpdate		(data_cref a,data_cref info);
	data_type contactSmartUpdate		(data_cref a) { return contactSmartUpdate(a,contactInfo(a)); };
	data_type contactSmartSet		(data_cref a,data_type info);
	data_type contactSmartSet		(data_cref a) { return contactSmartSet(a,contactInfo(a)); };

// EPP auxiliary functions
public:
	static	bool_type			isDomainUpdatable		(data_cref statuses);
	static	bool_type			isResponseOk			(data_cref response);
		void_type			pollAck				(data_cref id);

// RTK auxiliary functions
protected:
    epp_extension_ref_seq_ref   getExtensions       (data_cref a);
    epp_Extension_ref           getExtension        (data_cref a,line_cref e = "");
    static  line_type           getExt              (data_cref a);
    static  line_type           getExt              (line_cref a);

            epp_Command *       newCommand          (const epp_extension_ref_seq_ref &exts,line_cref trID)  { return new epp_Command(exts,epp_trid(trID)); };
            epp_Command *       newCommand          (const epp_Extension_ref &ext,line_cref trID)           { return new epp_Command(NULL,ext,epp_trid(trID)); };
            epp_Command *       newCommand          (data_cref a,line_cref op)                              { return newCommand(getExtension(a),trID(a.getLine("trID"),op)); };

            line_type           trID                (line_cref id,line_cref op) { return id.size() ? id : genTrID(op); };
            line_type           genTrID             (line_cref op) { return serialNo+'-'+size2line(batchNo)+'-'+size2line(commandNo++)+'-'+op; };
            line_type           genPass             () { return "Jiwynn-Op8"; };

            data_type           safeProcessAction   (epp_Action_ref request);

	static	data_type			errorResult			(int code,line_cref msg) { return data_type("result_code",code,"result_msg",msg); };

	static	epp_string_seq *		newStringSeq			(data_cref a);
	static	epp_AuthInfo *			newAuthInfo			(data_cref a);
	static	epp_PollOpType *		newPollOpType			(line_cref t);
	static	epp_DomainHostsType *		newDomainHostsType		(line_cref t);
	static	line_type			statusType			(data_cref a) { return a.first("type").toLine(); };
	static	bool_type			isClientStatus			(line_cref s) { return s.substr(0,6)=="client"; };
	static	bool_type			isClientStatus			(data_cref a) { return isClientStatus(statusType(a)); };
	static	data_type			checkClientStatuses		(data_cref a);

	static	void_type			addDomainContacts		(epp_domain_contact_seq *seq,line_cref type,data_cref ids);
	static	epp_domain_contact_seq *	newDomainContactSeq		(data_cref a);
	static	epp_DomainStatus		DomainStatus			(data_cref a);
	static	epp_domain_status_seq *		newDomainStatusSeq		(data_cref a);

	static	void_type			addEmailFwdContacts		(epp_emailFwd_contact_seq *seq,line_cref type,data_cref ids);
	static	epp_emailFwd_contact_seq *	newEmailFwdContactSeq		(data_cref a);
	static	epp_EmailFwdStatus		EmailFwdStatus			(data_cref a);
	static	epp_emailFwd_status_seq *	newEmailFwdStatusSeq		(data_cref a);

	static	epp_ContactStatus		ContactStatus			(data_cref a);
	static	epp_contact_status_seq *	newContactStatusSeq		(data_cref a);
	static	epp_HostStatus			HostStatus			(data_cref a);
	static	epp_host_status_seq *		newHostStatusSeq		(data_cref a);
	static	epp_host_address_seq *		newHostAddressSeq		(data_cref a);
	static	bool_type			hasContactAddress		(data_cref a) { return a.hasAny("street1","street2","street3","city","province","postal_code","country"); };
	static	bool_type			hasContactAddressName		(data_cref a) { return a.hasAny("name","organization") || hasContactAddress(a); };
	static	bool_type			hasContactData			(data_cref a) { return a.hasAny("voice_phone","fax_phone","email","password") || hasContactAddressName(a); };
	static	epp_ContactAddress *		newContactAddress		(data_cref a);
	static	epp_ContactNameAddress		ContactNameAddress		(data_cref a);
	static	epp_ContactNameAddress_seq *	newContactNameAddressSeq	(data_cref a);
	static	epp_ContactPhone *		newContactPhone			(data_cref p,data_cref e);

/// Extensions
    static  epp_Extension_ref       domainTrademark         (data_cref a);
    static  epp_Extension_ref       domainIDNScript         (data_cref a);
    static  epp_Extension_ref       domainIDNLang           (data_cref a);
    static  epp_Extension_ref       domainSecDNS            (data_cref a);
    static  SecDNSDsData_ref        SecDNS                  (data_cref a);

    static  data_type               readDomainTrnData       (const epp_PollResData_ref &t);
    static  data_type               readContactTrnData      (const epp_PollResData_ref &t);
    static  data_type               readLowBalancePollData  (const epp_PollResData_ref &t);
    static  data_type               readRGPPollData         (const epp_PollResData_ref &t); // Redemption Grace Period
    static  data_type               readMessageQueueData    (const epp_MessageQueue_ref &r);
    static  data_type               readTransIDData         (const epp_TransID_ref &r);
    static  data_type               readResultsData         (const epp_result_seq_ref &r);
    static  data_type               readResponseData        (const epp_Response_ref &r);

// Other auxiliary functions
protected:
	static	data_type			diffOldNew2AddRem		(data_cref old_k,data_cref new_k);
	static	data_type			allowUpdate			() { return data_type("remove",data_type("statuses","clientUpdateProhibited")); };
	static	data_type			prohibitUpdate			() { return data_type("add",   data_type("statuses","clientUpdateProhibited")); };

// PROPERTIES
private:
// parameters
	line_type				host;
	size_type				port;
	line_type				certificate;
	line_type				cacertfile;
	line_type				cacertdir;
	line_type				username;
	line_type				password;
	line_type				serialNo;
	size_type				batchNo;
	size_type				commandNo;
// resources
	epp_Session				session;
	epp_TransportSSL			*transport;
	Hash<epp_Extension_ref>			extensions;
}; // class EPP

}; // namespace re

#endif//__RE_EPP_H__
