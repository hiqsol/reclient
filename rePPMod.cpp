// (c) Andrii Vasyliev
// rePPMod

#include "rePPMod.h"

reHash<rePP> rePPMod::sessions;

reValue rePPMod::init (const reValue &a) {
	sessions.set(a.gl("session",0),rePP(a.gl("host"),a.gi4("port"),a.gl("certificate")));
	return reValue::Null;
};

reValue rePPMod::logout (const reValue &a) {
	reMod::set(reValue("REPP_LOGGEDOUT","YES"));
	return sessions.let(a.gl("session",0)).logout(a);
};

reValue rePPMod::poll		(const reValue &a) { return sessions.let(a.gl("session",0)).poll(a); };
reValue rePPMod::hello		(const reValue &a) { return sessions.let(a.gl("session",0)).hello(a); };
reValue rePPMod::login		(const reValue &a) { return sessions.let(a.gl("session",0)).login(a); };
reValue rePPMod::domainInfo	(const reValue &a) { return sessions.let(a.gl("session",0)).domainInfo(a); };
reValue rePPMod::domainSync	(const reValue &a) { return sessions.let(a.gl("session",0)).domainSync(a); };
reValue rePPMod::domainCheck	(const reValue &a) { return sessions.let(a.gl("session",0)).domainCheck(a); };
reValue rePPMod::domainRenew	(const reValue &a) { return sessions.let(a.gl("session",0)).domainRenew(a); };
reValue rePPMod::domainCreate	(const reValue &a) { return sessions.let(a.gl("session",0)).domainCreate(a); };
reValue rePPMod::domainUpdate	(const reValue &a) { return sessions.let(a.gl("session",0)).domainUpdate(a); };
reValue rePPMod::domainDelete	(const reValue &a) { return sessions.let(a.gl("session",0)).domainDelete(a); };
reValue rePPMod::domainTransfer	(const reValue &a) { return sessions.let(a.gl("session",0)).domainTransfer(a); };
reValue rePPMod::hostInfo	(const reValue &a) { return sessions.let(a.gl("session",0)).hostInfo(a); };
reValue rePPMod::hostCheck	(const reValue &a) { return sessions.let(a.gl("session",0)).hostCheck(a); };
reValue rePPMod::hostCreate	(const reValue &a) { return sessions.let(a.gl("session",0)).hostCreate(a); };
reValue rePPMod::hostUpdate	(const reValue &a) { return sessions.let(a.gl("session",0)).hostUpdate(a); };
reValue rePPMod::hostDelete	(const reValue &a) { return sessions.let(a.gl("session",0)).hostDelete(a); };
reValue rePPMod::contactCheck	(const reValue &a) { return sessions.let(a.gl("session",0)).contactCheck(a); };

