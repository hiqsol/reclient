// (c) Andrii Vasyliev
// reClient - Registrar EPP Client

#include "rePPMod.h"
#include "rebase/reScript/reBaseMod.h"
#include "rebase/reScript/reScript.h"
#include "rebase/reUtils/reExec.h"
#include "rebase/reUtils/reFile.h"
#include "epp-rtk-cpp/data/epp_Exception.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"
#include "epp-rtk-cpp/testTools.h"
#include <sys/time.h>
#include <signal.h>

// GLOBAL VARIABLES
size_t THE_SNO;
size_t THE_PID;
reLine THE_EXT;
reLine THE_DAY;
reLine THE_LOG;
FILE * LOGFILE;

reLine checkDir (reLine dir) {
	if (!reFile::isDir(dir)) {
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
void writelog (const reLine &s) { writelog(s.c_str()); };

int main (int argc,char *argv[]) {
	RE_COMPILER_MINUSE();
	// INITIALIZING reScript
	reScript script;
	reBaseMod base;
	rePPMod repp;
	script.add(&base,"");
	script.add(&repp,"repp");

	// GETTING CONFIG
	if (argc<2 || !reFile::isFile(argv[1])) {
		printf("usage: reClient configfile\n");
		exit(1);
	};
	script.runFile(argv[1]);
	reLine requestDir	= checkDir(reBaseMod::gl("requestDir"));
	reLine reportDir	= checkDir(reBaseMod::gl("reportDir"));
	reLine storeDir		= checkDir(reBaseMod::gl("storeDir"));
	reLine errorDir		= checkDir(reBaseMod::gl("errorDir"));
	reLine logDir		= checkDir(reBaseMod::gl("logDir"));
	reValue dirs;
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
	bool doSerialize	= reBaseMod::get("doSerialize").toBool();

	// INITIALIZING LOG
	reValue lastlog = csplit(chomp(reExec::backtick("ls -t "+logDir+" | head -n 1")),".");
	size_t lastpid = lastlog.gi4(2);
	if (lastpid && !kill(lastpid,0)) {
		printf("Already running\n");
		exit(1);
	};
	printf("lastlog: '%s' '%i'\n",lastlog.gl(0).c_str(),lastpid);
	THE_SNO = lastlog.gi4(1)+1;
	THE_PID = getpid();
	THE_EXT = "."+u2line(THE_SNO);
	THE_DAY = chomp(reExec::backtick("date '+%Y-%m-%d'"));
	THE_LOG = logDir+THE_DAY+THE_EXT+'.'+i2line(THE_PID);
	LOGFILE = fopen(THE_LOG.c_str(),"w");
	if (!LOGFILE) {
		printf("can't open log: %s\n",THE_LOG.c_str());
		exit(1);
	};
	writelog("opened");

	// INITIALIZING EPP
	script.runFunc("repp.init",reValue(
		"host",		reBaseMod::get("host"),
		"port",		reBaseMod::get("port"),
		"certificate",	reBaseMod::get("certificate"),
		"cacertfile",	reBaseMod::get("cacertfile"),
		"cacertdir",	reBaseMod::get("cacertdir"),
		"serial",	THE_SNO
	));
	script.runFunc("repp.setNamestoreExtension",reValue(
		"ext",		".com",
		"data",		"dotCOM"
	));
	script.runFunc("repp.setNamestoreExtension",reValue(
		"ext",		".net",
		"data",		"dotNET"
	));
	reValue loginOptions = reValue(
		"username",	reBaseMod::get("username"),
		"password",	reBaseMod::get("password")
	);
	if (reBaseMod::has("new_password").toBool()) loginOptions.set("new_password",reBaseMod::get("new_password"));
	if (reBaseMod::has("login_trID").toBool()) loginOptions.set("trID",reBaseMod::get("login_trID"));
	script.runFunc("repp.login",loginOptions);
	writelog("inited, loggedin");

	bool loggedIn = true;
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
		size_t num = 0;
		reValue files = csplit(reExec::backtick("ls -rt "+requestDir),"\n");
		for (reValue::size_type i=0,n=files.size();i<n;i++) {
			reLine file = files.gl(i);
			reLine path = requestDir + file;
			reLine b_no = script.runFunc("repp.incBatchNo").toLine();
			reLine rnam = u2line(THE_SNO)+'-'+b_no+'.'+file;
			reValue res = script.runFile(path);
			reLine lres = doSerialize ? reScript::data2text(res) : res.dump2line();
			reFile::writeln(reportDir+file,lres);
			reFile::appendln(path,"\n\n"+lres);
			bool is_ok = rePP::isResponseOk(res);
			writelog(rnam+" "+(is_ok ? "ok" : "error"));
			reLine save = storeDir+rnam;
			if (reFile::rename(path,save)) {
				printf("can't move %s to %s\n",path.c_str(),save.c_str());
				exit(5);
			};
			if (!is_ok) reExec::system("cp "+save+" "+errorDir+rnam);
			num++;
			if (reBaseMod::get("REPP_LOGGEDOUT").toBool()) {
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

