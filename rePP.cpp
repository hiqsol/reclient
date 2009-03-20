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

#include "epp-rtk-cpp/data/epp_Exception.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"

using namespace eppobject::domain;
using namespace eppobject::sync;
using namespace eppobject::host;
using namespace eppobject::contact;

reValue rePP::poll (const reValue &a) {
	// preparing request
	epp_PollReq_ref request(new epp_PollReq());
	request->m_cmd.ref(newCommand(a,"PO"));
	request->m_op.ref(new epp_PollOpType(a.gl("op")=="ack" ? ACK : REQ));
	if (a.has("id")) request->m_msgID.ref(new epp_string(a.gl("id")));
	// performing command
	epp_Poll_ref command(new epp_Poll());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_PollRsp_ref r = command->getResponseData();
	return readDomainTrnData(r->m_res_data)+readResponseData(r->m_rsp);
};

reValue rePP::hello (const reValue &a) {
	// performing command
	epp_Hello_ref command(new epp_Hello());
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_Greeting_ref r = command->getResponseData();
	return reValue::Null;
};

reValue rePP::login (const reValue &a) {
	if (a.has("username")) setCredentials(a.gl("username"),a.gl("password"));
	// preparing request
	epp_LoginReq_ref request(new epp_LoginReq());
	request->m_cmd.ref(newCommand(a,"LI"));
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
	// TODO read Greeting
	// performing command
	epp_Login_ref command(new epp_Login());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_LoginRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

reValue rePP::logout (const reValue &a) {
	// preparing request
	epp_LogoutReq_ref request(new epp_LogoutReq());
	request->m_client_trid.ref(new epp_trid(trID(a.gl("trID"),"LO")));
	// performing command
	epp_Logout_ref command(new epp_Logout());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_LogoutRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

reLine rePP::getExt (const reLine &a) {
	reLine::size_type pos = a.rfind('.');
	return pos==reLine::npos ? "" : a.substr(pos);
};

reLine rePP::getExt (const reValue &a) {
	if (a.has("ext")) return a.gl("ext");
	if (a.has("zone")) return a.gl("zone");
	if (a.has("name")) return getExt(a.gl("name"));
	if (a.has("names")) {
		reLine n = a.gl("names");
		return getExt(n.substr(0,n.find(',')));
	};
	return "";
};

reValue rePP::domainInfo (const reValue &a) {
	// preparing request
	epp_DomainInfoReq_ref request(new epp_DomainInfoReq());
	request->m_cmd.ref(newCommand(a,"DI"));
	request->m_name.ref(new epp_string(a.gl("name")));
	if (a.has("type")) request->m_hosts_type.ref(newDomainHostsType(a.gl("type")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainInfo_ref command(new epp_DomainInfo());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainInfoRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
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
			if (i->m_type!=NULL) statuses.set(returnStatusType(*i->m_type),reValue::Null);
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
			nses.set(*i,reValue::Null);
		};
	};
	if (r->m_hosts!=NULL) {
		reValue &hosts = res["hosts"];
		for (epp_string_seq::iterator i = r->m_hosts->begin();i!=r->m_hosts->end();i++) {
			hosts.set(*i,reValue::Null);
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
	epp_Command_ref cmd(newCommand(a,"DS"));
	epp_string_ref name(new epp_string(a.gl("name")));
	epp_SyncData_ref data(new epp_SyncData(epp_SyncMonthType(a.gi4("month")),epp_short(a.gi4("day"))));
	epp_DomainSyncReq_ref request(new epp_DomainSyncReq(cmd,name,data));
	// performing command
	epp_DomainSync_ref command(new epp_DomainSync());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainUpdateRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

reValue rePP::domainCheck (const reValue &a) {
	// preparing request
	epp_DomainCheckReq_ref request(new epp_DomainCheckReq());
	request->m_cmd.ref(newCommand(a,"DH"));
	request->m_names.ref(newStringSeq(a.get("names").csplit()));
	// performing command
	epp_DomainCheck_ref command(new epp_DomainCheck());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainCheckRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
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
	request->m_cmd.ref(newCommand(a,"DR"));
	request->m_name.ref(new epp_string(a.gl("name")));
	request->m_current_expiration_date.ref(new epp_date(a.gl("expires")));
	request->m_period.ref(new epp_DomainPeriod(YEAR,a.get("period").toInt4(1)));
	// performing command
	epp_DomainRenew_ref command(new epp_DomainRenew());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainRenewRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

reValue rePP::domainCreate (const reValue &a) {
	// preparing request
	epp_DomainCreateReq_ref request(new epp_DomainCreateReq());
	request->m_cmd.ref(newCommand(a,"DC"));
	request->m_name.ref(new epp_string(a.gl("name")));
	request->m_period.ref(new epp_DomainPeriod(YEAR,a.get("period").toInt4(1)));
	if (a.has("nameservers")) request->m_name_servers.ref(newStringSeq(a.get("nameservers").csplit()));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainCreate_ref command(new epp_DomainCreate());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainCreateRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_creation_date!=NULL) res["created_date"] = *r->m_creation_date;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

reValue rePP::domainUpdate (const reValue &a) {
	// preparing request
	epp_DomainUpdateReq_ref request(new epp_DomainUpdateReq());
	request->m_cmd.ref(newCommand(a,"DU"));
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
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainUpdateRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
	return res;
};

reValue rePP::domainDelete (const reValue &a) {
	// preparing request
	epp_DomainDeleteReq_ref request(new epp_DomainDeleteReq());
	request->m_cmd.ref(newCommand(a,"DD"));
	request->m_name.ref(new epp_string(a.gl("name")));
	// performing command
	epp_DomainDelete_ref command(new epp_DomainDelete());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainDeleteRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

reValue rePP::domainTransfer (const reValue &a) {
	// preparing request
	epp_DomainTransferReq_ref request(new epp_DomainTransferReq());
	request->m_cmd.ref(newCommand(a,"DT"));
	request->m_name.ref(new epp_string(a.gl("name")));
	if (a.has("period")) request->m_period.ref(new epp_DomainPeriod(YEAR,a.get("period").toInt4()));
	request->m_trans.ref(new epp_TransferRequest());
	try {
		request->m_trans->m_op.ref(new epp_TransferOpType(returnTransferType(a.gl("op"))));
	} catch (const epp_XMLException &ex) {
		printf("epp_XMLException!!!\n\n");
		return errorResult(9999,ex.getString());
	};
	if (a.has("password")) request->m_trans->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainTransfer_ref command(new epp_DomainTransfer());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainTransferRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp)+readDomainTrnData(r->m_trn_data);
};

reValue rePP::hostInfo (const reValue &a) {
	// preparing request
	epp_HostInfoReq_ref request(new epp_HostInfoReq());
	request->m_cmd.ref(newCommand(a,"HI"));
	request->m_name.ref(new epp_string(a.gl("name")));
	// performing command
	epp_HostInfo_ref command(new epp_HostInfo());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostInfoRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
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
			if (i->m_type!=NULL) statuses.set(returnStatusType(*i->m_type),reValue::Null);
		};
	};
	if (r->m_addresses!=NULL) {
		reValue &ips = res["ips"];
		for (epp_host_address_seq::iterator i=r->m_addresses->begin();i!=r->m_addresses->end();i++) {
			if (i->m_ip!=NULL) ips.set(*i->m_ip,reValue::Null);
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
	request->m_cmd.ref(newCommand(a,"HH"));
	request->m_names.ref(newStringSeq(a.get("names").csplit()));
	// performing command
	epp_HostCheck_ref command(new epp_HostCheck());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostCheckRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
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
	request->m_cmd.ref(newCommand(a,"HC"));
	request->m_name.ref(new epp_string(a.gl("name")));
	request->m_addresses.ref(newHostAddressSeq(a.get("ips").csplit()));
	// performing command
	epp_HostCreate_ref command(new epp_HostCreate());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostCreateRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_creation_date!=NULL) res["creation_date"] = *r->m_creation_date;
	return res;
};

reValue rePP::hostUpdate (const reValue &a) {
	// preparing request
	epp_HostUpdateReq_ref request(new epp_HostUpdateReq());
	request->m_cmd.ref(newCommand(a,"HU"));
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
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostUpdateRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
	return res;
};

reValue rePP::hostDelete (const reValue &a) {
	// preparing request
	epp_HostDeleteReq_ref request(new epp_HostDeleteReq());
	request->m_cmd.ref(newCommand(a,"HD"));
	request->m_name.ref(new epp_string(a.gl("name")));
	// performing command
	epp_HostDelete_ref command(new epp_HostDelete());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostDeleteRsp_ref r = command->getResponseData();
	reValue res = readResponseData(r->m_rsp);
	return res;
};

reValue rePP::contactCheck (const reValue &a) {
	// preparing request
	epp_ContactCheckReq_ref request(new epp_ContactCheckReq());
	request->m_cmd.ref(newCommand(a,"CH"));
	request->m_ids.ref(newStringSeq(a.get("ids").csplit()));
	// performing command
	epp_ContactCheck_ref command(new epp_ContactCheck());
	command->setRequestData(*request);
	reValue err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactCheckRsp_ref r = command->getResponseData();
	return reValue();
};

// SMART COMMANDS

void rePP::pollAck (const reValue &id) {
	poll(reValue(
		"op",	"ack",
		"id",	id
	));
};

reValue rePP::pollOne (const reValue &a) {
	reValue res = poll(a);
	if (res.has("msgQ_id")) pollAck(res.get("msgQ_id"));
	return res;
};

reValue rePP::pollAll (const reValue &a) {
	reValue p = poll(a);
	if (!p.has("msgQ_id")) return reValue::Null;
	reValue res(1,false);
	do {
		res.push(p);
		pollAck(p.get("msgQ_id"));
		p = poll(a);
	} while (p.has("msgQ_id"));
	return res;
};

reValue rePP::domainSmartCheck (const reValue &a) {
	const reValue names = a.get("names").csplit();
	bool check = true;
	reValue res;
	reValue b = a;
	for (reValue::size_type i=0,n=names.size();i<n;i++) {
		reLine name = names.gl(i);
		b.set("names",name);
		reValue h = domainCheck(b);
		if (h.has(name) && !h.get(name).get("avail").toBool()) check = false;
		res.incSelf(h);
	};
	res.set("check",check);
	return res;
};

reValue rePP::domainSmartUpdate (const reValue &a,const reValue &info) {
	reValue r = a;
	reValue old_s = info.get("statuses");
	bool old_u = old_s.del("clientUpdateProhibited");
	bool new_u = old_u;
	if (r.has("statuses")) {
		reValue new_s = r.get("statuses").ksplit();
		new_u = new_s.del("clientUpdateProhibited");
		reValue add_s,rem_s;
		for (reValue::size_type i=0,n=old_s.size();i<n;i++) if (!new_s.has(old_s.key(i))) rem_s.set(old_s.key(i),reValue::Null);
		for (reValue::size_type i=0,n=new_s.size();i<n;i++) if (!old_s.has(new_s.key(i))) add_s.set(new_s.key(i),reValue::Null);
		if (rem_s.size()) r.let("remove").set("statuses",rem_s);
		if (add_s.size()) r.let("add").set("statuses",add_s);
	};
	if (r.has("nameservers")) {
		reValue old_n = info.get("nameservers").lowercase();
		reValue new_n = r.get("nameservers").lowercase().ksplit();
		reValue add_n,rem_n;
		for (reValue::size_type i=0,n=old_n.size();i<n;i++) if (!new_n.has(old_n.key(i))) rem_n.set(old_n.key(i),reValue::Null);
		for (reValue::size_type i=0,n=new_n.size();i<n;i++) if (!old_n.has(new_n.key(i))) add_n.set(new_n.key(i),reValue::Null);
		if (rem_n.size()) r.let("remove").set("nameservers",rem_n);
		if (add_n.size()) {
			r.let("add").set("nameservers",add_n);
			reValue h = hostSmartCheck(reValue("ext",getExt(a),"names",add_n.keys()));
			for (reValue::size_type i=0,n=add_n.size();i<n;i++) {
				if (h.get(add_n.key(i)).get("avail").toBool()) hostCreate(reValue("ext",getExt(a),"name",add_n.key(i)));
			};
		};
	};
	if (a.has("new_password")) {
		reLine pwd = r.pop("new_password").toLine();
		if (pwd!=info.gl("password")) r.let("change").set("password",pwd);
	};
	/*
		printf("\n\n%s\n\n",r.dump2line().c_str());
		return reValue::Null;
	*/
	if (r.has("add") || r.has("remove") || r.has("change")) {
		if (old_u) domainAllowUpdate(a);
		if (new_u) r.let("add").let("statuses").set("clientUpdateProhibited",reValue::Null);
		reValue res = domainUpdate(r);
		if (!isResponseOk(res) && new_u) domainProhibitUpdate(a);
		return res;
	} else if (old_u && !new_u) {
		return domainAllowUpdate(a);
	} else if (new_u && !old_u) {
		return domainProhibitUpdate(a);
	} else return reValue::Null;
};

reValue rePP::domainAllowUpdate (const reValue &a) {
	return domainUpdate(a+reValue(
		"remove",reValue(
			"statuses","clientUpdateProhibited"
		)
	));
};

reValue rePP::domainProhibitUpdate (const reValue &a) {
	return domainUpdate(a+reValue(
		"add",reValue(
			"statuses","clientUpdateProhibited"
		)
	));
};

reValue rePP::domainSmartLock (const reValue &a) {
	reValue info = domainInfo(a);
	reValue o_s = info.get("statuses");
	reValue n_s = o_s;
	reValue s_s = ksplit("clientUpdateProhibited,clientDeleteProhibited,clientTransferProhibited");
	int go = 0;
	for (reValue::size_type i=0,n=s_s.size();i<n;i++) if (!o_s.has(s_s.key(i))) {
		n_s.set(s_s.key(i),reValue::Null);
		go++;
	};
	return go ? domainSmartUpdate(reValue("name",a.gl("name"),"statuses",n_s),info) : reValue::Null;
};

reValue rePP::domainSmartUnlock (const reValue &a) {
	reValue info = domainInfo(a);
	reValue o_s = info.get("statuses");
	reValue n_s(o_s.size(),true);
	reValue s_s = ksplit("clientUpdateProhibited,clientDeleteProhibited,clientTransferProhibited");
	int go = 0;
	for (reValue::size_type i=0,n=o_s.size();i<n;i++) {
		if (s_s.has(o_s.key(i))) go++;
		else n_s.set(o_s.key(i),reValue::Null);
	};
	return go ? domainSmartUpdate(reValue("name",a.gl("name"),"statuses",n_s),info) : reValue::Null;
};

reValue rePP::hostSmartCheck (const reValue &a) {
	const reValue names = a.get("names").csplit();
	bool check = true;
	reValue res;
	reValue b = a;
	for (reValue::size_type i=0,n=names.size();i<n;i++) {
		reLine name = names.gl(i);
		b.set("names",name);
		reValue h = hostCheck(b);
		if (h.has(name) && !h.get(name).get("avail").toBool()) check = false;
		res.incSelf(h);
	};
	res.set("check",check);
	return res;
};

reValue rePP::hostAllowUpdate (const reValue &a) {
	return hostUpdate(a+reValue(
		"remove",reValue(
			"statuses","clientUpdateProhibited"
		)
	));
};

reValue rePP::hostProhibitUpdate (const reValue &a) {
	return hostUpdate(a+reValue(
		"add",reValue(
			"statuses","clientUpdateProhibited"
		)
	));
};

reValue rePP::hostSmartUpdate (const reValue &a,const reValue &info) {
	reValue r = a;
	reValue old_s = info.get("statuses").ksplit();
	bool old_u = old_s.del("clientUpdateProhibited");
	bool new_u = old_u;
	if (r.has("statuses")) {
		reValue new_s = r.pop("statuses").ksplit();
		new_u = new_s.del("clientUpdateProhibited");
		reValue add_s,rem_s;
		for (reValue::size_type i=0,n=old_s.size();i<n;i++) if (!new_s.has(old_s.key(i))) rem_s.set(old_s.key(i),reValue::Null);
		for (reValue::size_type i=0,n=new_s.size();i<n;i++) if (!old_s.has(new_s.key(i))) add_s.set(new_s.key(i),reValue::Null);
		if (rem_s.size()) r.let("remove").set("statuses",rem_s);
		if (add_s.size()) r.let("add").set("statuses",add_s);
	};
	if (r.has("ips")) {
		reValue old_i = info.get("ips").ksplit();
		reValue new_i = r.pop("ips").ksplit();
		reValue add_i,rem_i;
		for (reValue::size_type i=0,n=old_i.size();i<n;i++) if (!new_i.has(old_i.key(i))) rem_i.set(old_i.key(i),reValue::Null);
		for (reValue::size_type i=0,n=new_i.size();i<n;i++) if (!old_i.has(new_i.key(i))) add_i.set(new_i.key(i),reValue::Null);
		if (rem_i.size()) r.let("remove").set("ips",rem_i);
		if (add_i.size()) r.let("add").set("ips",add_i);
	};
	if (a.has("new_name")) {
		reLine name = r.pop("new_name").toLine();
		if (name!=info.gl("name")) r.let("change").set("name",name);
	};
	if (r.has("add") || r.has("remove") || r.has("change")) {
		if (old_u) hostAllowUpdate(a);
		if (new_u) r.let("add").let("statuses").set("clientUpdateProhibited",reValue::Null);
		reValue res = hostUpdate(r);
		if (!isResponseOk(res) && new_u) hostProhibitUpdate(a);
		return res;
	} else if (old_u && !new_u) {
		return hostAllowUpdate(a);
	} else if (new_u && !old_u) {
		return hostProhibitUpdate(a);
	} else return info;
};

reValue rePP::hostSmartSet (const reValue &a,reValue info) {
	if (!isResponseOk(info)) {
		reValue r = hostCreate(a);
		if (!isResponseOk(r)) return r;
		info = r+a;
	};
	return hostSmartUpdate(a,info);
};

// EPP auxiliary functions

reValue rePP::safeProcessAction (epp_Action_ref command) {
	try {
		session.processAction(command);
		return reValue::Null;
	} catch (const epp_XMLException &ex) {
		printf("epp_XMLException!!!\n\n");
		return reValue(
			"result_code",		9999,
			"result_msg",		ex.getString()
		);
	} catch (const epp_Exception &ex) {
		printf("epp_Exception!!!\n\n");
		return readResultsData(ex.m_details)+readTransIDData(ex.m_trans_id);
	};
};

bool rePP::isDomainUpdatable (const reValue &statuses) {
	reValue f_stats = csplit("pendingTransfer,clientUpdateProhibited,serverUpdateProhibited");
	for (reValue::size_type i=0,n=f_stats.size();i<n;i++) if (statuses.has(f_stats.get(i))) return false;
	return true;
};

bool rePP::isResponseOk (const reValue &r) {
	return r.gi4("result_code")<2000;
};

// RTK auxiliary functions

void rePP::setNamestoreExtension (const reLine &ext,const reLine &data) {
	epp_NamestoreExt_ref e(new epp_NamestoreExt());
	e->setRequestData(epp_NamestoreExtData(data.size() ? data : "dot"+uppercase(ext.substr(1))));
	extensions.set(ext,e);
};

epp_string_seq *rePP::newStringSeq (const reValue &a) {
	epp_string_seq *res = new epp_string_seq();
	for (reValue::size_type i=0,n=a.size();i<n;i++) res->push_back(a.gk(i));
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
	for (reValue::size_type i=0,n=a.size();i<n;i++) res->push_back(DomainStatus(a.gk(i)));
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
	for (reValue::size_type i=0,n=a.size();i<n;i++) res->push_back(HostStatus(a.gk(i)));
	return res;
};

epp_host_address_seq *rePP::newHostAddressSeq (const reValue &a) {
	epp_host_address_seq *res = new epp_host_address_seq();
	for (reValue::size_type i=0,n=a.size();i<n;i++) {
		reLine ip = a.gk(i);
		epp_HostAddress address(ip.find(".")==reLine::npos ? IPV6 : IPV4,epp_string(ip));
		res->push_back(address);
	};
	return res;
};

reValue rePP::readDomainTrnData (const epp_PollResData_ref &p) {
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

reValue rePP::readTransIDData (const epp_TransID_ref &t) {
	reValue res;
	if (t!=NULL) {
		if (t->m_server_trid!=NULL) res["server_trid"] = *t->m_server_trid;
		if (t->m_client_trid!=NULL) res["client_trid"] = *t->m_client_trid;
	};
	return res;
};

reValue rePP::readMessageQueueData (const epp_MessageQueue_ref &m) {
	reValue res;
	if (m!=NULL) {
		if (m->m_count!=NULL) res["msgQ_count"] = (int32_t)*m->m_count;
		if (m->m_id!=NULL) res["msgQ_id"] = *m->m_id;
		if (m->m_msg!=NULL) res["msgQ_message"] = *m->m_msg->m_value;
		if (m->m_queue_date!=NULL) res["msgQ_date"] = *m->m_queue_date;
	};
	return res;
};

reValue rePP::readResultsData (const epp_result_seq_ref &r) {
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

reValue rePP::readResponseData (const epp_Response_ref &r) {
	return r==NULL ? reValue::Null : readResultsData(r->m_results)+readMessageQueueData(r->m_message_queue)+readTransIDData(r->m_trans_id);
};

