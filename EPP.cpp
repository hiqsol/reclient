// (c) Andrii Vasyliev
// EPP - EPP interface

#include <ostream>

#include "reclient/EPP.h"
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
#include "nameaddon/epp_EmailFwdInfo.h"
#include "nameaddon/epp_EmailFwdCheck.h"
#include "nameaddon/epp_EmailFwdRenew.h"
#include "nameaddon/epp_EmailFwdCreate.h"
#include "nameaddon/epp_EmailFwdUpdate.h"
#include "nameaddon/epp_EmailFwdDelete.h"
#include "nameaddon/epp_EmailFwdTransfer.h"
#include "epp-rtk-cpp/epp_HostInfo.h"
#include "epp-rtk-cpp/epp_HostCheck.h"
#include "epp-rtk-cpp/epp_HostCreate.h"
#include "epp-rtk-cpp/epp_HostUpdate.h"
#include "epp-rtk-cpp/epp_HostDelete.h"
#include "epp-rtk-cpp/epp_ContactCheck.h"
#include "epp-rtk-cpp/epp_ContactCreate.h"
#include "epp-rtk-cpp/epp_ContactInfo.h"
#include "epp-rtk-cpp/epp_ContactTransfer.h"
#include "epp-rtk-cpp/epp_ContactUpdate.h"
#include "epp-rtk-cpp/epp_ContactDelete.h"
#include "epp-rtk-cpp/data/epp_Greeting.h"
#include "epp-rtk-cpp/data/epp_domainXMLbase.h"
#include "nameaddon/data/epp_emailFwdXMLbase.h"
#include "epp-rtk-cpp/data/epp_hostXMLbase.h"
#include "epp-rtk-cpp/data/epp_contactXMLbase.h"
#include "epp-rtk-cpp/data/epp_Exception.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"
#include "comnetaddon/epp_DomainSync.h"
#include "comnetaddon/data/epp_LowBalancePollResData.h"
#include "comnetaddon/data/epp_RGPPollResData.h"

/// extensions
#include "reclient/DomainTrademark.h"
#include "reclient/ProSupplementalData.h"
#include "reclient/XxxLaunch.h"
#include "liberty-org-extensions/SecDNSCreate.h"
#include "liberty-org-extensions/SecDNSKeyData.h"
#include "liberty-org-extensions/SecDNSUpdate.h"
#include "liberty-org-extensions/SecDNSUpdateAdd.h"
#include "liberty-org-extensions/SecDNSUpdateChg.h"
#include "liberty-org-extensions/SecDNSUpdateRem.h"
#include "liberty-org-extensions/IDN.h"
#include "comnetaddon/epp_IDNLangExt.h"
#include "comnetaddon/data/epp_IDNLangExtData.h"

using namespace eppobject::domain;
using namespace eppobject::emailFwd;
using namespace eppobject::sync;
using namespace eppobject::host;
using namespace eppobject::contact;
using namespace eppobject::rgpPoll;
using namespace eppobject::idnLang;
using namespace eppobject::lowbalancePoll;

namespace re {

data_type EPP::poll (data_cref a) {
	// preparing request
	epp_PollReq_ref request(new epp_PollReq());
	request->m_cmd.ref(newCommand(a,"PO"));
	request->m_op.ref(new epp_PollOpType(a.getLine("op")=="ack" ? ACK : REQ));
	if (a.has("id")) request->m_msgID.ref(new epp_string(a.getLine("id")));
	// performing command
	epp_Poll_ref command(new epp_Poll());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_PollRsp_ref r = command->getResponseData();
	return	readDomainTrnData(r->m_res_data)+
		readContactTrnData(r->m_res_data)+
		readLowBalancePollData(r->m_res_data)+
		readRGPPollData(r->m_res_data)+
		readResponseData(r->m_rsp);
};

data_type EPP::hello (data_cref a) {
	// performing command
	epp_Hello_ref command(new epp_Hello());
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_Greeting_ref r = command->getResponseData();
	return data_null;
};

data_type EPP::login (data_cref a) {
	if (a.has("username")) setCredentials(a.getLine("username"),a.getLine("password"));
	// preparing request
	epp_LoginReq_ref request(new epp_LoginReq());
	request->m_cmd.ref(newCommand(a,"LI"));
	request->m_options.ref(new epp_Options(epp_string("1.0"),epp_string("en")));
	request->m_client_id.ref(new epp_string(username));
	request->m_password.ref(new epp_string(password));
	if (a.has("new_password")) request->m_new_password.ref(new epp_string(a.getLine("new_password")));
	// getting greeting and filling services info
	request->m_services.ref(new epp_objuri_seq());
	epp_Greeting_ref rsp = session.connectAndGetGreeting();
	if (rsp->m_svc_menu != NULL && rsp->m_svc_menu->m_services != NULL) {
		for (epp_objuri_seq::iterator i=rsp->m_svc_menu->m_services->begin();i!=rsp->m_svc_menu->m_services->end();i++) {
			request->m_services->push_back(*i); // COM,NET don't support contacts
		};
	};
	//request->m_services->push_back("urn:ietf:params:xml:ns:contact-1.0"); // COM,NET don't support contacts
	//request->m_services->push_back("urn:ietf:params:xml:ns:domain-1.0");
	//request->m_services->push_back("urn:ietf:params:xml:ns:host-1.0");
	// performing command
	epp_Login_ref command(new epp_Login());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_LoginRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::logout (data_cref a) {
	// preparing request
	epp_LogoutReq_ref request(new epp_LogoutReq());
	request->m_client_trid.ref(new epp_trid(trID(a.getLine("trID"),"LO")));
	// performing command
	epp_Logout_ref command(new epp_Logout());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_LogoutRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

line_type EPP::getExt (line_cref a) {
	size_type pos = a.rfind('.');
	return pos==line_npos ? a : a.substr(pos);
};

line_type EPP::getExt (data_cref a) {
	if (a.has("ext"))       return a.getLine("ext");
	if (a.has("trademark")) return "trademark";
	if (a.has("idnScript")) return "idnScript";
	if (a.has("idnLang"))   return "idnLang";
	if (a.has("secdns"))    return "secdns";
	if (a.has("pro"))       return "pro";
	if (a.has("launch"))    return "launch";
	if (a.has("zone"))      return a.getLine("zone");
	if (a.has("name"))      return getExt(a.getLine("name"));
	if (a.has("names")) {
		line_type n = a.getLine("names");
		return getExt(n.substr(0,n.find(',')));
	};
	return "";
};

epp_Extension_ref EPP::getExtension (data_cref a,line_cref e) {
	line_cref ext = e.size() ? e : getExt(a);
	if ("trademark"==ext)   return domainTrademark(a);
	if ("idnScript"==ext)   return domainIDNScript(a);
    if ("idnLang"==ext)     return domainIDNLang(a);
    if ("secdns"==ext)      return domainSecDNS(a);
    if ("pro"==ext)         return domainPro(a);
    if ("launch"==ext)      return domainLaunch(a);
	return extensions.has(ext) ? extensions.let(ext) : NULL;
};

epp_extension_ref_seq_ref EPP::getExtensions (data_cref a) {
    data_type b = a;
    epp_extension_ref_seq_ref res;
    res.ref(new epp_extension_ref_seq());
    while (1) {
        line_type ext = getExt(b);
        if (ext.empty()) break;
        b.del(ext);
        res->push_back(getExtension(a,ext));
    };
    return res;
};

data_type EPP::domainInfo (data_cref a) {
	// preparing request
	epp_DomainInfoReq_ref request(new epp_DomainInfoReq());
	request->m_cmd.ref(newCommand(a,"DI"));
		//l_req->m_cmd.ref(new epp_Command(NULL,epp_Unspec_ref(new epp_Unspec()),epp_trid("ABC-12346")));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("type")) request->m_hosts_type.ref(newDomainHostsType(a.getLine("type")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainInfo_ref command(new epp_DomainInfo());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainInfoRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = lc(*r->m_name);
	if (r->m_roid!=NULL) res["roid"] = *r->m_roid;
	if (r->m_status!=NULL) {
		data_type &statuses = res["statuses"];
		for (epp_domain_status_seq::iterator i = r->m_status->begin();i!=r->m_status->end();i++) {
			if (statuses.size()) statuses += ",";
			statuses += returnStatusType(*i->m_type);
		};
	};
	if (r->m_registrant!=NULL) res["registrant"] = *r->m_registrant;
	if (r->m_contacts!=NULL) {
		for (epp_domain_contact_seq::iterator i = r->m_contacts->begin();i!=r->m_contacts->end();i++) {
			data_type &c = res.let(returnContactType(*i->m_type));
			if (c.size()) c += ",";
			c += *i->m_id;
		};
	};
	if (r->m_name_servers!=NULL) {
		data_type &nses = res["nameservers"];
		for (epp_string_seq::iterator i = r->m_name_servers->begin();i!=r->m_name_servers->end();i++) {
			if (nses.size()) nses += ",";
			nses += lc(*i);
		};
	};
	if (r->m_hosts!=NULL) {
		data_type &hosts = res["hosts"];
		for (epp_string_seq::iterator i = r->m_hosts->begin();i!=r->m_hosts->end();i++) {
			if (hosts.size()) hosts += ",";
			hosts += lc(*i);
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
	// getting extensions info
	if(r->m_rsp->m_ext_strings != NULL) {
		DomainTrademark_ref trademark(new DomainTrademark());
		eppobject::epp::epp_xml_string_seq_ref extension_strings;
		extension_strings = r->m_rsp->m_ext_strings;
		eppobject::epp::epp_xml_string first_extension = extension_strings->front();
		trademark->fromXML(first_extension);
		if (trademark->m_name!=NULL)		res["trademark"]		= *trademark->m_name;
		if (trademark->m_date!=NULL)		res["trademark_date"]		= *trademark->m_date;
		if (trademark->m_number!=NULL)		res["trademark_number"]		= *trademark->m_number;
		if (trademark->m_country!=NULL)		res["trademark_country"]	= *trademark->m_country;
		if (trademark->m_owner_country!=NULL)	res["trademark_owner_country"]	= *trademark->m_owner_country;
	};
	return res;
};

data_type EPP::domainSync (data_cref a) {
	// preparing request
	epp_Command_ref cmd(newCommand(a,"DS"));
	epp_string_ref name(new epp_string(a.getLine("name")));
	epp_SyncData_ref data(new epp_SyncData(epp_SyncMonthType(a.getIntN("month")),epp_short(a.getIntN("day"))));
	epp_DomainSyncReq_ref request(new epp_DomainSyncReq(cmd,name,data));
	// performing command
	epp_DomainSync_ref command(new epp_DomainSync());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainUpdateRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::domainCheck (data_cref a) {
	// preparing request
	epp_DomainCheckReq_ref request(new epp_DomainCheckReq());
	request->m_cmd.ref(newCommand(a,"DH"));
	request->m_names.ref(newStringSeq(a.get("names").csplited()));
	// performing command
	epp_DomainCheck_ref command(new epp_DomainCheck());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainCheckRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			data_type s;
			if (i->m_avail!=NULL) s["avail"] = *i->m_avail;
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_reason!=NULL) s["reason"] = *i->m_reason;
			if (i->m_value!=NULL) res[lc(*i->m_value)] = s; else res.push(s);
		};
	};
	return res;
};

data_type EPP::domainSimpleCheck (data_cref a) {
	// preparing request
	epp_DomainCheckReq_ref request(new epp_DomainCheckReq());
	request->m_cmd.ref(newCommand(a,"DH"));
	request->m_names.ref(newStringSeq(a.get("names").csplited()));
	// performing command
	epp_DomainCheck_ref command(new epp_DomainCheck());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainCheckRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			if (i->m_value!=NULL && i->m_avail!=NULL) res[lc(*i->m_value)] = *i->m_avail;
		};
	};
	return res;
};

data_type EPP::domainRenew (data_cref a) {
	// preparing request
	epp_DomainRenewReq_ref request(new epp_DomainRenewReq());
	request->m_cmd.ref(newCommand(a,"DR"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	request->m_current_expiration_date.ref(new epp_date(a.getLine("expires")));
	request->m_period.ref(new epp_DomainPeriod(eppobject::domain::YEAR,a.get("period").toIntN(1)));
	// performing command
	epp_DomainRenew_ref command(new epp_DomainRenew());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainRenewRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = lc(*r->m_name);
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

data_type EPP::domainCreate (data_cref a) {
	// preparing request
	epp_DomainCreateReq_ref request(new epp_DomainCreateReq());
	request->m_cmd.ref(newCommand(a,"DC"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	request->m_period.ref(new epp_DomainPeriod(eppobject::domain::YEAR,a.get("period").toIntN(1)));
	if (a.has("nameservers")) request->m_name_servers.ref(newStringSeq(a.get("nameservers").csplited()));
	if (a.has("registrant")) request->m_registrant.ref(new epp_string(a.getLine("registrant")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	if (a.hasAny("admin","tech","billing")) request->m_contacts.ref(newDomainContactSeq(a));
	// performing command
	epp_DomainCreate_ref command(new epp_DomainCreate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainCreateRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_creation_date!=NULL) res["created_date"] = *r->m_creation_date;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

epp_Extension_ref EPP::domainSecDNS (data_cref a) {
	if (!a.has("secdns")) return NULL;
    data_cref secdns = a.get("secdns");
    if (secdns.has("create")) {
        data_cref adds = secdns.get("create");
        SecDNSCreate_ref secdns_create(new SecDNSCreate());
        secdns_create->m_ds_data.ref(new SecDNSDsData_seq());
        for (size_type i=0,n=adds.size();i<n;i++) {
            SecDNSDsData_ref ds_data = SecDNS(adds.get(i));
            secdns_create->m_ds_data->push_back(*ds_data);
        };
        return secdns_create;
    } else if (secdns.has("update")) {
        data_cref update = secdns.get("update");
        SecDNSUpdate_ref secdns_update(new SecDNSUpdate());
        secdns_update->m_urgent = true;
        if (update.has("add")) {
            data_cref adds = update.get("add");
            secdns_update->m_add.ref(new SecDNSUpdateAdd());
            secdns_update->m_add->m_ds_data.ref(new SecDNSDsData_seq());
            for (size_type i=0,n=adds.size();i<n;i++) {
                SecDNSDsData_ref ds_data = SecDNS(adds.get(i));
                secdns_update->m_add->m_ds_data->push_back(*ds_data);
            };
        };
        if (update.has("remove")) {
            data_cref rems = update.get("remove");
            secdns_update->m_rem.ref(new SecDNSUpdateRem());
            if (rems=="all") {
                secdns_update->m_rem->m_all = true;
            } else {
                secdns_update->m_rem->m_ds_data.ref(new SecDNSDsData_seq());
                for (size_type i=0,n=rems.size();i<n;i++) {
                    SecDNSDsData_ref ds_data = SecDNS(rems.get(i));
                    secdns_update->m_rem->m_ds_data->push_back(*ds_data);
                };
            };
        };
        if (update.has("change")) {
            secdns_update->m_chg.ref(new SecDNSUpdateChg());
            secdns_update->m_chg->m_ds_data.ref(new SecDNSDsData_seq());
            SecDNSDsData_ref ds_old = SecDNS(update.get("change").get("old"));
            SecDNSDsData_ref ds_new = SecDNS(update.get("change").get("new"));
            secdns_update->m_chg->m_ds_data->push_back(*ds_old);
            secdns_update->m_chg->m_ds_data->push_back(*ds_new);
        };
        return secdns_update;
    };
	return NULL;
};

SecDNSDsData_ref EPP::SecDNS (data_cref a) {
    SecDNSDsData_ref ds_data(new SecDNSDsData());
    ds_data->m_ds_key_tag.ref(new epp_long(a.getIntN("key_tag")));
    ds_data->m_ds_alg.ref(new epp_short(a.getIntN("algorithm")));
    ds_data->m_ds_digest_type.ref(new epp_short(a.getIntN("digest_type")));
    ds_data->m_ds_digest.ref(new epp_string(a.getLine("digest")));
    // ds_data->m_ds_max_sig_life.ref(new epp_long(604800));
    // ds_data->m_ds_key_data.ref(new SecDNSKeyData());
    // ds_data->m_ds_key_data->m_ds_flags.ref(new epp_short(256));
    // ds_data->m_ds_key_data->m_ds_protocol.ref(new epp_short(3));
    // ds_data->m_ds_key_data->m_ds_alg.ref(new epp_short(1));
    // ds_data->m_ds_key_data->m_ds_pub_key.ref(new epp_string("AQOyKAL4mM81OhKl9dlhJ8nw5m2S0z0EEpVtHo4AUzjxgU6mupRZXjjSnavw4QxCTFPuwTpGGqAG4WQCwKh5Sr1OJ/xAxUIdp5Sbi/P/ugUcdau1QJydq3yvxLIOTOv9FZuLRbpNPZGqXSfMD5kWTbZwtFS8jbmxJojoGp1xBSej0Q=="));
    return ds_data;
};

epp_Extension_ref EPP::domainIDNLang (data_cref a) {
    epp_IDNLangExt_ref ext(new epp_IDNLangExt());
    epp_IDNLangExtData data(a.getLine("idnLang"));
    ext->setRequestData(data);
    return ext;
};

epp_Extension_ref EPP::domainIDNScript (data_cref a) {
    IDN_ref ext(new IDN(true));
    line_type script  = a.getLine("idnScript");
    line_type command = a.getLine("idnCommand");
    ext->m_script.ref(new epp_string(script.size() ? script : "ru"));
    ext->m_command.ref(new epp_string(command.size() ? command : "create"));
    return ext;
};

epp_Extension_ref EPP::domainPro (data_cref a) {
    ProSupplementalData_ref ext(new ProSupplementalData());
    data_cref pro = a.get("pro");
    ext->m_op.ref           (new epp_string(pro.getLine("op")));
    ext->m_profession.ref   (new epp_string(pro.getLine("profession")));
    ext->m_authorityName.ref(new epp_string(pro.getLine("authorityName")));
    ext->m_authorityUrl.ref (new epp_string(pro.getLine("authorityUrl")));
    ext->m_licenseNumber.ref(new epp_string(pro.getLine("licenseNumber")));
    return ext;
};

epp_Extension_ref EPP::domainLaunch (data_cref a) {
    XxxLaunch_ref ext(new XxxLaunch());
    data_cref pro = a.get("launch");
    ext->m_op.ref           (new epp_string(pro.get("op").toLine(pro.toLine())));
    ext->m_name.ref         (new epp_string(pro.get("name").toLine()));
//  ext->m_name.ref         (new epp_string(pro.get("name").toLine("restricted")));
    ext->m_phase.ref        (new epp_string(pro.get("phase").toLine("open")));
    ext->m_noticeID.ref     (new epp_string(pro.getLine("noticeID")));
    ext->m_notAfter.ref     (new epp_string(pro.getLine("notAfter")));
    ext->m_acceptedDate.ref (new epp_string(pro.getLine("acceptedDate")));
    return ext;
};

epp_Extension_ref EPP::domainTrademark (data_cref a) {
	if (!a.has("trademark")) return NULL;
	DomainTrademark_ref trademark(new DomainTrademark());
						trademark->m_op.ref		(new epp_string(a.getLine("trademark_op")));
						trademark->m_name.ref		(new epp_string(a.getLine("trademark")));
	if (a.has("trademark_date"))		trademark->m_date.ref		(new epp_string(a.getLine("trademark_date")));
	if (a.has("trademark_number"))		trademark->m_number.ref		(new epp_string(a.getLine("trademark_number")));
	if (a.has("trademark_country"))		trademark->m_country.ref	(new epp_string(a.getLine("trademark_country")));
	if (a.has("trademark_owner_country"))	trademark->m_owner_country.ref	(new epp_string(a.getLine("trademark_owner_country")));
	return trademark;
};

data_type EPP::domainUpdate (data_cref a) {
	// preparing request
	epp_DomainUpdateReq_ref request(new epp_DomainUpdateReq());
	request->m_cmd.ref(newCommand(a,"DU"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("add")) {
		data_cref add = a.get("add");
		request->m_add.ref(new epp_DomainUpdateAddRemove());
		if (add.has("statuses")) request->m_add->m_status.ref(newDomainStatusSeq(add.get("statuses").csplited()));
		if (add.has("nameservers")) request->m_add->m_name_servers.ref(newStringSeq(add.get("nameservers").csplited()));
		if (add.hasAny("admin","tech","billing")) request->m_add->m_contacts.ref(newDomainContactSeq(add));
	};
	if (a.has("remove")) {
		data_cref remove = a.get("remove");
		request->m_remove.ref(new epp_DomainUpdateAddRemove());
		if (remove.has("statuses")) request->m_remove->m_status.ref(newDomainStatusSeq(remove.get("statuses").csplited()));
		if (remove.has("nameservers")) request->m_remove->m_name_servers.ref(newStringSeq(remove.get("nameservers").csplited()));
		if (remove.hasAny("admin","tech","billing")) request->m_remove->m_contacts.ref(newDomainContactSeq(remove));
	};
	if (a.has("change")) {
		data_cref change = a.get("change");
		request->m_change.ref(new epp_DomainUpdateChange());
		if (change.has("password")) request->m_change->m_auth_info.ref(newAuthInfo(change));
		if (change.has("registrant")) request->m_change->m_registrant.ref(new epp_string(change.getLine("registrant")));
	};
	// performing command
	epp_DomainUpdate_ref command(new epp_DomainUpdate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainUpdateRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::domainDelete (data_cref a) {
	// preparing request
	epp_DomainDeleteReq_ref request(new epp_DomainDeleteReq());
	request->m_cmd.ref(newCommand(a,"DD"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	// performing command
	epp_DomainDelete_ref command(new epp_DomainDelete());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainDeleteRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::domainTransfer (data_cref a) {
	// for .NAME zone try create nameservers
	line_type name = a.getLine("name");
	if (name.substr(name.size()-5)==".name") {
		data_type info = domainInfo(a);
		if (info.has("nameservers")) {
			data_type nses = info.get("nameservers").csplited();
			for (size_type i=0,n=nses.size();i<n;i++) hostCreate(data_type("name",nses.get(i)));
		};
	};
	// preparing request
	epp_DomainTransferReq_ref request(new epp_DomainTransferReq());
	request->m_cmd.ref(newCommand(a,"DT"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("period")) request->m_period.ref(new epp_DomainPeriod(eppobject::domain::YEAR,a.get("period").toIntN()));
	request->m_trans.ref(new epp_TransferRequest());
	try {
		request->m_trans->m_op.ref(new epp_TransferOpType(returnTransferType(a.getLine("op"))));
	} catch (const epp_XMLException &ex) {
		printf("epp_XMLException!!!\n\n");
		return errorResult(9999,ex.getString());
	};
	if (a.has("password")) request->m_trans->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_DomainTransfer_ref command(new epp_DomainTransfer());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_DomainTransferRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp)+readDomainTrnData(r->m_trn_data);
};

data_type EPP::hostInfo (data_cref a) {
	// preparing request
	epp_HostInfoReq_ref request(new epp_HostInfoReq());
	request->m_cmd.ref(newCommand(a,"HI"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	// performing command
	epp_HostInfo_ref command(new epp_HostInfo());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostInfoRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = lc(*r->m_name);
	if (r->m_roid!=NULL) res["roid"] = *r->m_roid;
	if (r->m_status!=NULL) {
		data_type &statuses = res["statuses"];
		for (epp_host_status_seq::iterator i = r->m_status->begin();i!=r->m_status->end();i++) {
			if (statuses.size()) statuses += ",";
			statuses += returnStatusType(*i->m_type);
		};
	};
	if (r->m_addresses!=NULL) {
		data_type &ips = res["ips"];
		for (epp_host_address_seq::iterator i=r->m_addresses->begin();i!=r->m_addresses->end();i++) {
			if (ips.size()) ips += ",";
			ips += *i->m_ip;
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

data_type EPP::hostCheck (data_cref a) {
	// preparing request
	epp_HostCheckReq_ref request(new epp_HostCheckReq());
	request->m_cmd.ref(newCommand(a,"HH"));
	request->m_names.ref(newStringSeq(a.get("names").csplited()));
	// performing command
	epp_HostCheck_ref command(new epp_HostCheck());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostCheckRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			data_type s;
			if (i->m_avail!=NULL) s["avail"] = *i->m_avail;
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_reason!=NULL) s["reason"] = *i->m_reason;
			if (i->m_value!=NULL) res[lc(*i->m_value)] = s; else res.push(s);
		};
	};
	return res;
};

data_type EPP::hostCreate (data_cref a) {
	// preparing request
	epp_HostCreateReq_ref request(new epp_HostCreateReq());
	request->m_cmd.ref(newCommand(a,"HC"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	request->m_addresses.ref(newHostAddressSeq(a.get("ips").csplited()));
	// performing command
	epp_HostCreate_ref command(new epp_HostCreate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostCreateRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = lc(*r->m_name);
	if (r->m_creation_date!=NULL) res["creation_date"] = *r->m_creation_date;
	return res;
};

data_type EPP::hostUpdate (data_cref a) {
	// preparing request
	epp_HostUpdateReq_ref request(new epp_HostUpdateReq());
	request->m_cmd.ref(newCommand(a,"HU"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("add")) {
		data_cref add = a.get("add");
		request->m_add.ref(new epp_HostUpdateAddRemove());
		if (add.has("statuses")) request->m_add->m_status.ref(newHostStatusSeq(add.get("statuses").csplited()));
		if (add.has("ips")) request->m_add->m_addresses.ref(newHostAddressSeq(add.get("ips").csplited()));
	};
	if (a.has("remove")) {
		data_cref remove = a.get("remove");
		request->m_remove.ref(new epp_HostUpdateAddRemove());
		if (remove.has("statuses")) request->m_remove->m_status.ref(newHostStatusSeq(remove.get("statuses").csplited()));
		if (remove.has("ips")) request->m_remove->m_addresses.ref(newHostAddressSeq(remove.get("ips").csplited()));
	};
	if (a.has("change")) {
		data_cref change = a.get("change");
		request->m_change.ref(new epp_HostUpdateChange());
		if (change.has("name")) request->m_change->m_name.ref(new epp_string(change.getLine("name")));
	};
	// performing command
	epp_HostUpdate_ref command(new epp_HostUpdate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostUpdateRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	return res;
};

data_type EPP::hostDelete (data_cref a) {
	// preparing request
	epp_HostDeleteReq_ref request(new epp_HostDeleteReq());
	request->m_cmd.ref(newCommand(a,"HD"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	// performing command
	epp_HostDelete_ref command(new epp_HostDelete());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_HostDeleteRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	return res;
};

data_type EPP::contactCheck (data_cref a) {
	// preparing request
	epp_ContactCheckReq_ref request(new epp_ContactCheckReq());
	request->m_cmd.ref(newCommand(a,"CH"));
	request->m_ids.ref(newStringSeq(a.get("ids").csplited()));
	// performing command
	epp_ContactCheck_ref command(new epp_ContactCheck());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactCheckRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			data_type s;
			if (i->m_avail!=NULL) s["avail"] = *i->m_avail;
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_reason!=NULL) s["reason"] = *i->m_reason;
			if (i->m_value!=NULL) res[*i->m_value] = s; else res.push(s);
		};
	};
	return res;
};

data_type EPP::contactCreate (data_cref a) {
	// preparing request
	epp_ContactCreateReq_ref request(new epp_ContactCreateReq());
	request->m_cmd.ref(newCommand(a,"CC"));
	request->m_id.ref(new epp_string(a.getLine("id")));
	if (hasContactAddressName(a)) request->m_addresses.ref(newContactNameAddressSeq(a));
	if (a.has("voice_phone")) request->m_voice.ref(newContactPhone(a.get("voice_phone"),a.get("voice_extension")));
	if (a.has("fax_phone")) request->m_fax.ref(newContactPhone(a.get("fax_phone"),a.get("fax_extension")));
	if (a.has("email")) request->m_email.ref(new epp_string(a.getLine("email")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_ContactCreate_ref command(new epp_ContactCreate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactCreateRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_id!=NULL) res["id"] = *r->m_id;
	if (r->m_creation_date!=NULL) res["creation_date"] = *r->m_creation_date;
	return res;
};

data_type EPP::contactInfo (data_cref a) {
	// preparing request
	epp_ContactInfoReq_ref request(new epp_ContactInfoReq());
	request->m_cmd.ref(newCommand(a,"CI"));
	request->m_id.ref(new epp_string(a.getLine("id")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_ContactInfo_ref command(new epp_ContactInfo());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactInfoRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_id!=NULL) res["id"] = *r->m_id;
	if (r->m_roid!=NULL) res["roid"] = *r->m_roid;
	if (r->m_status!=NULL) {
		data_type &statuses = res["statuses"];
		for (epp_contact_status_seq::iterator i=r->m_status->begin();i!=r->m_status->end();i++) {
			if (statuses.size()) statuses += ",";
			statuses += returnStatusType(*i->m_type);
		};
	};
	if (r->m_addresses!=NULL) {
		for (epp_ContactNameAddress_seq::iterator i=r->m_addresses->begin();i!=r->m_addresses->end();i++ ) {
			if (*i->m_type==LOC) res["postal_info_type"] = "LOC";
			if (i->m_name!=NULL) res["name"] = *i->m_name;
			if (i->m_org!=NULL) res["organization"] = *i->m_org;
			if (i->m_address!=NULL) {
				if (i->m_address->m_street1!=NULL) res["street1"] = *i->m_address->m_street1;
				if (i->m_address->m_street2!=NULL) res["street2"] = *i->m_address->m_street2;
				if (i->m_address->m_street3!=NULL) res["street3"] = *i->m_address->m_street3;
				if (i->m_address->m_city!=NULL) res["city"] = *i->m_address->m_city;
				if (i->m_address->m_state_province!=NULL) res["province"] = *i->m_address->m_state_province;
				if (i->m_address->m_postal_code!=NULL) res["postal_code"] = *i->m_address->m_postal_code;
				if (i->m_address->m_country_code!=NULL) res["country"] = *i->m_address->m_country_code;
			};
		};
	};
	if (r->m_voice!=NULL) {
		if (r->m_voice->m_value!=NULL) res["voice_phone"] = *r->m_voice->m_value;
		if (r->m_voice->m_extension!=NULL) res["voice_extension"] = *r->m_voice->m_extension;
	};
	if (r->m_fax!=NULL) {
		if (r->m_fax->m_value!=NULL) res["fax_phone"] = *r->m_fax->m_value;
		if (r->m_fax->m_extension!=NULL) res["fax_extension"] = *r->m_fax->m_extension;
	};
	if (r->m_email!=NULL) res["email"] = *r->m_email;
	if (r->m_client_id!=NULL) res["client_id"] = *r->m_client_id;
	if (r->m_created_by!=NULL) res["created_by"] = *r->m_created_by;
	if (r->m_created_date!=NULL) res["created_date"] = *r->m_created_date;
	if (r->m_updated_by!=NULL) res["updated_by"] = *r->m_updated_by;
	if (r->m_updated_date!=NULL) res["updated_date"] = *r->m_updated_date;
	if (r->m_transfer_date!=NULL) res["transfer_date"] = *r->m_transfer_date;
	if (r->m_auth_info!=NULL && r->m_auth_info->m_value!=NULL) res["password"] = *r->m_auth_info->m_value;
	// getting extensions info
	if(r->m_rsp->m_ext_strings != NULL) {
		ProSupplementalData_ref pro(new ProSupplementalData());
		eppobject::epp::epp_xml_string_seq_ref extension_strings;
		extension_strings = r->m_rsp->m_ext_strings;
		eppobject::epp::epp_xml_string first_extension = extension_strings->front();
		pro->fromXML(first_extension);
		if (pro->m_profession!=NULL)		res["profession"]		= *pro->m_profession;
		if (pro->m_authorityName!=NULL)		res["authorityName"]    = *pro->m_authorityName;
		if (pro->m_authorityUrl!=NULL)		res["authorityUrl"]		= *pro->m_authorityUrl;
		if (pro->m_licenseNumber!=NULL)		res["licenseNumber"]	= *pro->m_licenseNumber;
	};
	return res;
};

data_type EPP::contactTransfer (data_cref a) {
	// preparing request
	epp_ContactTransferReq_ref request(new epp_ContactTransferReq());
	request->m_cmd.ref(newCommand(a,"CT"));
	request->m_id.ref(new epp_string(a.getLine("id")));
	request->m_trans.ref(new epp_TransferRequest());
	try {
		request->m_trans->m_op.ref(new epp_TransferOpType(returnTransferType(a.getLine("op"))));
	} catch (const epp_XMLException &ex) {
		printf("epp_XMLException!!!\n\n");
		return errorResult(9999,ex.getString());
	};
	if (a.has("password")) request->m_trans->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_ContactTransfer_ref command(new epp_ContactTransfer());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactTransferRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp)+readContactTrnData(r->m_trn_data);
};

data_type EPP::contactUpdate (data_cref a) {
	// preparing request
	epp_ContactUpdateReq_ref request(new epp_ContactUpdateReq());
	request->m_cmd.ref(newCommand(a,"CU"));
	request->m_id.ref(new epp_string(a.getLine("id")));
	if (a.has("add")) {
		data_cref add = a.get("add");
		request->m_add.ref(new epp_ContactUpdateAddRemove());
		if (add.has("statuses")) request->m_add->m_status.ref(newContactStatusSeq(add.get("statuses").csplited()));
	};
	if (a.has("remove")) {
		data_cref remove = a.get("remove");
		request->m_remove.ref(new epp_ContactUpdateAddRemove());
		if (remove.has("statuses")) request->m_remove->m_status.ref(newContactStatusSeq(remove.get("statuses").csplited()));
	};
	if (a.has("change")) {
		data_cref change = a.get("change");
		if (hasContactData(change)) {
			request->m_change.ref(new epp_ContactUpdateChange());
			if (hasContactAddressName(change))			request->m_change->m_addresses.ref(newContactNameAddressSeq(change));
			if (change.hasAny("voice_phone","voice_extension"))	request->m_change->m_voice.ref(newContactPhone(change.get("voice_phone"),change.get("voice_extension")));
			if (change.hasAny("fax_phone","fax_extension"))		request->m_change->m_fax.ref(newContactPhone(change.get("fax_phone"),change.get("fax_extension")));
			if (change.has("email"))				request->m_change->m_email.ref(new epp_string(change.getLine("email")));
			if (change.has("password"))				request->m_change->m_auth_info.ref(newAuthInfo(change));
		};
	};
	// performing command
	epp_ContactUpdate_ref command(new epp_ContactUpdate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactUpdateRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::contactDelete (data_cref a) {
	// preparing request
	epp_ContactDeleteReq_ref request(new epp_ContactDeleteReq());
	request->m_cmd.ref(newCommand(a,"CD"));
	request->m_id.ref(new epp_string(a.getLine("id")));
	// performing command
	epp_ContactDelete_ref command(new epp_ContactDelete());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_ContactDeleteRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

// EMAIL FORWARD COMMANDS

data_type EPP::emailFwdInfo (data_cref a) {
	// preparing request
	epp_EmailFwdInfoReq_ref request(new epp_EmailFwdInfoReq());
	request->m_cmd.ref(newCommand(a,"EI"));
		//l_req->m_cmd.ref(new epp_Command(NULL,epp_Unspec_ref(new epp_Unspec()),epp_trid("ABC-12346")));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_EmailFwdInfo_ref command(new epp_EmailFwdInfo());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdInfoRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = lc(*r->m_name);
	if (r->m_roid!=NULL) res["roid"] = *r->m_roid;
	if (r->m_status!=NULL) {
		data_type &statuses = res["statuses"];
		for (epp_emailFwd_status_seq::iterator i = r->m_status->begin();i!=r->m_status->end();i++) {
			if (statuses.size()) statuses += ",";
			statuses += returnStatusType(*i->m_type);
		};
	};
	if (r->m_registrant!=NULL) res["registrant"] = *r->m_registrant;
	if (r->m_contacts!=NULL) {
		for (epp_emailFwd_contact_seq::iterator i = r->m_contacts->begin();i!=r->m_contacts->end();i++) {
			data_type &c = res.let(returnContactType(*i->m_type));
			if (c.size()) c += ",";
			c += *i->m_id;
		};
	};
	if (r->m_fwdto!=NULL) res["fwdTo"] = *r->m_fwdto;
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

data_type EPP::emailFwdCheck (data_cref a) {
	// preparing request
	epp_EmailFwdCheckReq_ref request(new epp_EmailFwdCheckReq());
	request->m_cmd.ref(newCommand(a,"EH"));
	request->m_names.ref(newStringSeq(a.get("names").csplited()));
	// performing command
	epp_EmailFwdCheck_ref command(new epp_EmailFwdCheck());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdCheckRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_results!=NULL) {
		for (epp_check_result_seq::iterator i=r->m_results->begin();i!=r->m_results->end();i++) {
			data_type s;
			if (i->m_avail!=NULL) s["avail"] = *i->m_avail;
			if (i->m_lang!=NULL) s["lang"] = *i->m_lang;
			if (i->m_reason!=NULL) s["reason"] = *i->m_reason;
			if (i->m_value!=NULL) res[lc(*i->m_value)] = s; else res.push(s);
		};
	};
	return res;
};

data_type EPP::emailFwdRenew (data_cref a) {
	// preparing request
	epp_EmailFwdRenewReq_ref request(new epp_EmailFwdRenewReq());
	request->m_cmd.ref(newCommand(a,"ER"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	request->m_current_expiration_date.ref(new epp_date(a.getLine("expires")));
	request->m_period.ref(new epp_EmailFwdPeriod(eppobject::emailFwd::YEAR,a.get("period").toIntN(1)));
	// performing command
	epp_EmailFwdRenew_ref command(new epp_EmailFwdRenew());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdRenewRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = lc(*r->m_name);
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

data_type EPP::emailFwdCreate (data_cref a) {
	// preparing request
	epp_EmailFwdCreateReq_ref request(new epp_EmailFwdCreateReq());
	request->m_cmd.ref(newCommand(a,"EC"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	request->m_period.ref(new epp_EmailFwdPeriod(eppobject::emailFwd::YEAR,a.get("period").toIntN(1)));
	if (a.has("fwdTo")) request->m_fwdto.ref(new epp_string(a.getLine("fwdTo")));
	if (a.has("registrant")) request->m_registrant.ref(new epp_string(a.getLine("registrant")));
	if (a.has("password")) request->m_auth_info.ref(newAuthInfo(a));
	if (a.hasAny("admin","tech","billing")) request->m_contacts.ref(newEmailFwdContactSeq(a));
	// performing command
	epp_EmailFwdCreate_ref command(new epp_EmailFwdCreate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdCreateRsp_ref r = command->getResponseData();
	data_type res = readResponseData(r->m_rsp);
	if (r->m_name!=NULL) res["name"] = *r->m_name;
	if (r->m_creation_date!=NULL) res["created_date"] = *r->m_creation_date;
	if (r->m_expiration_date!=NULL) res["expiration_date"] = *r->m_expiration_date;
	return res;
};

data_type EPP::emailFwdUpdate (data_cref a) {
	// preparing request
	epp_EmailFwdUpdateReq_ref request(new epp_EmailFwdUpdateReq());
	request->m_cmd.ref(newCommand(a,"EU"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("add")) {
		data_cref add = a.get("add");
		request->m_add.ref(new epp_EmailFwdUpdateAddRemove());
		if (add.has("statuses")) request->m_add->m_status.ref(newEmailFwdStatusSeq(add.get("statuses").csplited()));
		if (add.hasAny("admin","tech","billing")) request->m_add->m_contacts.ref(newEmailFwdContactSeq(add));
	};
	if (a.has("remove")) {
		data_cref remove = a.get("remove");
		request->m_remove.ref(new epp_EmailFwdUpdateAddRemove());
		if (remove.has("statuses")) request->m_remove->m_status.ref(newEmailFwdStatusSeq(remove.get("statuses").csplited()));
		if (remove.hasAny("admin","tech","billing")) request->m_remove->m_contacts.ref(newEmailFwdContactSeq(remove));
	};
	if (a.has("change")) {
		data_cref change = a.get("change");
		request->m_change.ref(new epp_EmailFwdUpdateChange());
		if (change.has("fwdTo")) request->m_change->m_fwdto.ref(new epp_string(change.getLine("fwdTo")));
		if (change.has("password")) request->m_change->m_auth_info.ref(newAuthInfo(change));
		if (change.has("registrant")) request->m_change->m_registrant.ref(new epp_string(change.getLine("registrant")));
	};
	// performing command
	epp_EmailFwdUpdate_ref command(new epp_EmailFwdUpdate());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdUpdateRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::emailFwdDelete (data_cref a) {
	// preparing request
	epp_EmailFwdDeleteReq_ref request(new epp_EmailFwdDeleteReq());
	request->m_cmd.ref(newCommand(a,"ED"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	// performing command
	epp_EmailFwdDelete_ref command(new epp_EmailFwdDelete());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdDeleteRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp);
};

data_type EPP::emailFwdTransfer (data_cref a) {
	// preparing request
	epp_EmailFwdTransferReq_ref request(new epp_EmailFwdTransferReq());
	request->m_cmd.ref(newCommand(a,"ET"));
	request->m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("period")) request->m_period.ref(new epp_EmailFwdPeriod(eppobject::emailFwd::YEAR,a.get("period").toIntN()));
	request->m_trans.ref(new epp_TransferRequest());
	try {
		request->m_trans->m_op.ref(new epp_TransferOpType(returnTransferType(a.getLine("op"))));
	} catch (const epp_XMLException &ex) {
		printf("epp_XMLException!!!\n\n");
		return errorResult(9999,ex.getString());
	};
	if (a.has("password")) request->m_trans->m_auth_info.ref(newAuthInfo(a));
	// performing command
	epp_EmailFwdTransfer_ref command(new epp_EmailFwdTransfer());
	command->setRequestData(*request);
	data_type err = safeProcessAction(command);
	if (err.notNull()) return err;
	// getting response
	epp_EmailFwdTransferRsp_ref r = command->getResponseData();
	return readResponseData(r->m_rsp)+readDomainTrnData(r->m_trn_data);
};

// SMART COMMANDS

void_type EPP::pollAck (data_cref id) {
	poll(data_type(
		"op",	"ack",
		"id",	id
	));
};

data_type EPP::pollOne (data_cref a) {
	data_type res = poll(a);
	if (res.has("msgQ_id")) pollAck(res.get("msgQ_id"));
	return res;
};

data_type EPP::pollAll (data_cref a) {
	data_type p = poll(a);
	if (!p.has("msgQ_id")) return data_null;
	data_type res(1,false);
	do {
		res.push(p);
		pollAck(p.get("msgQ_id"));
		p = poll(a);
	} while (p.has("msgQ_id"));
	return res;
};

data_type EPP::domainMassCheck (data_cref a) {
	data_cnst names = a.get("names").csplited();
	data_type res(names.size()+4,false);
	size_type k = 0;
	line_type list;
	for (size_type i=0,n=names.size()-1;i<=n;i++) {
		list += (k ? "," : "")+names.getLine(i);
		k++;
		if (k==5 || i==n) {
			res += domainSimpleCheck(data_type("names",list));
			k = 0;
			list.clear();
		};
	};
	return res;
};

data_type EPP::domainSmartCheck (data_cref a) {
	data_cnst names = a.get("names").csplited();
	bool_type check = true;
	data_type res;
	data_type b = a;
	for (size_type i=0,n=names.size();i<n;i++) {
		line_type name = names.getLine(i);
		b.set("names",name);
		data_type h = domainCheck(b);
		if (h.has(name) && !h.get(name).get("avail").toBool()) check = false;
		res.inc(h);
	};
	res.set("check",check);
	return res;
};

data_type EPP::diffOldNew2AddRem (data_cref old_k,data_cref new_k) {
	data_type rem_k,add_k;
	for (size_type i=0,n=old_k.size();i<n;i++) if (!new_k.has(old_k.key(i))) rem_k.set(old_k.key(i),data_null);
	for (size_type i=0,n=new_k.size();i<n;i++) if (!old_k.has(new_k.key(i))) add_k.set(new_k.key(i),data_null);
	return data_type(add_k,rem_k);
};

data_type EPP::domainSmartUpdate (data_cref a,data_cref info) {
	data_type r = a;
	data_type old_s = checkClientStatuses(info.get("statuses").ksplited());
	bool_type old_u = old_s.del("clientUpdateProhibited");
	bool_type new_u = old_u;
	if (r.has("statuses")) {
		data_type new_s = checkClientStatuses(r.pop("statuses").ksplited());
		new_u = new_s.del("clientUpdateProhibited");
		data_type dif_s = diffOldNew2AddRem(old_s,new_s);
		data_type add_s = dif_s.get(0);
		data_type rem_s = dif_s.get(1);
		if (rem_s.size()) r.let("remove").set("statuses",rem_s);
		if (add_s.size()) r.let("add").set("statuses",add_s);
	};
	if (r.has("nameservers")) {
		r.set("nameservers",r.get("nameservers").lced());
		data_type old_n = info.get("nameservers").ksplited();
		data_type new_n = r.pop("nameservers").ksplited();
		data_type dif_n = diffOldNew2AddRem(old_n,new_n);
		data_type add_n = dif_n.get(0);
		data_type rem_n = dif_n.get(1);
		if (rem_n.size()) r.let("remove").set("nameservers",rem_n);
		if (add_n.size()) {
			r.let("add").set("nameservers",add_n);
			data_type h = hostSmartCheck(data_type("ext",getExt(a),"names",add_n.keys()));
			for (size_type i=0,n=add_n.size();i<n;i++) {
				if (h.get(add_n.key(i)).get("avail").toBool()) hostCreate(data_type("ext",getExt(a),"name",add_n.key(i)));
			};
		};
	};
	if (r.hasAny("admin","tech","billing")) {
		char_cptr cons[] = {"admin","tech","billing"};
		for (size_type i=0,n=sizeof(cons)/sizeof(*cons);i<n;i++) {
			if (r.has(cons[i])) {
				data_type old_c = info.get(cons[i]).ksplited();
				data_type new_c = r.pop(cons[i]).ksplited();
				data_type dif_c = diffOldNew2AddRem(old_c,new_c);
				data_type add_c = dif_c.get(0);
				data_type rem_c = dif_c.get(1);
				if (rem_c.size()) r.let("remove").set(cons[i],rem_c.keys());
				if (add_c.size()) r.let("add").set(cons[i],add_c.keys());
			};
		};
	};
	if (a.has("registrant")) {
		line_type reg = r.pop("registrant").toLine();
		if (reg!=info.getLine("registrant")) r.let("change").set("registrant",reg);
	};
	if (a.has("new_password")) {
		line_type pwd = r.pop("new_password").toLine();
		if (pwd!=info.getLine("password")) r.let("change").set("password",pwd);
	};
	if (r.hasAny("add","remove","change")) {
		if (old_u) domainAllowUpdate(a);
		if (new_u) r.let("add").let("statuses").set("clientUpdateProhibited",data_null);
		data_type res = domainUpdate(r);
		if (!isResponseOk(res) && new_u) domainProhibitUpdate(a);
		return res;
	} else if (old_u && !new_u) {
		return domainAllowUpdate(a);
	} else if (new_u && !old_u) {
		return domainProhibitUpdate(a);
	} else return info;
};

data_type EPP::domainSmartDelete (data_cref a) {
	data_type p = domainDelete(a);
	return isResponseOk(p) ? p : domainSmartDelete(a,domainInfo(a));
};

data_type EPP::domainSmartDelete (data_cref a,data_cref info) {
	if (info.getLine("statuses").find("pendingDelete")!=line_npos) return info;
	data_type r("name",a.get("name"));
	if (info.getLine("statuses").find("clientUpdateProhibited")!=line_npos) domainAllowUpdate(a);
	if (info.getLine("statuses").find("clientDeleteProhibited")!=line_npos) r.let("remove").set("statuses","clientDeleteProhibited");
	if (info.getLine("nameservers").find(a.getLine("name"))!=line_npos) r.let("remove").set("nameservers",info.getLine("nameservers"));
	if (r.has("remove")) domainUpdate(r);
	data_type p = domainDelete(a);
	if (isResponseOk(p)) return p;
	data_type hosts = csplit(info.getLine("hosts"));
	for (size_type i=0,n=hosts.size();i<n;i++) hostSmartDelete(data_type("name",hosts.getLine(i)));
	return domainDelete(a);
};

data_type EPP::domainSmartLock (data_cref a,data_cref info) {
	data_type old_s = info.get("statuses").ksplited();
	data_type new_s = old_s;
	char_cptr stats[] = {"clientUpdateProhibited","clientDeleteProhibited","clientTransferProhibited"};
	size_type go = 0;
	for (size_type i=0,n=sizeof(stats)/sizeof(*stats);i<n;i++) if (!old_s.has(stats[i])) {
		new_s.set(stats[i],data_null);
		go++;
	};
	return go ? domainSmartUpdate(data_type("name",a.getLine("name"),"statuses",new_s),info) : info;
};

data_type EPP::domainSmartUnlock (data_cref a,data_cref info) {
	data_type old_s = info.get("statuses").ksplited();
	data_type new_s(old_s.size(),true);
	data_type stats = ksplit(line_stat("clientUpdateProhibited,clientDeleteProhibited,clientTransferProhibited"));
	size_type go = 0;
	for (size_type i=0,n=old_s.size();i<n;i++) {
		if (stats.has(old_s.key(i))) go++;
		else new_s.set(old_s.key(i),data_null);
	};
	return go ? domainSmartUpdate(data_type("name",a.getLine("name"),"statuses",new_s),info) : info;
};

data_type EPP::domainSmartHold (data_cref a,data_cref info) {
	data_type o_s = info.get("statuses").ksplited();
	if (o_s.has("clientHold")) return info;
	o_s.set("clientHold",data_null);
	return domainSmartUpdate(data_type("name",a.getLine("name"),"statuses",o_s),info);
};

data_type EPP::domainSmartUnhold (data_cref a,data_cref info) {
	data_type o_s = info.get("statuses").ksplited();
	if (!o_s.has("clientHold")) return info;
	o_s.del("clientHold");
	return domainSmartUpdate(data_type("name",a.getLine("name"),"statuses",o_s),info);
};

data_type EPP::hostSmartCheck (data_cref a) {
	data_cnst names = a.get("names").csplited();
	bool_type check = true;
	data_type res;
	data_type b = a;
	for (size_type i=0,n=names.size();i<n;i++) {
		line_type name = names.getLine(i);
		b.set("names",name);
		data_type h = hostCheck(b);
		if (h.has(name) && !h.get(name).get("avail").toBool()) check = false;
		res.inc(h);
	};
	res.set("check",check);
	return res;
};

data_type EPP::hostSmartUpdate (data_cref a,data_cref info) {
	data_type r = a;
	data_type old_s = checkClientStatuses(info.get("statuses").ksplited());
	bool_type old_u = old_s.del("clientUpdateProhibited");
	bool_type new_u = old_u;
	if (r.has("statuses")) {
		data_type new_s = checkClientStatuses(r.pop("statuses").ksplited());
		new_u = new_s.del("clientUpdateProhibited");
		data_type dif_s = diffOldNew2AddRem(old_s,new_s);
		data_type add_s = dif_s.get(0);
		data_type rem_s = dif_s.get(1);
		if (rem_s.size()) r.let("remove").set("statuses",rem_s);
		if (add_s.size()) r.let("add").set("statuses",add_s);
	};
	if (r.has("ips")) {
		data_type old_i = info.get("ips").ksplited();
		data_type new_i = r.pop("ips").ksplited();
		data_type dif_i = diffOldNew2AddRem(old_i,new_i);
		data_type add_i = dif_i.get(0);
		data_type rem_i = dif_i.get(1);
		if (rem_i.size()) r.let("remove").set("ips",rem_i);
		if (add_i.size()) r.let("add").set("ips",add_i);
	};
	if (r.has("new_name")) {
		line_type name = r.pop("new_name").toLine();
		if (name!=info.getLine("name")) r.let("change").set("name",name);
	};
	if (r.hasAny("add","remove","change")) {
		if (old_u) hostAllowUpdate(a);
		if (new_u) r.let("add").let("statuses").set("clientUpdateProhibited",data_null);
		data_type res = hostUpdate(r);
		if (!isResponseOk(res) && new_u) hostProhibitUpdate(a);
		return res;
	} else if (old_u && !new_u) {
		return hostAllowUpdate(a);
	} else if (new_u && !old_u) {
		return hostProhibitUpdate(a);
	} else return info;
};

data_type EPP::hostSmartDelete (data_cref a) {
	data_type p = hostDelete(a);
	return isResponseOk(p) ? p : hostSmartRename(a.getLine("name"));
};

data_type EPP::hostRename (line_cref oldn,line_cref newn) {
	return hostUpdate(data_type("name",oldn,"change",data_type("name",newn)));
};

data_type EPP::hostSmartRename (line_cref oldn,line_cref pfix) {
	data_type p;
	for (size_type i=0;i<10;i++) {
		line_type newn = oldn+(i ? "-"+size2line(i)+pfix : pfix);
		hostDelete(data_type("name",newn));
		p = hostRename(oldn,newn);
		if (isResponseOk(p)) return p;
	};
	return p;
};

data_type EPP::domainSmartRenew (data_cref a) {
	data_type r = a;
	data_type i = domainInfo(a);
	line_type act_e = i.getLine("expiration_date").substr(0,10);
	line_type req_e = r.getLine("expires").substr(0,10);
	if (req_e!=act_e) {
		if (req_e.size()) {
			// XXX TODO atol -> line2intN
			r.set("period",(intN_type)(r.getIntN("period")+atol(req_e.substr(0,4).c_str())-atol(act_e.substr(0,4).c_str())));
		};
		r.set("expires",act_e);
	};
	return r.getIntN("period")>0 ? domainRenew(r) : i;
};

data_type EPP::emailFwdSmartRenew (data_cref a) {
	data_type r = a;
	if (!r.hasNotNull("expires")) r.set("expires",emailFwdInfo(a).getLine("expiration_date").substr(0,10));
	return emailFwdRenew(r);
};

data_type EPP::hostSmartSet (data_cref a,data_type info) {
	if (!isResponseOk(info)) {
		data_type r = hostCreate(a);
		if (!isResponseOk(r)) return r;
		info = r+a;
	};
	return hostSmartUpdate(a,info);
};

data_type EPP::contactSmartCheck (data_cref a) {
	data_cnst ids = a.get("ids").csplited();
	bool_type check = true;
	data_type res;
	data_type b = a;
	for (size_type i=0,n=ids.size();i<n;i++) {
		line_type id = ids.getLine(i);
		b.set("ids",id);
		data_type h = contactCheck(b);
		if (h.has(id) && !h.get(id).get("avail").toBool()) check = false;
		res.inc(h);
	};
	res.set("check",check);
	return res;
};

data_type EPP::contactSmartUpdate (data_cref a,data_cref info) {
	data_type r = a;
	data_type old_s = checkClientStatuses(info.get("statuses").ksplited());
	bool_type old_u = old_s.del("clientUpdateProhibited");
	bool_type new_u = old_u;
	if (r.has("statuses")) {
		data_type new_s = checkClientStatuses(r.pop("statuses").ksplited());
		new_u = new_s.del("clientUpdateProhibited");
		data_type dif_s = diffOldNew2AddRem(old_s,new_s);
		data_type add_s = dif_s.get(0);
		data_type rem_s = dif_s.get(1);
		if (rem_s.size()) r.let("remove").set("statuses",rem_s);
		if (add_s.size()) r.let("add").set("statuses",add_s);
	};
	if (hasContactData(r)) {
		char_cptr vars[] = {
			"name","organization","email","password",
			"street1","street2","street3","city","province","postal_code","country",
			"voice_phone","voice_extension","fax_phone","fax_extension"
		};
		for (size_type i=0,n=sizeof(vars)/sizeof(*vars);i<n;i++) {
			if (r.has(vars[i])) {
				line_type val = r.pop(vars[i]).toLine();
				if (val!=info.getLine(vars[i])) r.let("change").set(vars[i],val);
			};
		};
		if (hasContactAddress(r.get("change"))) {
			data_vref c = r.let("change");
			char_cptr vars[] = {"street1","street2","street3","city","province","postal_code","country"};
			for (size_type i=0,n=sizeof(vars)/sizeof(*vars);i<n;i++) if (!c.has(vars[i])) c.set(vars[i],info.get(vars[i]));
		};
	};
	if (r.hasAny("add","remove","change")) {
		if (old_u) contactAllowUpdate(a);
		if (new_u) r.let("add").let("statuses").set("clientUpdateProhibited",data_null);
		data_type res = contactUpdate(r);
		if (!isResponseOk(res) && new_u) contactProhibitUpdate(a);
		return res;
	} else if (old_u && !new_u) {
		return contactAllowUpdate(a);
	} else if (new_u && !old_u) {
		return contactProhibitUpdate(a);
	} else return info;
};

data_type EPP::contactSmartSet (data_cref a,data_type info) {
	if (!isResponseOk(info)) {
		data_type r = contactSmartCreate(a);
		if (!isResponseOk(r)) return r;
		info = r+a;
	};
	return contactSmartUpdate(a,info);
};

// EPP auxiliary functions

data_type EPP::safeProcessAction (epp_Action_ref command) {
	try {
		session.processAction(command);
		return data_null;
	} catch (const epp_XMLException &ex) {
		printf("epp_XMLException!!!\n\n");
		return data_type(
			"result_code",		9999,
			"result_msg",		line_type(ex.getString())
		);
	} catch (const epp_Exception &ex) {
		printf("epp_Exception!!!\n\n");
		std::cout << ex << std::endl;
		printf("epp_Exception!!!\n\n");
		return readResultsData(ex.m_details)+readTransIDData(ex.m_trans_id);
	};
};

bool_type EPP::isDomainUpdatable (data_cref statuses) {
	char_cptr vars[] = {"pendingTransfer","clientUpdateProhibited","serverUpdateProhibited"};
	for (size_type i=0,n=sizeof(vars)/sizeof(*vars);i<n;i++) if (statuses.has(vars[i])) return false;
	return true;
};

bool_type EPP::isResponseOk (data_cref r) {
	return r.getIntN("result_code")<2000;
};

// RTK auxiliary functions

void_type EPP::setNamestoreExtension (line_cref ext,line_cref data) {
	epp_NamestoreExt_ref e(new epp_NamestoreExt());
	e->setRequestData(epp_NamestoreExtData(data.size() ? data : "dot"+uc(ext.substr(1))));
	extensions.set(ext,e);
};

epp_string_seq *EPP::newStringSeq (data_cref a) {
	epp_string_seq *res = new epp_string_seq();
	for (size_type i=0,n=a.size();i<n;i++) res->push_back(a.gk(i).toLine());
	return res;
};

epp_AuthInfo *EPP::newAuthInfo (data_cref a) {
	epp_AuthInfo *res = new epp_AuthInfo();
	res->m_type.ref(new epp_AuthInfoType(PW));
	res->m_value.ref(new epp_string(a.getLine("password")));
	return res;
};

epp_PollOpType *EPP::newPollOpType (line_cref t) {
	epp_PollOpType type = REQ;
	if ("ack"==t) type = ACK;
	return new epp_PollOpType(type);
};

epp_DomainHostsType *EPP::newDomainHostsType (line_cref t) {
	epp_DomainHostsType type = eppobject::domain::ALL;
	if ("none"==t) type = eppobject::domain::NONE;
	else if ("del"==t) type = eppobject::domain::DEL;
	else if ("sub"==t) type = eppobject::domain::SUB;
	return new epp_DomainHostsType(type);
};

void_type EPP::addDomainContacts (epp_domain_contact_seq *seq,line_cref type,data_cref ids) {
	for (size_type i=0,n=ids.hashSize();i<n;i++) {
		epp_DomainContact res;
		res.m_type.ref(new epp_DomainContactType(eppobject::domain::returnContactEnumType(type)));
		res.m_id.ref(new epp_string(ids.gk(i).toLine()));
		seq->push_back(res);
	};
};

epp_domain_contact_seq *EPP::newDomainContactSeq (data_cref a) {
	epp_domain_contact_seq *res = new epp_domain_contact_seq();
	char_cptr cons[] = {"admin","tech","billing"};
	for (size_type i=0,n=sizeof(cons)/sizeof(*cons);i<n;i++) {
		if (a.has(cons[i])) addDomainContacts(res,cons[i],a.get(cons[i]).csplited());
	};
	return res;
};

epp_DomainStatus EPP::DomainStatus (data_cref a) {
	epp_DomainStatus res;
	res.m_type.ref(new epp_DomainStatusType(eppobject::domain::returnStatusEnumType(statusType(a))));
	if (a.has("value")) res.m_value.ref(new epp_string(a.getLine("value")));
	if (a.has("lang")) res.m_lang.ref(new epp_string(a.getLine("lang")));
	return res;
};

epp_domain_status_seq *EPP::newDomainStatusSeq (data_cref a) {
	data_type s = checkClientStatuses(a);
	epp_domain_status_seq *res = new epp_domain_status_seq();
	for (size_type i=0,n=s.size();i<n;i++) res->push_back(DomainStatus(s.gk(i)));
	return res;
};

void_type EPP::addEmailFwdContacts (epp_emailFwd_contact_seq *seq,line_cref type,data_cref ids) {
	for (size_type i=0,n=ids.hashSize();i<n;i++) {
		epp_EmailFwdContact res;
		res.m_type.ref(new epp_EmailFwdContactType(eppobject::emailFwd::returnContactEnumType(type)));
		res.m_id.ref(new epp_string(ids.gk(i).toLine()));
		seq->push_back(res);
	};
};

epp_emailFwd_contact_seq *EPP::newEmailFwdContactSeq (data_cref a) {
	epp_emailFwd_contact_seq *res = new epp_emailFwd_contact_seq();
	char_cptr cons[] = {"admin","tech","billing"};
	for (size_type i=0,n=sizeof(cons)/sizeof(*cons);i<n;i++) {
		if (a.has(cons[i])) addEmailFwdContacts(res,cons[i],a.get(cons[i]).csplited());
	};
	return res;
};

epp_EmailFwdStatus EPP::EmailFwdStatus (data_cref a) {
	epp_EmailFwdStatus res;
	res.m_type.ref(new epp_EmailFwdStatusType(eppobject::emailFwd::returnStatusEnumType(statusType(a))));
	if (a.has("value")) res.m_value.ref(new epp_string(a.getLine("value")));
	if (a.has("lang")) res.m_lang.ref(new epp_string(a.getLine("lang")));
	return res;
};

epp_emailFwd_status_seq *EPP::newEmailFwdStatusSeq (data_cref a) {
	data_type s = checkClientStatuses(a);
	epp_emailFwd_status_seq *res = new epp_emailFwd_status_seq();
	for (size_type i=0,n=s.size();i<n;i++) res->push_back(EmailFwdStatus(s.gk(i)));
	return res;
};

epp_HostStatus EPP::HostStatus (data_cref a) {
	epp_HostStatus res;
	res.m_type.ref(new epp_HostStatusType(eppobject::host::returnStatusEnumType(statusType(a))));
	if (a.has("value")) res.m_value.ref(new epp_string(a.getLine("value")));
	if (a.has("lang")) res.m_lang.ref(new epp_string(a.getLine("lang")));
	return res;
};

epp_ContactStatus EPP::ContactStatus (data_cref a) {
	epp_ContactStatus res;
	res.m_type.ref(new epp_ContactStatusType(eppobject::contact::returnStatusEnumType(statusType(a))));
	if (a.has("value")) res.m_value.ref(new epp_string(a.getLine("value")));
	if (a.has("lang")) res.m_lang.ref(new epp_string(a.getLine("lang")));
	return res;
};

data_type EPP::checkClientStatuses (data_cref a) {
	data_type res;
	for (size_type i=0,n=a.size();i<n;i++) {
		line_type status = a.gk(i).toLine();
		if (isClientStatus(status)) res.set(status,data_null);
	};
	return res;
};

epp_host_status_seq *EPP::newHostStatusSeq (data_cref a) {
	data_type s = checkClientStatuses(a);
	epp_host_status_seq *res = new epp_host_status_seq();
	for (size_type i=0,n=s.size();i<n;i++) res->push_back(HostStatus(s.gk(i)));
	return res;
};

epp_contact_status_seq *EPP::newContactStatusSeq (data_cref a) {
	data_type s = checkClientStatuses(a);
	epp_contact_status_seq *res = new epp_contact_status_seq();
	for (size_type i=0,n=s.size();i<n;i++) res->push_back(ContactStatus(s.gk(i)));
	return res;
};

epp_host_address_seq *EPP::newHostAddressSeq (data_cref a) {
	epp_host_address_seq *res = new epp_host_address_seq();
	for (size_type i=0,n=a.size();i<n;i++) {
		line_type ip = a.gk(i).toLine();
		epp_HostAddress address(ip.find(".")==line_npos ? IPV6 : IPV4,epp_string(ip));
		res->push_back(address);
	};
	return res;
};

epp_ContactAddress *EPP::newContactAddress (data_cref a) {
	epp_ContactAddress *res = new epp_ContactAddress();
	if (a.has("street1")) res->m_street1.ref(new epp_string(a.getLine("street1")));
	if (a.has("street2")) res->m_street2.ref(new epp_string(a.getLine("street2")));
	if (a.has("street3")) res->m_street3.ref(new epp_string(a.getLine("street3")));
	if (a.has("city")) res->m_city.ref(new epp_string(a.getLine("city")));
	if (a.has("province")) res->m_state_province.ref(new epp_string(a.getLine("province")));
	if (a.has("postal_code")) res->m_postal_code.ref(new epp_string(a.getLine("postal_code")));
	if (a.has("country")) res->m_country_code.ref(new epp_string(a.getLine("country")));
	return res;
};

epp_ContactNameAddress EPP::ContactNameAddress (data_cref a) {
	epp_ContactNameAddress res;
	res.m_type.ref(new epp_ContactPostalInfoType(a.getLine("postal_info_type")=="LOC" ? LOC : INT));
	if (a.has("name")) res.m_name.ref(new epp_string(a.getLine("name")));
	if (a.has("organization")) res.m_org.ref(new epp_string(a.getLine("organization")));
	if (hasContactAddress(a)) res.m_address.ref(newContactAddress(a));
	return res;
};

epp_ContactNameAddress_seq *EPP::newContactNameAddressSeq (data_cref a) {
	epp_ContactNameAddress_seq *res = new epp_ContactNameAddress_seq();
	res->push_back(ContactNameAddress(a));
	return res;
};

epp_ContactPhone *EPP::newContactPhone (data_cref p,data_cref e) {
	epp_ContactPhone *res = new epp_ContactPhone();
	res->m_value.ref(new epp_string(p.toLine()));
	if (e.notNull()) res->m_extension.ref(new epp_string(e.toLine()));
	return res;
};

data_type EPP::readDomainTrnData (const epp_PollResData_ref &p) {
	if (p==NULL) return data_null;
	if (p->getType()!="domain:trnData") return data_null;
	const epp_DomainTrnData_ref &t = p;
	data_type res("poll_type","domain");
	if (t->m_name!=NULL) res["name"] = lc(*t->m_name);
	if (t->m_transfer_status!=NULL) res["transfer_status"] = returnTransferStatusType(*t->m_transfer_status);
	if (t->m_request_client_id!=NULL) res["request_client_id"] = *t->m_request_client_id;
	if (t->m_request_date!=NULL) res["request_date"] = *t->m_request_date;
	if (t->m_action_client_id!=NULL) res["action_client_id"] = *t->m_action_client_id;
	if (t->m_action_date!=NULL) res["action_date"] = *t->m_action_date;
	if (t->m_expiration_date!=NULL) res["expiration_date"] = *t->m_expiration_date;
	return res;
};

data_type EPP::readContactTrnData (const epp_PollResData_ref &p) {
	if (p==NULL) return data_null;
	if (p->getType()!="contact:trnData") return data_null;
	const epp_ContactTrnData_ref &t = p;
	data_type res("poll_type","contact");
	if (t->m_id!=NULL) res["id"] = *t->m_id;
	if (t->m_transfer_status!=NULL) res["transfer_status"] = returnTransferStatusType(*t->m_transfer_status);
	if (t->m_request_client_id!=NULL) res["request_client_id"] = *t->m_request_client_id;
	if (t->m_request_date!=NULL) res["request_date"] = *t->m_request_date;
	if (t->m_action_client_id!=NULL) res["action_client_id"] = *t->m_action_client_id;
	if (t->m_action_date!=NULL) res["action_date"] = *t->m_action_date;
	return res;
};

data_type EPP::readLowBalancePollData (const epp_PollResData_ref &p) {
	if (p==NULL || p->getType()!="lowbalance-poll:pollData") return data_null;
	const epp_LowBalancePollResData_ref &t = p;
	data_type res("poll_type","lowbalance");
	if (t->m_registrar_name!=NULL) res["registrar_name"] = *t->m_registrar_name;
	if (t->m_credit_limit!=NULL) res["credit_limit"] = *t->m_credit_limit;
	if (t->m_available_credit!=NULL) res["available_credit"] = *t->m_available_credit;
	if (t->m_threshold_type!=NULL) res["threshold_type"] = *t->m_threshold_type;
	if (t->m_threshold_value!=NULL) res["threshold_value"] = *t->m_threshold_value;
	return res;
};

data_type EPP::readRGPPollData (const epp_PollResData_ref &p) {
	if (p==NULL || p->getType()!="rgp-poll:pollData") return data_null;
	const epp_RGPPollResData_ref &t = p;
	data_type res("poll_type","rgp");
	if (t->m_name!=NULL) res["name"] = *t->m_name; // domain name that is candidate for restoration
	if (t->m_status!=NULL) res["status"] = returnRGPStatusType(*t->m_status); // RGP status of the domain
	if (t->m_req_date!=NULL) res["req_date"] = *t->m_req_date; // date the server is requesting the client's restore report
	if (t->m_report_due_date!=NULL) res["report_due_date"] = *t->m_report_due_date; // date the client's restore report must be received by the server
	return res;
};

data_type EPP::readTransIDData (const epp_TransID_ref &t) {
	data_type res;
	if (t!=NULL) {
		if (t->m_server_trid!=NULL) res["server_trid"] = *t->m_server_trid;
		if (t->m_client_trid!=NULL) res["client_trid"] = *t->m_client_trid;
	};
	return res;
};

data_type EPP::readMessageQueueData (const epp_MessageQueue_ref &m) {
	data_type res;
	if (m!=NULL) {
		if (m->m_count!=NULL) res["msgQ_count"] = (int32_t)*m->m_count;
		if (m->m_id!=NULL) res["msgQ_id"] = *m->m_id;
		if (m->m_msg!=NULL) res["msgQ_message"] = *m->m_msg->m_value;
		if (m->m_queue_date!=NULL) res["msgQ_date"] = *m->m_queue_date;
	};
	return res;
};

data_type EPP::readResultsData (const epp_result_seq_ref &r) {
	if (r==NULL) return data_null;
	data_type res;
	for (epp_result_seq::iterator i=r->begin();i!=r->end();i++) {
		if (i->m_msg!=NULL) {
			data_type &s = res.push(data_type("result_msg",*i->m_msg));
			if (i->m_code!=NULL) s["result_code"] = *i->m_code;
			if (i->m_lang!=NULL) s["result_lang"] = *i->m_lang;
		};
	};
	if (res.size()==1) res = res.first();
	return res;
};

data_type EPP::readResponseData (const epp_Response_ref &r) {
	return r==NULL ? data_null : readResultsData(r->m_results)+readMessageQueueData(r->m_message_queue)+readTransIDData(r->m_trans_id);
};

}; // namespace re
