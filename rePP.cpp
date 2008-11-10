// (c) Andrii Vasyliev
// rePP - Registrar EPP

#include "rePP.h"
#include "epp-rtk-cpp/epp_Action.h"
#include "epp-rtk-cpp/epp_Poll.h"
#include "epp-rtk-cpp/epp_Hello.h"
#include "epp-rtk-cpp/epp_Login.h"
#include "epp-rtk-cpp/epp_Logout.h"
#include "epp-rtk-cpp/epp_DomainInfo.h"
#include "epp-rtk-cpp/epp_DomainCheck.h"
#include "epp-rtk-cpp/epp_DomainRenew.h"
#include "epp-rtk-cpp/epp_DomainCreate.h"
#include "epp-rtk-cpp/epp_DomainUpdate.h"
#include "epp-rtk-cpp/epp_DomainDelete.h"
#include "epp-rtk-cpp/epp_DomainTransfer.h"
#include "epp-rtk-cpp/epp_HostInfo.h"
#include "epp-rtk-cpp/epp_HostCheck.h"
#include "epp-rtk-cpp/epp_HostCreate.h"
#include "epp-rtk-cpp/epp_HostUpdate.h"
#include "epp-rtk-cpp/epp_HostDelete.h"
#include "epp-rtk-cpp/epp_ContactCheck.h"
#include "epp-rtk-cpp/data/epp_Greeting.h"
#include "epp-rtk-cpp/data/epp_domainXMLbase.h"
#include "epp-rtk-cpp/data/epp_hostXMLbase.h"
#include "comnetaddon/epp_DomainSync.h"

using namespace eppobject::domain;
using namespace eppobject::sync;
using namespace eppobject::host;
using namespace eppobject::contact;

reValue rePP::poll (const reValue &a) {
	// preparing request
	epp_PollReq_ref request(new epp_PollReq());
	request->m_cmd.ref(new epp_Command(NULL,NULL,epp_trid(trID(a.gl("trID"),"PO"))));
	request->m_op.ref(new epp_PollOpType(a.gl("op")=="ack" ? ACK : REQ));
	if (a.has("id")) request->m_msgID.ref(new epp_string(a.gl("id")));
	// performing command
	epp_Poll_ref command(new epp_Poll());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_PollRsp_ref r = command->getResponseData();
	return getDomainTrnData(r->m_res_data)+getResponseData(r->m_rsp);
};

reValue rePP::hello (const reValue &a) {
	// performing command
	epp_Hello_ref command(new epp_Hello());
	session.processAction(command);
	// getting response
	epp_Greeting_ref r = command->getResponseData();
	return reValue::Null;
};

reValue rePP::login (const reValue &a) {
	if (a.has("username")) setCredentials(a.gl("username"),a.gl("password"));
	// preparing request
	epp_LoginReq_ref request(new epp_LoginReq());
	request->m_cmd.ref(new epp_Command(NULL,NULL,epp_trid(trID(a.gl("trID"),"LI"))));
	request->m_options.ref(new epp_Options(epp_string("1.0"),epp_string("en")));
	request->m_client_id.ref(new epp_string(username));
	request->m_password.ref(new epp_string(password));
	if (a.get("newPassword").toBool()) request->m_new_password.ref(new epp_string(a.gl("newPassword")));
	// filling services info
	request->m_services.ref(new epp_objuri_seq());
	request->m_services->push_back("urn:ietf:params:xml:ns:contact-1.0"); // COM,NET don't support contacts
	request->m_services->push_back("urn:ietf:params:xml:ns:domain-1.0");
	request->m_services->push_back("urn:ietf:params:xml:ns:host-1.0");
	// getting greeting
	session.connectAndGetGreeting();
	// performing command
	epp_Login_ref command(new epp_Login());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_LoginRsp_ref response = command->getResponseData();
	return reValue();
};

reValue rePP::logout (const reValue &a) {
	// preparing request
	epp_LogoutReq_ref request(new epp_LogoutReq());
	request->m_client_trid.ref(new epp_trid(trID(a.gl("trID"),"LO")));
	// performing command
	epp_Logout_ref command(new epp_Logout());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_LogoutRsp_ref response = command->getResponseData();
	return reValue();
};

reValue rePP::domainInfo (const reValue &a) {
	// preparing request
	epp_DomainInfoReq_ref request(new epp_DomainInfoReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DI"));
	request->m_name.ref(new epp_string(a.gl("name")));
	if (a.has("type")) request->m_hosts_type.ref(newDomainHostsType(a.gl("type")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainInfo_ref command(new epp_DomainInfo());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainInfoRsp_ref r = command->getResponseData();
	reValue res = getResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_roid!=NULL) res["roid"] = *r->m_roid;
	if (r->m_status!=NULL) {
		reValue &statuses = res["statuses"];
		for (epp_domain_status_seq::iterator i = r->m_status->begin();i!=r->m_status->end();i++) {
			/*
			reValue s;
			if (i->m_type!=NULL) s["type"] = returnStatusType(*i->m_type);
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_value!=NULL) s["value"] = *i->m_value;
			*/
			if (i->m_type!=NULL) statuses.push(returnStatusType(*i->m_type));
		};
	};
	if (r->m_registrant!=NULL) res["registrant"] = *r->m_registrant;
	if (r->m_contacts!=NULL) {
		reValue &contacts = res["contacts"];
		for (epp_domain_contact_seq::iterator i = r->m_contacts->begin();i!=r->m_contacts->end();i++) {
			reValue c;
			if (i->m_type!=NULL) c["type"] = returnContactType(*i->m_type);
			if (i->m_id!=NULL) c["id"] = *i->m_id;
			contacts.push(c);
		};
	};
	if (r->m_name_servers!=NULL) {
		reValue &nses = res["nameservers"];
		for (epp_string_seq::iterator i = r->m_name_servers->begin();i!=r->m_name_servers->end();i++) {
			nses.push(*i);
		};
	};
	if (r->m_hosts!=NULL) {
		reValue &hosts = res["hosts"];
		for (epp_string_seq::iterator i = r->m_hosts->begin();i!=r->m_hosts->end();i++) {
			hosts.push(*i);
		};
	};
	if (r->m_client_id!=NULL) res["client_id"] = *r->m_client_id;
	if (r->m_created_by!=NULL) res["created_by"] = *r->m_created_by;
	if (r->m_created_date!=NULL) res["created_date"] = *r->m_created_date;
	if (r->m_updated_by!=NULL) res["updated_by"] = *r->m_updated_by;
	if (r->m_updated_date!=NULL) res["updated_date"] = *r->m_updated_date;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	if (r->m_transfer_date!=NULL) res["transfer_date"] = *r->m_transfer_date;
	if (r->m_auth_info!=NULL && r->m_auth_info->m_value!=NULL) res["password"] = *r->m_auth_info->m_value;
	return res;
};

reValue rePP::domainSync (const reValue &a) {
	// preparing request
	epp_Command_ref cmd(newCommand(a.gl("trID"),"DS"));
	epp_string_ref name(new epp_string(a.gl("name")));
	epp_SyncData_ref data(new epp_SyncData(epp_SyncMonthType(a.gi4("month")),epp_short(a.gi4("day"))));
	epp_DomainSyncReq_ref request(new epp_DomainSyncReq(cmd,name,data));
	// performing command
	epp_DomainSync_ref command(new epp_DomainSync());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainUpdateRsp_ref r = command->getResponseData();
	reValue res;
	return res;
};

reValue rePP::domainCheck (const reValue &a) {
	// preparing request
	epp_DomainCheckReq_ref request(new epp_DomainCheckReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DH"));
	request->m_names.ref(newStringSeq(a.get("names").csplit()));
	// performing command
	epp_DomainCheck_ref command(new epp_DomainCheck());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainCheckRsp_ref r = command->getResponseData();
	reValue res;
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			reValue s;
			if (i->m_avail!=NULL) s["avail"] = *i->m_avail;
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_reason!=NULL) s["reason"] = *i->m_reason;
			if (i->m_value!=NULL) res[*i->m_value] = s; else res.push(s);
		};
	};
	return res;
};

reValue rePP::domainRenew (const reValue &a) {
	// preparing request
	epp_DomainRenewReq_ref request(new epp_DomainRenewReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DR"));
	request->m_name.ref(new epp_string(a.gl("name")));
	request->m_current_expiration_date.ref(new epp_date(a.gl("expires")));
	request->m_period.ref(new epp_DomainPeriod(YEAR,a.get("period").toInt4(1)));
	// performing command
	epp_DomainRenew_ref command(new epp_DomainRenew());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainRenewRsp_ref r = command->getResponseData();
	reValue res;
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

reValue rePP::domainCreate (const reValue &a) {
	// preparing request
	epp_DomainCreateReq_ref request(new epp_DomainCreateReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DC"));
	request->m_name.ref(new epp_string(a.gl("name")));
	request->m_period.ref(new epp_DomainPeriod(YEAR,a.get("period").toInt4(1)));
	if (a.has("nameservers")) request->m_name_servers.ref(newStringSeq(a.get("nameservers").csplit()));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainCreate_ref command(new epp_DomainCreate());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainCreateRsp_ref r = command->getResponseData();
	reValue res;
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_creation_date!=NULL) res["created_date"] = *r->m_creation_date;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

reValue rePP::domainUpdate (const reValue &a) {
	// preparing request
	epp_DomainUpdateReq_ref request(new epp_DomainUpdateReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DU"));
	request->m_name.ref(new epp_string(a.gl("name")));
	if (a.has("add")) {
		const reValue &add = a.get("add");
		request->m_add.ref(new epp_DomainUpdateAddRemove());
		if (add.has("statuses")) request->m_add->m_status.ref(newDomainStatusSeq(add.get("statuses").csplit()));
		if (add.has("nameservers")) request->m_add->m_name_servers.ref(newStringSeq(add.get("nameservers").csplit()));
	};
	if (a.has("remove")) {
		const reValue &remove = a.get("remove");
		request->m_remove.ref(new epp_DomainUpdateAddRemove());
		if (remove.has("statuses")) request->m_remove->m_status.ref(newDomainStatusSeq(remove.get("statuses").csplit()));
		if (remove.has("nameservers")) request->m_remove->m_name_servers.ref(newStringSeq(remove.get("nameservers").csplit()));
	};
	if (a.has("change")) {
		const reValue &change = a.get("change");
		request->m_change.ref(new epp_DomainUpdateChange());
		if (change.has("password")) request->m_change->m_auth_info.ref(newAuthInfo(change));
	};
	// performing command
	epp_DomainUpdate_ref command(new epp_DomainUpdate());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainUpdateRsp_ref r = command->getResponseData();
	reValue res;
	return res;
};

reValue rePP::domainDelete (const reValue &a) {
	// preparing request
	epp_DomainDeleteReq_ref request(new epp_DomainDeleteReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DD"));
	request->m_name.ref(new epp_string(a.gl("name")));
	// performing command
	epp_DomainDelete_ref command(new epp_DomainDelete());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainDeleteRsp_ref r = command->getResponseData();
	reValue res;
	return res;
};

reValue rePP::domainTransfer (const reValue &a) {
	// preparing request
	epp_DomainTransferReq_ref request(new epp_DomainTransferReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"DT"));
	request->m_name.ref(new epp_string(a.gl("name")));
	if (a.has("period")) request->m_period.ref(new epp_DomainPeriod(YEAR,a.get("period").toInt4()));
	request->m_trans.ref(new epp_TransferRequest());
	request->m_trans->m_op.ref(new epp_TransferOpType(returnTransferType(a.gl("op"))));
	if (a.has("password")) request->m_trans->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainTransfer_ref command(new epp_DomainTransfer());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_DomainTransferRsp_ref r = command->getResponseData();
	return getDomainTrnData(r->m_trn_data);
};

reValue rePP::hostInfo (const reValue &a) {
	// preparing request
	epp_HostInfoReq_ref request(new epp_HostInfoReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"HI"));
	request->m_name.ref(new epp_string(a.gl("name")));
	// performing command
	epp_HostInfo_ref command(new epp_HostInfo());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_HostInfoRsp_ref r = command->getResponseData();
	reValue res;
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_roid!=NULL) res["roid"] = *r->m_roid;
	if (r->m_status!=NULL) {
		reValue &statuses = res["statuses"];
		for (epp_host_status_seq::iterator i = r->m_status->begin();i!=r->m_status->end();i++) {
			/*
			reValue s;
			if (i->m_type!=NULL) s["type"] = returnStatusType(*i->m_type);
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_value!=NULL) s["value"] = *i->m_value;
			*/
			if (i->m_type!=NULL) statuses.push(returnStatusType(*i->m_type));
		};
	};
	if (r->m_addresses!=NULL) {
		reValue &ips = res["ips"];
		for (epp_host_address_seq::iterator i=r->m_addresses->begin();i!=r->m_addresses->end();i++) {
			if (i->m_ip!=NULL) ips.push(*i->m_ip);
		};
	};
	if (r->m_client_id!=NULL) res["client_id"] = *r->m_client_id;
	if (r->m_created_by!=NULL) res["created_by"] = *r->m_created_by;
	if (r->m_created_date!=NULL) res["created_date"] = *r->m_created_date;
	if (r->m_updated_by!=NULL) res["updated_by"] = *r->m_updated_by;
	if (r->m_updated_date!=NULL) res["updated_date"] = *r->m_updated_date;
	if (r->m_transfer_date!=NULL) res["transfer_date"] = *r->m_transfer_date;
	return res;
};

reValue rePP::hostCheck (const reValue &a) {
	// preparing request
	epp_HostCheckReq_ref request(new epp_HostCheckReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"HH"));
	request->m_names.ref(newStringSeq(a.get("names").csplit()));
	// performing command
	epp_HostCheck_ref command(new epp_HostCheck());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_HostCheckRsp_ref r = command->getResponseData();
	reValue res;
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			reValue s;
			if (i->m_avail!=NULL) s["avail"] = *i->m_avail;
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_reason!=NULL) s["reason"] = *i->m_reason;
			if (i->m_value!=NULL) res[*i->m_value] = s; else res.push(s);
		};
	};
	return res;
};

reValue rePP::hostCreate (const reValue &a) {
	// preparing request
	epp_HostCreateReq_ref request(new epp_HostCreateReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"HC"));
	request->m_name.ref(new epp_string(a.gl("name")));
	request->m_addresses.ref(newHostAddressSeq(a.get("ips").csplit()));
	// performing command
	epp_HostCreate_ref command(new epp_HostCreate());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_HostCreateRsp_ref r = command->getResponseData();
	reValue res;
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_creation_date!=NULL) res["creation_date"] = *r->m_creation_date;
	return res;
};

reValue rePP::hostUpdate (const reValue &a) {
	// preparing request
	epp_HostUpdateReq_ref request(new epp_HostUpdateReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"HU"));
	request->m_name.ref(new epp_string(a.gl("name")));
	if (a.has("add")) {
		const reValue &add = a.get("add");
		request->m_add.ref(new epp_HostUpdateAddRemove());
		if (add.has("statuses")) request->m_add->m_status.ref(newHostStatusSeq(add.get("statuses").csplit()));
		if (add.has("ips")) request->m_add->m_addresses.ref(newHostAddressSeq(add.get("ips").csplit()));
	};
	if (a.has("remove")) {
		const reValue &remove = a.get("remove");
		request->m_remove.ref(new epp_HostUpdateAddRemove());
		if (remove.has("statuses")) request->m_remove->m_status.ref(newHostStatusSeq(remove.get("statuses").csplit()));
		if (remove.has("ips")) request->m_remove->m_addresses.ref(newHostAddressSeq(remove.get("ips").csplit()));
	};
	if (a.has("change")) {
		const reValue &change = a.get("change");
		request->m_change.ref(new epp_HostUpdateChange());
		if (change.has("name")) request->m_change->m_name.ref(new epp_string(change.gl("name")));
	};
	// performing command
	epp_HostUpdate_ref command(new epp_HostUpdate());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_HostUpdateRsp_ref r = command->getResponseData();
	reValue res;
	return res;
};

reValue rePP::hostDelete (const reValue &a) {
	// preparing request
	epp_HostDeleteReq_ref request(new epp_HostDeleteReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"HD"));
	request->m_name.ref(new epp_string(a.gl("name")));
	// performing command
	epp_HostDelete_ref command(new epp_HostDelete());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_HostDeleteRsp_ref r = command->getResponseData();
	return reValue::Null;
};

reValue rePP::contactCheck (const reValue &a) {
	// preparing request
	epp_ContactCheckReq_ref request(new epp_ContactCheckReq());
	request->m_cmd.ref(newCommand(a.gl("trID"),"CH"));
	request->m_ids.ref(newStringSeq(a.get("ids").csplit()));
	// performing command
	epp_ContactCheck_ref command(new epp_ContactCheck());
	command->setRequestData(*request);
	session.processAction(command);
	// getting response
	epp_ContactCheckRsp_ref r = command->getResponseData();
	return reValue();
};

epp_string_seq *rePP::newStringSeq (const reValue &a) {
	epp_string_seq *res = new epp_string_seq();
	for (reValue::size_type i=0,n=a.size();i<n;i++) res->push_back(a.gl(i));
	return res;
};

epp_AuthInfo *rePP::newAuthInfo (const reValue &a) {
	epp_AuthInfo *res = new epp_AuthInfo();
	res->m_type.ref(new epp_AuthInfoType(PW));
	res->m_value.ref(new epp_string(a.gl("password")));
	return res;
};

epp_PollOpType *rePP::newPollOpType (const reLine &t) {
	epp_PollOpType type = REQ;
	if ("ack"==t) type = ACK;
	return new epp_PollOpType(type);
};

epp_DomainHostsType *rePP::newDomainHostsType (const reLine &t) {
	epp_DomainHostsType type = eppobject::domain::ALL;
	if ("none"==t) type = eppobject::domain::NONE;
	else if ("del"==t) type = eppobject::domain::DEL;
	else if ("sub"==t) type = eppobject::domain::SUB;
	return new epp_DomainHostsType(type);
};

epp_DomainStatus rePP::DomainStatus (const reValue &a) {
	epp_DomainStatus res;
	res.m_type.ref(new epp_DomainStatusType(eppobject::domain::returnStatusEnumType(a.first("type").toLine())));
	if (a.has("value")) res.m_value.ref(new epp_string(a.gl("value")));
	if (a.has("lang")) res.m_lang.ref(new epp_string(a.gl("lang")));
	return res;
};

epp_domain_status_seq *rePP::newDomainStatusSeq (const reValue &a) {
	epp_domain_status_seq *res = new epp_domain_status_seq();
	for (reValue::size_type i=0,n=a.size();i<n;i++) res->push_back(DomainStatus(a.get(i)));
	return res;
};

epp_HostStatus rePP::HostStatus (const reValue &a) {
	epp_HostStatus res;
	res.m_type.ref(new epp_HostStatusType(eppobject::host::returnStatusEnumType(a.first("type").toLine())));
	if (a.has("value")) res.m_value.ref(new epp_string(a.gl("value")));
	if (a.has("lang")) res.m_lang.ref(new epp_string(a.gl("lang")));
	return res;
};

epp_host_status_seq *rePP::newHostStatusSeq (const reValue &a) {
	epp_host_status_seq *res = new epp_host_status_seq();
	for (reValue::size_type i=0,n=a.size();i<n;i++) res->push_back(HostStatus(a.get(i)));
	return res;
};

epp_host_address_seq *rePP::newHostAddressSeq (const reValue &a) {
	epp_host_address_seq *res = new epp_host_address_seq();
	for (reValue::size_type i=0,n=a.size();i<n;i++) {
		 reLine ip = a.gl(i);
		epp_HostAddress address(ip.find(".")==reLine::npos ? IPV6 : IPV4,epp_string(ip));
		res->push_back(address);
	};
	return res;
};

reValue rePP::getDomainTrnData (const epp_PollResData_ref &p) {
	if (p==NULL) return reValue::Null;
	if (p->getType()!="domain:trnData") return reValue::Null;
	const epp_DomainTrnData_ref &t = p;
	reValue res;
	if (t->m_name!=NULL) res["name"] = *t->m_name;
	if (t->m_transfer_status!=NULL) res["transfer_status"] = returnTransferStatusType(*t->m_transfer_status);
	if (t->m_request_client_id!=NULL) res["request_client_id"] = *t->m_request_client_id;
	if (t->m_request_date!=NULL) res["request_date"] = *t->m_request_date;
	if (t->m_action_client_id!=NULL) res["action_client_id"] = *t->m_action_client_id;
	if (t->m_action_date!=NULL) res["action_date"] = *t->m_action_date;
	if (t->m_expiration_date!=NULL) res["expiration_date"] = *t->m_expiration_date;
	return res;
};

reValue rePP::getResponseData (const epp_Response_ref &r) {
	if (r==NULL) return reValue::Null;
	reValue res = getResults(r->m_results);
	const epp_MessageQueue_ref &m = r->m_message_queue;
	if (m!=NULL) {
		if (m->m_count!=NULL) res["msgQ_count"] = (int32_t)*m->m_count;
		if (m->m_id!=NULL) res["msgQ_id"] = *m->m_id;
		if (m->m_msg!=NULL) res["msgQ_message"] = *m->m_msg->m_value;
		if (m->m_queue_date!=NULL) res["msgQ_date"] = *m->m_queue_date;
	};
	const epp_TransID_ref &t = r->m_trans_id;
	if (t!=NULL) {
		if (t->m_server_trid!=NULL) res["server_trid"] = *t->m_server_trid;
		if (t->m_client_trid!=NULL) res["client_trid"] = *t->m_client_trid;
	};
	return res;
};

reValue rePP::getResults (const epp_result_seq_ref &r) {
	if (r==NULL) return reValue::Null;
	reValue res;
	for (epp_result_seq::iterator i=r->begin();i!=r->end();i++) {
		if (i->m_msg!=NULL) {
			reValue &s = res.push(reValue("result_msg",*i->m_msg));
			if (i->m_code!=NULL) s["result_code"] = *i->m_code;
			if (i->m_lang!=NULL) s["result_lang"] = *i->m_lang;
		};
	};
	if (res.size()==1) res = res.first();
	return res;
};

