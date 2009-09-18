// (c) Andrii Vasyliev
// ModEPP

#ifndef __RE_MOD_EPP_H__
#define __RE_MOD_EPP_H__

#include "reclient/EPP.h"
#include "rebase/Script/Mod.h"
#include "rebase/Types/Hash.h"

namespace re {

class ModEPP : public Mod {
private: // PROPERTIES
	static Hash<EPP> sessions;
public:
	ModEPP () : Mod("ModEPP",
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

	static	data_type init			(data_cref a);
	static	data_type incBatchNo		(data_cref a);
	static	data_type setNamestoreExtension	(data_cref a);

// NATIVE EPP COMMANDS
	static	data_type poll			(data_cref a);
	static	data_type hello			(data_cref a);
	static	data_type login			(data_cref a);
	static	data_type logout		(data_cref a);
	static	data_type domainInfo		(data_cref a);
	static	data_type domainSync		(data_cref a);
	static	data_type domainCheck		(data_cref a);
	static	data_type domainRenew		(data_cref a);
	static	data_type domainCreate		(data_cref a);
	static	data_type domainUpdate		(data_cref a);
	static	data_type domainDelete		(data_cref a);
	static	data_type domainTransfer	(data_cref a);
	static	data_type hostInfo		(data_cref a);
	static	data_type hostCheck		(data_cref a);
	static	data_type hostCreate		(data_cref a);
	static	data_type hostUpdate		(data_cref a);
	static	data_type hostDelete		(data_cref a);
	static	data_type contactCheck		(data_cref a);
	static	data_type contactCreate		(data_cref a);
	static	data_type contactInfo		(data_cref a);
	static	data_type contactTransfer	(data_cref a);
	static	data_type contactUpdate		(data_cref a);
	static	data_type contactDelete		(data_cref a);

// SMART COMMANDS
	static	data_type pollOne		(data_cref a);
	static	data_type pollAll		(data_cref a);
	static	data_type domainAllowUpdate	(data_cref a);
	static	data_type domainProhibitUpdate	(data_cref a);
	static	data_type domainSmartCheck	(data_cref a);
	static	data_type domainSmartUpdate	(data_cref a);
	static	data_type domainSmartLock	(data_cref a);
	static	data_type domainSmartUnlock	(data_cref a);
	static	data_type domainSmartHold	(data_cref a);
	static	data_type domainSmartUnhold	(data_cref a);
	static	data_type domainSmartRenew	(data_cref a);
	static	data_type hostSmartCheck	(data_cref a);
	static	data_type hostSmartUpdate	(data_cref a);
	static	data_type hostSmartSet		(data_cref a);
	static	data_type contactSmartCheck	(data_cref a);
	static	data_type contactSmartUpdate	(data_cref a);
	static	data_type contactSmartSet	(data_cref a);
}; // class ModEPP

}; // namespace re

#endif//__RE_MOD_EPP_H__
