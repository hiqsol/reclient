// (c) Andrii Vasyliev
// reClient - Registrar EPP Client

#include "rePPMod.h"
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
	// INITIALIZING reScript
	reScript script;
	reMod base;
	rePPMod repp;
	script.add(&base,"");
	script.add(&repp,"repp");

	// GETTING CONFIG
	if (argc<2 || !reFile::isFile(argv[1])) {
		printf("usage: reClient configfile\n");
		exit(1);
	};
	script.runFile(argv[1]);
	reLine requestDir	= checkDir(reMod::gl("requestDir"));
	reLine reportDir	= checkDir(reMod::gl("reportDir"));
	reLine storeDir		= checkDir(reMod::gl("storeDir"));
	reLine errorDir		= checkDir(reMod::gl("errorDir"));
	reLine logDir		= checkDir(reMod::gl("logDir"));
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
	script.runFunction("repp:init",reValue(
		"host",		reMod::get("host"),
		"port",		reMod::get("port"),
		"certificate",	reMod::get("certificate"),
		"cacertfile",	reMod::get("cacertfile"),
		"cacertdir",	reMod::get("cacertdir"),
		"serial",	THE_SNO
	));
	script.runFunction("repp:setNamestoreExtension",reValue(
		"ext",		".com",
		"data",		"dotCOM"
	));
	script.runFunction("repp:setNamestoreExtension",reValue(
		"ext",		".net",
		"data",		"dotNET"
	));
	reValue loginOptions = reValue(
		"username",	reMod::get("username"),
		"password",	reMod::get("password"),
		"newPassword",	reMod::get("newPassword")
	);
	script.runFunction("repp:login",loginOptions);
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
			script.runFunction("repp:logout");
			script.runFunction("repp:login",loginOptions);
			lastlogin = nowtime;
			lasthello = nowtime;
			writelog("relogined");
		};
		// PREVENTING IDLE-TIMEOUT (VeriSign - 10 minutes)
		if (nowtime-lasthello>60*8) {
			script.runFunction("repp:hello");
			lasthello = nowtime;
			writelog("hello");
		};
		size_t num = 0;
		reValue files = csplit(reExec::backtick("ls -rt "+requestDir),"\n");
		for (reValue::size_type i=0,n=files.size();i<n;i++) {
			reLine file = files.gl(i);
			reLine path = requestDir + file;
			reLine b_no = script.runFunction("repp:incBatchNo").toLine();
			reLine rnam = u2line(THE_SNO)+'-'+b_no+'.'+file;
			reValue res = script.runFile(path);
			reLine lres = res.dump2line();
			reFile::writeln(reportDir+file,lres);
			reFile::appendln(path,"\n\n"+lres);
			bool is_ok = rePP::isResponseOk(res);
			writelog(rnam+" "+(is_ok ? "ok" : "error"));
			reLine save = storeDir+rnam;
			if (reFile::rename(path,save)) {
				printf("can't move %s to %s\n",path.c_str(),save.c_str());
				exit(5);
			};
			if (!is_ok) reExec::spawn("cp "+save+" "+errorDir+rnam);
			num++;
			if (reMod::get("REPP_LOGGEDOUT").toBool()) {
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

void minUse () {
	reValue i = (int64_t)1;
	reValue n = reMod::set(reValue("a","b"));
	reValue a = reMod::get(reValue(1,false));
	a.set("asdasd",123);
	reMod::inc(n);
	reMod::sum(a);
};

