// (c) Andrii Vasyliev
// reBomber - 

#include "reclient/EPP.h"
#include "rebase/Sys/Exec.h"
#include "rebase/Sys/File.h"
#include "rebase/Parsing/ConfigParser.h"
#include "rebase/Parsing/Reader.h"
#include "epp-rtk-cpp/data/epp_Exception.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"
#include "epp-rtk-cpp/data/epp_PollResFactory.h"
#include "comnetaddon/data/epp_LowBalancePollResData.h"
#include "comnetaddon/data/epp_RGPPollResData.h"
#include <sys/time.h>
#include <signal.h>

using namespace re;

void sigalrm (int a) {
	//printf("catched\n");
};

data_type parseConfig (line_cref path) {
	Reader r(path);
	return ConfigParser::parseConfig(r.c_str());
};

#define MAXDOMS 4096

data_type domains;
size_type domainNum;
size_type domainSum;
size_type domainWts[MAXDOMS] = {0};
size_type domainSnt[MAXDOMS] = {0};

void_type updateDomains (data_cref doms) {
	domainNum = doms.size();
	assert(domainNum<MAXDOMS);
	domainSum = 0;
	for (size_type i=0;i<domainNum;i++) {
		size_type wts = doms.getIntN(i);
		domainWts[i] = wts;
		domainSum += wts;
	};
};

size_type selectDomain () {
	size_type r = random() % domainSum;
	size_type i = 0;
	size_type s = 0;
	while (1) {
		s += domainWts[i];
		if (r<s) return i;
		i++;
	};
};

int main (int argc,char *argv[]) {
	// SETTING HANDLER FOR SIGNAL
	signal(SIGALRM,sigalrm);
	srandomdev();

	// READING CONFIG
	if (argc<2 || !File::isFile(argv[1])) errx(1,"usage: reBomber configfile");
	data_type config = parseConfig(argv[1]);
	printf("config: %s\n",config.dump2line().c_str());
	line_type regDomains = config.getLine("regDomains");
	size_type regNum = config.getIntN("regNum");
	if (!regNum) regNum = 10;
	size_type wait = config.getIntN("wait");
	if (!wait) wait = 100;

	// INITIALIZING EPP
	EPP session(config.getLine("host"),config.getIntN("port"),config.getLine("certificate"));
	session.setSerialNo("1");
	session.setNamestoreExtension(".com","dotCOM");
	session.setNamestoreExtension(".net","dotNET");
	session.login(data_type(
		"username",	config.getLine("username"),
		"password",	config.getLine("password")
	));

	// PREPARING REGISTRATION REQUEST
	data_type regRequest("password",config.getLine("regPassword"));
	line_type regRegistrant	= config.getLine("regRegistrant");
	if (regRegistrant.size()) {
		line_type regAdmin		= config.getLine("regAdmin");
		line_type regTech		= config.getLine("regTech");
		line_type regBilling		= config.getLine("regBilling");
		regRequest.set("registrant",	regRegistrant);
		regRequest.set("admin",		regAdmin.size()   ? regAdmin   : regRegistrant);
		regRequest.set("tech",		regTech.size()    ? regTech    : regRegistrant);
		regRequest.set("billing",	regBilling.size() ? regBilling : regRegistrant);
	};

	//time_t last = 0;
	//time_t curr = 0;
	// MAIN LOOP
	size_type count = 0;
	bool_type readDomains = false;
	bool_type dropStats  = false;
	while (1) {
		// CHECKING WHAT TO DO
		struct stat st;
		int rs = stat(regDomains.c_str(),&st);
		if (rs) {
			//fprintf(stderr,"couldn't open domains file: %s\n",regDomains.c_str());
			sleep(1);
			continue;
		};
		//curr = st.st_mtime.tv_sec;
		//if (curr>last) {
		if (count>0) count = 0;
		if (count==0) readDomains = true;
		count++;
		if (readDomains) dropStats = true;

		// DROPPING STATS
		if (dropStats) {
			for (size_type i=0;i<domainNum;i++) if (domainSnt[i]) {
				fprintf(stderr,"%-50s %i\n",domains.key(i),domainSnt[i]);
				domainSnt[i] = 0;
			};
			dropStats = false;
		};
		// READING DOMAINS LIST
		if (readDomains) {
			//last = curr;
			domains = parseConfig(regDomains);
			//fprintf(stderr,"domains: %s\n",domains.dump2line().c_str());
			updateDomains(domains);
			readDomains = false;
		};

		for (size_type i=0;i<regNum;i++) {
			// DETERMINING DOMAIN
			size_t n = selectDomain();
			line_type domain = domains.key(n);
			domainSnt[n]++;
			// SENDING REQUEST
			regRequest.set("name",domain);
			data_type regResult = session.domainCreate(regRequest);
			line_type regCode = regResult.getLine("result_code");
			/*
				fprintf(stderr,"%i: '%s'\n",n,domain.c_str());
				fprintf(stderr,"req: %s",regRequest.dump2line().c_str());
				fprintf(stderr,"%s: %s\n",domain.c_str(),regResult.dump2line().c_str());
			*/
			if (strcmp(regCode.c_str(),"2302"))
				fprintf(stderr,"%s: %s\n",domain.c_str(),regCode.c_str());
		};
		sleep(wait);
	};
	return 0;
};

