// (c) Andrii Vasyliev
// rePP - Registrar EPP interface

#ifndef __RE_PP_H__
#define __RE_PP_H__

#include "rebase/reTypes/reHash.h"
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

class rePP {
public:
	rePP () {};
	rePP (const reLine &h,unsigned p,const reLine &c,const reLine &f,const reLine &d)
		: host(h),port(p),certificate(c),cacertfile(f),cacertdir(d),serialNo("0"),batchNo(1),commandNo(1)
	{
		transport = new epp_TransportSSL(certificate,cacertfile,cacertdir);
		//printf("certificate:%s cacertfile:%s cacertdir:%s\n",certificate.c_str(),cacertfile.c_str(),cacertdir.c_str());
		transport->setServerName(host);
		transport->setServerPort(port);
		refcnt_ptr<epp_TransportSSL> tref(dynamic_cast<epp_TransportSSL*>(transport));
		session.setTransport(tref);
	};

	void setCredentials (const reLine &u,const reLine &p) { username = u;password = p; };
	void setNamestoreExtension (const reLine &ext,const reLine &data);
	epp_Extension_ref getExtension (const reLine &ext) { return extensions.has(ext) ? extensions.let(ext) : NULL; };

	void setSerialNo (const reLine &s) { serialNo = s; };
	unsigned incBatchNo () { return ++batchNo; };

// NATIVE EPP COMMANDS
public:
	reValue	poll			(const reValue &a=reValue::Null);
	reValue	hello			(const reValue &a=reValue::Null);
	reValue login			(const reValue &a=reValue::Null);
	reValue logout			(const reValue &a=reValue::Null);
	reValue domainInfo		(const reValue &a);
	reValue domainSync		(const reValue &a);
	reValue domainCheck		(const reValue &a);
	reValue domainRenew		(const reValue &a);
	reValue domainCreate		(const reValue &a);
	reValue domainUpdate		(const reValue &a);
	reValue domainDelete		(const reValue &a);
	reValue domainTransfer		(const reValue &a);
	reValue hostInfo		(const reValue &a);
	reValue hostCheck		(const reValue &a);
	reValue hostCreate		(const reValue &a);
	reValue hostUpdate		(const reValue &a);
	reValue hostDelete		(const reValue &a);
	reValue contactCheck		(const reValue &a);

// SMART COMMANDS
	reValue pollOne			(const reValue &a);
	reValue pollAll			(const reValue &a);
	reValue domainAllowUpdate	(const reValue &a);
	reValue domainProhibitUpdate	(const reValue &a);
	reValue domainSmartCheck	(const reValue &a);
	reValue domainSmartUpdate	(const reValue &a,const reValue &info);
	reValue domainSmartUpdate	(const reValue &a) { return domainSmartUpdate(a,domainInfo(a)); };
	reValue domainSmartLock		(const reValue &a,const reValue &info);
	reValue domainSmartLock		(const reValue &a) { return domainSmartLock(a,domainInfo(a)); };
	reValue domainSmartUnlock	(const reValue &a,const reValue &info);
	reValue domainSmartUnlock	(const reValue &a) { return domainSmartUnlock(a,domainInfo(a)); };
	reValue domainSmartHold		(const reValue &a,const reValue &info);
	reValue domainSmartHold		(const reValue &a) { return domainSmartHold(a,domainInfo(a)); };
	reValue domainSmartUnhold	(const reValue &a,const reValue &info);
	reValue domainSmartUnhold	(const reValue &a) { return domainSmartUnhold(a,domainInfo(a)); };
	reValue hostAllowUpdate		(const reValue &a);
	reValue hostProhibitUpdate	(const reValue &a);
	reValue hostSmartCheck		(const reValue &a);
	reValue hostSmartUpdate		(const reValue &a,const reValue &info);
	reValue hostSmartUpdate		(const reValue &a) { return hostSmartUpdate(a,hostInfo(a)); };
	reValue hostSmartSet		(const reValue &a,reValue info);
	reValue hostSmartSet		(const reValue &a) { return hostSmartSet(a,hostInfo(a)); };

// EPP auxiliary functions
public:
	static	bool isDomainUpdatable	(const reValue &statuses);
	static	bool isResponseOk	(const reValue &response);
		void pollAck		(const reValue &id);

// RTK auxiliary functions
protected:
	epp_Command	*newCommand	(const reLine &ext,const reLine &trID) { return new epp_Command(NULL,getExtension(ext),epp_trid(trID)); };
	epp_Command	*newCommand	(const reLine &ext,const reLine &id,const reLine &op) { return newCommand(ext,trID(id,op)); };
	epp_Command	*newCommand	(const reValue &a,const reLine &op) { return newCommand(getExt(a),trID(a.gl("trID"),op)); };
	reLine		trID		(const reLine &id,const reLine &op) { return id.size() ? id : genTrID(op); };
	reLine		genTrID		(const reLine &op) { return serialNo+'-'+u2line(batchNo)+'-'+u2line(commandNo++)+'-'+op; };

	static reLine	getExt		(const reValue &a);
	static reLine	getExt		(const reLine &a);
	reValue safeProcessAction	(epp_Action_ref request);

	static reValue	errorResult	(int code,const reLine &msg) { return reValue("result_code",code,"result_msg",msg); };

	static epp_string_seq		*newStringSeq		(const reValue &a);
	static epp_AuthInfo		*newAuthInfo		(const reValue &a);
	static epp_PollOpType		*newPollOpType		(const reLine &t);
	static epp_DomainHostsType	*newDomainHostsType	(const reLine &t);
	static epp_DomainStatus		DomainStatus		(const reValue &a);
	static epp_domain_status_seq	*newDomainStatusSeq	(const reValue &a);
	static epp_HostStatus		HostStatus		(const reValue &a);
	static epp_host_status_seq	*newHostStatusSeq	(const reValue &a);
	static epp_host_address_seq	*newHostAddressSeq	(const reValue &a);

	static reValue			readDomainTrnData	(const epp_PollResData_ref &t);
	static reValue			readMessageQueueData	(const epp_MessageQueue_ref &r);
	static reValue			readTransIDData		(const epp_TransID_ref &r);
	static reValue			readResultsData		(const epp_result_seq_ref &r);
	static reValue			readResponseData	(const epp_Response_ref &r);

// PROPERTIES
private:
// parameters
	reLine				host;
	unsigned			port;
	reLine				certificate;
	reLine				cacertfile;
	reLine				cacertdir;
	reLine				username;
	reLine				password;
	reLine				serialNo;
	unsigned			batchNo;
	unsigned			commandNo;
// resources
	epp_Session			session;
	epp_TransportSSL		*transport;
	reHash<epp_Extension_ref>	extensions;
};

#endif//__RE_PP_H__
