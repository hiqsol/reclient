// (c) Andrii Vasyliev
// rePPMod

#ifndef __RE_PP_MOD_H__
#define __RE_PP_MOD_H__

#include "rePP.h"
#include "rebase/reScript/reMod.h"
#include "rebase/reTypes/reHash.h"

class rePPMod : public reMod {
public:
	rePPMod () : reMod("rePPMod",
		"incBatchNo,setNamestoreExtension,"
		",init,hello,login,logout,poll"
		",domainInfo,domainSync,domainCheck,domainRenew,domainCreate,domainUpdate,domainDelete,domainTransfer"
		",hostInfo,hostCheck,hostCreate,hostUpdate,hostDelete"
		",contactCheck,contactCreate,contactInfo,contactTransfer,contactUpdate,contactDelete"
		",pollOne,pollAll"
		",domainAllowUpdate,domainProhibitUpdate"
		",domainSmartCheck,domainSmartUpdate,domainSmartLock,domainSmartUnlock,domainSmartHold,domainSmartUnhold,domainSmartRenew"
		",hostSmartCheck,hostSmartUpdate,hostSmartSet"
		",contactSmartCheck,contactSmartUpdate,contactSmartSet"
	) {};

// 
	static reValue init			(const reValue &a);
	static reValue incBatchNo		(const reValue &a);
	static reValue setNamestoreExtension	(const reValue &a);

// NATIVE EPP COMMANDS
	static reValue poll			(const reValue &a);
	static reValue hello			(const reValue &a);
	static reValue login			(const reValue &a);
	static reValue logout			(const reValue &a);
	static reValue domainInfo		(const reValue &a);
	static reValue domainSync		(const reValue &a);
	static reValue domainCheck		(const reValue &a);
	static reValue domainRenew		(const reValue &a);
	static reValue domainCreate		(const reValue &a);
	static reValue domainUpdate		(const reValue &a);
	static reValue domainDelete		(const reValue &a);
	static reValue domainTransfer		(const reValue &a);
	static reValue hostInfo			(const reValue &a);
	static reValue hostCheck		(const reValue &a);
	static reValue hostCreate		(const reValue &a);
	static reValue hostUpdate		(const reValue &a);
	static reValue hostDelete		(const reValue &a);
	static reValue contactCheck		(const reValue &a);
	static reValue contactCreate		(const reValue &a);
	static reValue contactInfo		(const reValue &a);
	static reValue contactTransfer		(const reValue &a);
	static reValue contactUpdate		(const reValue &a);
	static reValue contactDelete		(const reValue &a);

// SMART COMMANDS
	static reValue pollOne			(const reValue &a);
	static reValue pollAll			(const reValue &a);
	static reValue domainAllowUpdate	(const reValue &a);
	static reValue domainProhibitUpdate	(const reValue &a);
	static reValue domainSmartCheck		(const reValue &a);
	static reValue domainSmartUpdate	(const reValue &a);
	static reValue domainSmartLock		(const reValue &a);
	static reValue domainSmartUnlock	(const reValue &a);
	static reValue domainSmartHold		(const reValue &a);
	static reValue domainSmartUnhold	(const reValue &a);
	static reValue domainSmartRenew		(const reValue &a);
	static reValue hostSmartCheck		(const reValue &a);
	static reValue hostSmartUpdate		(const reValue &a);
	static reValue hostSmartSet		(const reValue &a);
	static reValue contactSmartCheck	(const reValue &a);
	static reValue contactSmartUpdate	(const reValue &a);
	static reValue contactSmartSet		(const reValue &a);

// PROPERTIES
private:
	static reHash<rePP> sessions;
};

#endif//__RE_PP_MOD_H__
