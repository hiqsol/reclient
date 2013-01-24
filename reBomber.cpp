// (c) Andrii Vasyliev
// reBomber - 

#include "reclient/EPP.h"
#include "rebase/Sys/Exec.h"
#include "rebase/Sys/File.h"
#include "rebase/Parsing/ConfigParser.h"
#include "rebase/Parsing/Reader.h"
#include "rebase/DB/PgDB.h"
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

size_type getRandom (size_type num) {
	return random() % num;
}

time_t getTime () {
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec;
};

#define MAXDOMS 4096

data_type domains;
size_type domainNum = 0;
size_type domainSum = 0;
size_type domainWts[MAXDOMS] = {0};
size_type domainSnt[MAXDOMS] = {0};

void_type updateDomains (data_cref doms) {
	domainNum = doms.size();
	assert(domainNum<MAXDOMS);
	domainSum = 0;
	for (size_type i=0;i<domainNum;i++) {
		domainSum += doms.getIntN(i);
		domainWts[i] = domainSum;
	};
};

size_type selectDomain () {
	assert(domainSum>0);
	size_type r = getRandom(domainSum);
	for (size_type i=0;r<domainNum;i++) if (r<domainWts[i]) return i;
	return 0;
};

int main (int argc,char *argv[]) {
	// SETTING HANDLER FOR SIGNAL
	signal(SIGALRM,sigalrm);
	srandomdev();

	// READING CONFIG
	if (argc<2 || !File::isFile(argv[1])) errx(1,"usage: reBomber configfile");
	data_type config = parseConfig(argv[1]);
	printf("config: %s\n",config.dump2line().c_str());
	time_t wait = config.has("wait") ? config.getUnsN("wait") : 3;
	time_t jitt = config.has("wait_jitter") ? config.getUnsN("wait_jitter") : wait;

	// INITIALIZING DB
	PgDB db(config.get("db"));

	// INITIALIZING EPP
	EPP session(
		config.getLine("host"),config.getIntN("port"),config.getLine("certificate"),
		config.getLine("cacertfile"),config.getLine("cacertdir")
	);
	session.setSerialNo("1");
	session.setNamestoreExtension(".com","dotCOM");
	session.setNamestoreExtension(".net","dotNET");
	session.login(data_type(
		"username",	config.getLine("username"),
		"password",	config.getLine("password")
	));

	// PREPARING THE REQUEST
	line_type regPassword = chomp(Exec::backtick(config.getLine("passgen")));
	data_type regRequest("password",regPassword);
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

	// MAIN LOOP
	time_t last = 0;
	while (1) {
		// DETERMINING WHAT TO BOMB if enough time has passed since last check
		time_t curr = getTime();
		if (curr>last+wait+(time_t)getRandom(jitt)) {
			last = curr;
			if (domainNum) {
				line_type sql;
				const size_t buffsize = 4096;
				char buff[buffsize];
				for (size_type i=0;i<domainNum;i++) {
					size_type sent = domainSnt[i];
					if (sent) {
						snprintf(buff,buffsize,"INSERT INTO intercept_log (domain,num) VALUES ('%s',%d);\n",domains.key(i),sent);
					};
					sql += buff;
					domainSnt[i] = 0;
				};
				db.exec(sql);
			};
			domains = db.hash1("\
				SELECT		domain,sum\
				FROM		simple_intercepting_domain\
				WHERE		time=to_day()\
			");
			fprintf(stderr,"domains: %s\n",domains.dump2line().c_str());
			updateDomains(domains);
		};

		if (!domainNum) {
			sleep(getRandom(jitt));
			continue;
		};

		// SELECTING DOMAIN
		size_t n = selectDomain();
		domainSnt[n]++;
		line_type domain = domains.key(n);

		// SENDING REQUEST
		regRequest.set("name",domain);
		data_type regResult = session.domainCreate(regRequest);
		size_type regCode = regResult.getUnsN("result_code");
		if (regCode!=2302) fprintf(stderr,"GOT IT: %4u %4u %s\n",domainSnt[n],regCode,domain.c_str());
		//usleep(300000);
	};
	return 0;
};

