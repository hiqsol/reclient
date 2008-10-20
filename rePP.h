// (c) Andrii Vasyliev
// rePP - Registrar EPP interface

#ifndef __RE_PP_H__
#define __RE_PP_H__

#include "rebase/reTypes/reValue.h"
#include "epp-rtk-cpp/epp_Session.h"
#include "epp-rtk-cpp/data/epp_Command.h"
#include "epp-rtk-cpp/data/epp_AuthInfo.h"
#include "epp-rtk-cpp/data/epp_domaindata.h"
#include "epp-rtk-cpp/data/epp_hostdata.h"
#include "epp-rtk-cpp/data/epp_PollResData.h"
#include "epp-rtk-cpp/data/epp_Response.h"
#include "epp-rtk-cpp/transport/transports.h"
#include "comnetaddon/epp_NamestoreExt.h"

using namespace epptransport;
using namespace eppobject::epp;
using namespace eppobject::domain;
using namespace eppobject::host;
using namespace eppobject::namestoreExt;
using std::string;

class rePP {
public:
	rePP () {};
	rePP (const string &h,unsigned n,const string &c,const string &e = "dotCOM")
		: host(h),port(n),certificate(c),extData(e),ext(new epp_NamestoreExt()),counter(1)
	{
		transport = new epp_TransportSSL(certificate,"","");
		transport->setServerName(host);
		transport->setServerPort(port);
		refcnt_ptr<epp_TransportSSL> tref(dynamic_cast<epp_TransportSSL*>(transport));
		session.setTransport(tref);
		ext->setRequestData(extData);
	};

	void setCredentials (const string &u,const string &p) { username = u;password = p; };

// EPP COMMANDS
public:
	reValue	poll		(const reValue &args=reValue::Null);
	reValue	hello		(const reValue &args=reValue::Null);
	reValue login		(const reValue &args=reValue::Null);
	reValue logout		(const reValue &args=reValue::Null);
	reValue domainInfo	(const reValue &args);
	reValue domainSync	(const reValue &args);
	reValue domainCheck	(const reValue &args);
	reValue domainRenew	(const reValue &args);
	reValue domainCreate	(const reValue &args);
	reValue domainUpdate	(const reValue &args);
	reValue domainDelete	(const reValue &args);
	reValue domainTransfer	(const reValue &args);
	reValue hostInfo	(const reValue &args);
	reValue hostCheck	(const reValue &args);
	reValue hostCreate	(const reValue &args);
	reValue hostUpdate	(const reValue &args);
	reValue hostDelete	(const reValue &args);
	reValue contactCheck	(const reValue &args);

// Auxilary functions
protected:
	epp_Command			*newCommand		(const string &trID) { return new epp_Command(NULL,ext,epp_trid(trID)); };
	epp_Command			*newCommand		(const string &id,const string &op) { return newCommand(trID(id,op)); };
	string				trID			(const string &id,const string &op) { return id.size() ? id : genTrID(op); };
	string				genTrID			(const string &op) { return op+"-"+u2line(counter++); };
	static epp_string_seq		*newStringSeq		(const reValue &a);
	static epp_AuthInfo		*newAuthInfo		(const reValue &a);
	static epp_PollOpType		*newPollOpType		(const reLine &t);
	static epp_DomainHostsType	*newDomainHostsType	(const reLine &t);
	static epp_DomainStatus		DomainStatus		(const reValue &a);
	static epp_domain_status_seq	*newDomainStatusSeq	(const reValue &a);
	static epp_HostStatus		HostStatus		(const reValue &a);
	static epp_host_status_seq	*newHostStatusSeq	(const reValue &a);
	static epp_host_address_seq	*newHostAddressSeq	(const reValue &a);

	static reValue			getDomainTrnData	(const epp_PollResData_ref &t);
	static reValue			getResponseData		(const epp_Response_ref &r);
	static reValue			getResults		(const epp_result_seq_ref &r);

// PROPERTIES
private:
// parameters
	string			host;
	unsigned		port;
	string			certificate;
	string			username;
	string			password;
// resources
	epp_TransportSSL	*transport;
	epp_Session		session;
	epp_NamestoreExtData	extData;
	epp_NamestoreExt_ref	ext;
	unsigned		counter;
};

#endif//__RE_PP_H__
