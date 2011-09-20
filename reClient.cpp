// (c) Andrii Vasyliev
// reClient - Registrar EPP Client

#include "reclient/ModEPP.h"
#include "rebase/Script/ModBase.h"
#include "rebase/Script/Script.h"
#include "rebase/Sys/Exec.h"
#include "rebase/Sys/File.h"
#include "epp-rtk-cpp/data/epp_Exception.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"
#include "epp-rtk-cpp/data/epp_PollResFactory.h"
#include "comnetaddon/data/epp_LowBalancePollResData.h"
#include "comnetaddon/data/epp_RGPPollResData.h"
#include <sys/time.h>
#include <signal.h>

using namespace re;

// GLOBAL VARIABLES
size_type THE_SNO;
size_type THE_PID;
line_type THE_EXT;
line_type THE_DAY;
line_type THE_LOG;
FILE *    LOGFILE;

line_type checkDir (line_type dir) {
	if (!File::isDir(dir)) {
		printf("%s is not a dir\n",dir.c_str());
		exit(2);
	};
	if (dir[dir.size()-1]!='/') dir += "/";
	return dir;
};

time_t timestamp () {
	struct timeval tp;
	gettimeofday(&tp,NULL);
	return tp.tv_sec;
};

void writelog (const char *s) {
	time_t n = timestamp();
	tm *t = localtime(&n);
	fprintf(LOGFILE,"%04d-%02d-%02d %02d:%02d:%02d %s\n",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec,s);
	fflush(LOGFILE);
};
void writelog (line_cref s) { writelog(s.c_str()); };

void sigalrm (int a) {
	//printf("catched\n");
};

int main (int argc,char *argv[]) {
	RE_COMPILER_MINUSE();

	// SETTING HANDLER FOR SIGNAL
	signal(SIGALRM,sigalrm);

	// INITIALIZING Script
	Script script;
	ModBase base;
	ModEPP repp;
	script.add(&base,"");
	script.add(&repp,"repp");

	// GETTING CONFIG
	if (argc<2 || !File::isFile(argv[1])) {
		printf("usage: reClient configfile\n");
		exit(1);
	};
	script.runFile(argv[1]);
	line_type requestDir	= checkDir(ModBase::getLine("requestDir"));
	line_type reportDir	= checkDir(ModBase::getLine("reportDir"));
	line_type storeDir	= checkDir(ModBase::getLine("storeDir"));
	line_type errorDir	= checkDir(ModBase::getLine("errorDir"));
	line_type logDir	= checkDir(ModBase::getLine("logDir"));
	data_type dirs;
	dirs[requestDir]	= 1;
	dirs[reportDir]		= 1;
	dirs[storeDir]		= 1;
	dirs[errorDir]		= 1;
	dirs[logDir]		= 1;
	if (dirs.size()<5) {
		printf("all dirs must differ!\n");
		exit(1);
	};
	printf("dirs ok: %s %s %s\n",requestDir.c_str(),reportDir.c_str(),storeDir.c_str());
	bool_type doSerialize	= ModBase::get("doSerialize").toBool();

	// INITIALIZING LOG
	data_type lastlog = csplit(chomp(Exec::backtick("ls -t "+logDir+" | head -n 1")),".");
	size_type lastpid = lastlog.getIntN(2);
	if (lastpid && !kill(lastpid,0)) {
		printf("Already running\n");
		exit(1);
	};
	THE_SNO = lastlog.getIntN(1)+1;
	THE_PID = getpid();
	THE_EXT = "."+size2line(THE_SNO);
	THE_DAY = chomp(Exec::backtick("date '+%Y-%m-%d'"));
	THE_LOG = logDir+THE_DAY+THE_EXT+'.'+size2line(THE_PID);
	LOGFILE = fopen(THE_LOG.c_str(),"w");
	if (!LOGFILE) {
		printf("can't open log: %s\n",THE_LOG.c_str());
		exit(1);
	};
	printf("LastLog:%s LastPID:%i THE_SNO:%i' \n",lastlog.getLine(0).c_str(),lastpid,THE_SNO);
	writelog("opened");
	File::writeln(logDir+"pid",size2line(THE_PID));

	// INITIALIZING EPP
	script.runFunc("repp.init",data_type(
		"host",		ModBase::get("host"),
		"port",		ModBase::get("port"),
		"certificate",	ModBase::get("certificate"),
		"cacertfile",	ModBase::get("cacertfile"),
		"cacertdir",	ModBase::get("cacertdir"),
		"serial",	THE_SNO
	));
	if (ModBase::has("namestores").toBool()) {
		data_type namestores = csplit(ModBase::get("namestores"));
		for (size_type i=0,n=namestores.size();i<n;i++) {
			script.runFunc("repp.setNamestoreExtension",data_type(
				"ext",		"."+namestores.get(i),
				"data",		"dot"+uc(namestores.get(i))
			));
		};
	};
	data_type loginOptions = data_type(
		"username",	ModBase::get("username"),
		"password",	ModBase::get("password")
	);
	if (ModBase::has("new_password").toBool()) loginOptions.set("new_password",ModBase::get("new_password"));
	if (ModBase::has("login_trID").toBool()) loginOptions.set("trID",ModBase::get("login_trID"));
	script.runFunc("repp.login",loginOptions);
	writelog("inited, loggedin");
	// Register poll response handler
	epp_PollResFactory::addClass("lowbalance-poll","pollData",eppobject::lowbalancePoll::createLowBalancePollResData);
	epp_PollResFactory::addClass("rgp-poll","pollData",eppobject::rgpPoll::createRGPPollResData);

	bool_type loggedIn = true;
	time_t logintime = timestamp();
	time_t lastlogin = logintime;
	time_t lasthello = logintime;
	// MAIN LOOP
	while (loggedIn) {
		time_t nowtime = timestamp();
		// PREVENTING ABSOLUTE-TIMEOUT (VeriSign - 24 hours)
		if (nowtime-lastlogin>60*60*23) {
			script.runFunc("repp.logout");
			script.runFunc("repp.login",loginOptions);
			lastlogin = nowtime;
			lasthello = nowtime;
			writelog("relogined");
		};
		// PREVENTING IDLE-TIMEOUT (VeriSign - 10 minutes)
		if (nowtime-lasthello>60*8) {
			script.runFunc("repp.hello");
			lasthello = nowtime;
			writelog("hello");
		};
		size_type num = 0;
		data_type files = csplit(Exec::backtick("ls -rt "+requestDir),"\n");
		for (size_type i=0,n=files.size();i<n;i++) {
			line_type file = files.getLine(i);
			line_type path = requestDir + file;
			line_type b_no = script.runFunc("repp.incBatchNo").toLine();
			line_type rnam = size2line(THE_SNO)+'-'+b_no+'.'+file;
			data_type res = script.runFile(path);
			line_type lres = doSerialize ? Script::data2text(res) : res.dump2line();
			File::writeln(reportDir+file,lres);
			File::appendln(path,"\n\n"+lres);
			bool_type is_ok = EPP::isResponseOk(res);
			writelog(rnam+" "+(is_ok ? "ok" : "error"));
			line_type save = storeDir+rnam;
			if (File::rename(path,save)) {
				printf("can't move %s to %s\n",path.c_str(),save.c_str());
				exit(5);
			};
			if (!is_ok) Exec::system("cp "+save+" "+errorDir+rnam);
			num++;
			if (ModBase::get("REPP_LOGGEDOUT").toBool()) {
				printf("\nLogged OUT\n");
				loggedIn = false;
				break;
			};
		};
		if (!num) sleep(1);
		else lasthello = nowtime;
	};
	fclose(LOGFILE);
	return 0;
};

