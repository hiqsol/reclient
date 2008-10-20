// (c) Andrii Vasyliev
// rePPMod

#ifndef __RE_PP_MOD_H__
#define __RE_PP_MOD_H__

#include "rePP.h"
#include "rebase/reScript/reMod.h"
#include "rebase/reTypes/reHash.h"

class rePPMod : public reMod {
public:
	rePPMod () : reMod("rePPMod","init,poll,hello,login,logout,domainInfo,domainSync,domainCheck,domainRenew,domainCreate,domainUpdate,domainDelete,domainTransfer,hostInfo,hostCheck,hostCreate,hostUpdate,hostDelete,contactCheck") {};

	static reValue init		(const reValue &a);
	static reValue poll		(const reValue &a);
	static reValue hello		(const reValue &a);
	static reValue login		(const reValue &a);
	static reValue logout		(const reValue &a);
	static reValue domainInfo	(const reValue &a);
	static reValue domainSync	(const reValue &a);
	static reValue domainCheck	(const reValue &a);
	static reValue domainRenew	(const reValue &a);
	static reValue domainCreate	(const reValue &a);
	static reValue domainUpdate	(const reValue &a);
	static reValue domainDelete	(const reValue &a);
	static reValue domainTransfer	(const reValue &a);
	static reValue hostInfo		(const reValue &a);
	static reValue hostCheck	(const reValue &a);
	static reValue hostCreate	(const reValue &a);
	static reValue hostUpdate	(const reValue &a);
	static reValue hostDelete	(const reValue &a);
	static reValue contactCheck	(const reValue &a);

// PROPERTIES
private:
	static reHash<rePP> sessions;
};

#endif//__RE_PP_MOD_H__
