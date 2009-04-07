// (c) Andrii Vasyliev
// rePPMod

#include "rePPMod.h"

reHash<rePP> rePPMod::sessions;

reValue rePPMod::init (const reValue &a) {
	sessions.set(a.gl("session",0),rePP(a.gl("host"),a.gi4("port"),a.gl("certificate"),a.gl("serial")));
	return reValue::Null;
};

reValue rePPMod::incBatchNo (const reValue &a) { return sessions.let(a.gl("session",0)).incBatchNo(); };

reValue rePPMod::setNamestoreExtension (const reValue &a) {
	sessions.let(a.gl("session",0)).setNamestoreExtension(a.gl("ext"),a.gl("data"));
	return reValue::Null;
};

// EPP NATIVE COMMANDS

reValue rePPMod::poll			(const reValue &a) { return sessions.let(a.gl("session",0)).poll(a); };
reValue rePPMod::hello			(const reValue &a) { return sessions.let(a.gl("session",0)).hello(a); };
reValue rePPMod::login			(const reValue &a) { return sessions.let(a.gl("session",0)).login(a); };
reValue rePPMod::logout			(const reValue &a) { return sessions.let(a.gl("session",0)).logout(a); };
reValue rePPMod::domainInfo		(const reValue &a) { return sessions.let(a.gl("session",0)).domainInfo(a); };
reValue rePPMod::domainSync		(const reValue &a) { return sessions.let(a.gl("session",0)).domainSync(a); };
reValue rePPMod::domainCheck		(const reValue &a) { return sessions.let(a.gl("session",0)).domainCheck(a); };
reValue rePPMod::domainRenew		(const reValue &a) { return sessions.let(a.gl("session",0)).domainRenew(a); };
reValue rePPMod::domainCreate		(const reValue &a) { return sessions.let(a.gl("session",0)).domainCreate(a); };
reValue rePPMod::domainUpdate		(const reValue &a) { return sessions.let(a.gl("session",0)).domainUpdate(a); };
reValue rePPMod::domainDelete		(const reValue &a) { return sessions.let(a.gl("session",0)).domainDelete(a); };
reValue rePPMod::domainTransfer		(const reValue &a) { return sessions.let(a.gl("session",0)).domainTransfer(a); };
reValue rePPMod::hostInfo		(const reValue &a) { return sessions.let(a.gl("session",0)).hostInfo(a); };
reValue rePPMod::hostCheck		(const reValue &a) { return sessions.let(a.gl("session",0)).hostCheck(a); };
reValue rePPMod::hostCreate		(const reValue &a) { return sessions.let(a.gl("session",0)).hostCreate(a); };
reValue rePPMod::hostUpdate		(const reValue &a) { return sessions.let(a.gl("session",0)).hostUpdate(a); };
reValue rePPMod::hostDelete		(const reValue &a) { return sessions.let(a.gl("session",0)).hostDelete(a); };
reValue rePPMod::contactCheck		(const reValue &a) { return sessions.let(a.gl("session",0)).contactCheck(a); };

// SMART COMMANDS

reValue rePPMod::pollOne		(const reValue &a) { return sessions.let(a.gl("session",0)).pollOne(a); };
reValue rePPMod::pollAll		(const reValue &a) { return sessions.let(a.gl("session",0)).pollAll(a); };
reValue rePPMod::domainAllowUpdate	(const reValue &a) { return sessions.let(a.gl("session",0)).domainAllowUpdate(a); };
reValue rePPMod::domainProhibitUpdate	(const reValue &a) { return sessions.let(a.gl("session",0)).domainProhibitUpdate(a); };
reValue rePPMod::domainSmartCheck	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSmartCheck(a); };
reValue rePPMod::domainSmartUpdate	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSmartUpdate(a); };
reValue rePPMod::domainSmartLock	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSmartLock(a); };
reValue rePPMod::domainSmartUnlock	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSmartUnlock(a); };
reValue rePPMod::domainSmartHold	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSmartHold(a); };
reValue rePPMod::domainSmartUnhold	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSmartUnhold(a); };
reValue rePPMod::hostSmartCheck		(const reValue &a) { return sessions.let(a.gl("session",0)).hostSmartCheck(a); };
reValue rePPMod::hostSmartUpdate	(const reValue &a) { return sessions.let(a.gl("session",0)).hostSmartUpdate(a); };
reValue rePPMod::hostSmartSet		(const reValue &a) { return sessions.let(a.gl("session",0)).hostSmartSet(a); };

