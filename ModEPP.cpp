// (c) Andrii Vasyliev
// ModEPP

#include "reclient/ModEPP.h"

namespace re {

Hash<EPP> ModEPP::sessions;

data_type ModEPP::init (data_cref a) {
	sessions.set(a.getLine("session",0),EPP(a.getLine("host"),a.getIntN("port"),a.getLine("certificate"),a.getLine("cacertfile"),a.getLine("cacertdir")));
	if (a.has("serial")) sessions.let(a.getLine("session",0)).setSerialNo(a.getLine("serial"));
	return data_null;
};

data_type ModEPP::incBatchNo (data_cref a) { return sessions.let(a.getLine("session",0)).incBatchNo(); };

data_type ModEPP::setNamestoreExtension (data_cref a) {
	sessions.let(a.getLine("session",0)).setNamestoreExtension(a.getLine("ext"),a.getLine("data"));
	return data_null;
};

// EPP NATIVE COMMANDS
data_type ModEPP::poll			(data_cref a) { return sessions.let(a.getLine("session",0)).poll(a); };
data_type ModEPP::hello			(data_cref a) { return sessions.let(a.getLine("session",0)).hello(a); };
data_type ModEPP::login			(data_cref a) { return sessions.let(a.getLine("session",0)).login(a); };
data_type ModEPP::logout		(data_cref a) { return sessions.let(a.getLine("session",0)).logout(a); };
data_type ModEPP::domainInfo		(data_cref a) { return sessions.let(a.getLine("session",0)).domainInfo(a); };
data_type ModEPP::domainSync		(data_cref a) { return sessions.let(a.getLine("session",0)).domainSync(a); };
data_type ModEPP::domainCheck		(data_cref a) { return sessions.let(a.getLine("session",0)).domainCheck(a); };
data_type ModEPP::domainRenew		(data_cref a) { return sessions.let(a.getLine("session",0)).domainRenew(a); };
data_type ModEPP::domainCreate		(data_cref a) { return sessions.let(a.getLine("session",0)).domainCreate(a); };
data_type ModEPP::domainUpdate		(data_cref a) { return sessions.let(a.getLine("session",0)).domainUpdate(a); };
data_type ModEPP::domainDelete		(data_cref a) { return sessions.let(a.getLine("session",0)).domainDelete(a); };
data_type ModEPP::domainTransfer	(data_cref a) { return sessions.let(a.getLine("session",0)).domainTransfer(a); };
data_type ModEPP::emailFwdInfo		(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdInfo(a); };
data_type ModEPP::emailFwdCheck		(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdCheck(a); };
data_type ModEPP::emailFwdRenew		(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdRenew(a); };
data_type ModEPP::emailFwdCreate	(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdCreate(a); };
data_type ModEPP::emailFwdUpdate	(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdUpdate(a); };
data_type ModEPP::emailFwdDelete	(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdDelete(a); };
data_type ModEPP::emailFwdTransfer	(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdTransfer(a); };
data_type ModEPP::hostInfo		(data_cref a) { return sessions.let(a.getLine("session",0)).hostInfo(a); };
data_type ModEPP::hostCheck		(data_cref a) { return sessions.let(a.getLine("session",0)).hostCheck(a); };
data_type ModEPP::hostCreate		(data_cref a) { return sessions.let(a.getLine("session",0)).hostCreate(a); };
data_type ModEPP::hostUpdate		(data_cref a) { return sessions.let(a.getLine("session",0)).hostUpdate(a); };
data_type ModEPP::hostDelete		(data_cref a) { return sessions.let(a.getLine("session",0)).hostDelete(a); };
data_type ModEPP::contactCheck		(data_cref a) { return sessions.let(a.getLine("session",0)).contactCheck(a); };
data_type ModEPP::contactCreate		(data_cref a) { return sessions.let(a.getLine("session",0)).contactCreate(a); };
data_type ModEPP::contactInfo		(data_cref a) { return sessions.let(a.getLine("session",0)).contactInfo(a); };
data_type ModEPP::contactTransfer	(data_cref a) { return sessions.let(a.getLine("session",0)).contactTransfer(a); };
data_type ModEPP::contactUpdate		(data_cref a) { return sessions.let(a.getLine("session",0)).contactUpdate(a); };
data_type ModEPP::contactDelete		(data_cref a) { return sessions.let(a.getLine("session",0)).contactDelete(a); };

// SMART COMMANDS
data_type ModEPP::pollOne		(data_cref a) { return sessions.let(a.getLine("session",0)).pollOne(a); };
data_type ModEPP::pollAll		(data_cref a) { return sessions.let(a.getLine("session",0)).pollAll(a); };
data_type ModEPP::domainAllowUpdate	(data_cref a) { return sessions.let(a.getLine("session",0)).domainAllowUpdate(a); };
data_type ModEPP::domainProhibitUpdate	(data_cref a) { return sessions.let(a.getLine("session",0)).domainProhibitUpdate(a); };
data_type ModEPP::domainSmartCheck	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartCheck(a); };
data_type ModEPP::domainSmartUpdate	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartUpdate(a); };
data_type ModEPP::domainSmartDelete	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartDelete(a); };
data_type ModEPP::domainSmartLock	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartLock(a); };
data_type ModEPP::domainSmartUnlock	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartUnlock(a); };
data_type ModEPP::domainSmartHold	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartHold(a); };
data_type ModEPP::domainSmartUnhold	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartUnhold(a); };
data_type ModEPP::domainSmartRenew	(data_cref a) { return sessions.let(a.getLine("session",0)).domainSmartRenew(a); };
data_type ModEPP::emailFwdSmartRenew	(data_cref a) { return sessions.let(a.getLine("session",0)).emailFwdSmartRenew(a); };
data_type ModEPP::hostSmartCheck	(data_cref a) { return sessions.let(a.getLine("session",0)).hostSmartCheck(a); };
data_type ModEPP::hostSmartUpdate	(data_cref a) { return sessions.let(a.getLine("session",0)).hostSmartUpdate(a); };
data_type ModEPP::hostSmartDelete	(data_cref a) { return sessions.let(a.getLine("session",0)).hostSmartDelete(a); };
data_type ModEPP::hostSmartSet		(data_cref a) { return sessions.let(a.getLine("session",0)).hostSmartSet(a); };
data_type ModEPP::contactSmartCheck	(data_cref a) { return sessions.let(a.getLine("session",0)).contactSmartCheck(a); };
data_type ModEPP::contactSmartUpdate	(data_cref a) { return sessions.let(a.getLine("session",0)).contactSmartUpdate(a); };
data_type ModEPP::contactSmartSet	(data_cref a) { return sessions.let(a.getLine("session",0)).contactSmartSet(a); };

}; // namespace re
