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

int main (int argc,char *argv[]) {
	// INITIALIZING reScript
	reScript script;
	reMod base;
	rePPMod repp;
	script.add(&base,"");
	script.add(&repp,"rePP");

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
	reValue dirs;
	dirs[requestDir] = 1;
	dirs[reportDir] = 1;
	dirs[storeDir] = 1;
	dirs[errorDir] = 1;
	if (dirs.size()<4) {
		printf("all dirs must differ!\n");
		exit(1);
	};
	printf("dirs ok: %s %s %s\n",requestDir.c_str(),reportDir.c_str(),storeDir.c_str());

	// INITIALIZING EPP CONNECTION
	script.runFunction("rePP:init",reValue(
		"host",		reMod::get("host"),
		"port",		reMod::get("port"),
		"certificate",	reMod::get("certificate")
	));
	reValue loginOptions = reValue(
		"username",	reMod::get("username"),
		"password",	reMod::get("password"),
		"newPassword",	reMod::get("newPassword")
	);
	script.runFunction("rePP:login",loginOptions);

	bool loggedIn = true;
	time_t logintime = timestamp();
	time_t lastlogin = logintime;
	time_t lasthello = logintime;
	// MAIN LOOP
	try {
		while (loggedIn) {
			time_t nowtime = timestamp();
			// PREVENTING IDLE-TIMEOUT (VeriSign - 10 minutes)
			if (nowtime-lasthello>60*8) {
				script.runFunction("rePP:hello");
				lasthello = nowtime;
				printf("HELLO\n");
			};
			// PREVENTING ABSOLUTE-TIMEOUT (VeriSign - 24 hours)
			if (nowtime-lastlogin>60*60*23) {
				script.runFunction("rePP:logout");
				script.runFunction("rePP:login",loginOptions);
				lastlogin = nowtime;
				lasthello = nowtime;
				printf("RELOGIN\n");
			};
			size_t num = 0;
			reValue files = csplit(reExec::backtick("ls -rt "+requestDir),"\n");
			//printf("files: %s\n",files.dump2line().c_str());
			for (reValue::size_type i=0,n=files.size();i<n;i++) {
				reLine file = files.gl(i);
				//if (!isdigits(file.c_str())) continue;
				reLine path = requestDir + file;
				printf("%s \n",path.c_str());
				reFile::writeln(reportDir+file,script.runFile(path).dump2line());
				if (reFile::rename(path,storeDir+file)) {
					printf("can't move %s\n",path.c_str());
					exit(5);
				};
				num++;
				if (reMod::get("REPP_LOGGEDOUT").toBool()) {
					loggedIn = false;
					break;
				};
			};
			if (!num) sleep(1);
		};
	} catch (const epp_TrException &ex) {
		std::cout << "Transport Exception: " << ex.getString() << std::endl;
		exit(1);
	} catch (const epp_XMLException &ex) {
		std::cout << "XML Exception: " << ex.getString() << std::endl;
		exit(1);
	} catch (const epp_Exception &ex) {
		std::cout << "EPP Exception: " << std::endl;
		cerr << "<response>" << std::endl;
		printResultsSeq(ex.m_details);
		printTransID(ex.m_trans_id);
		cerr << "</response>" << std::endl;
		exit(1);
	} catch (...) {
		std::cout << "Other exception " << std::endl;
		exit(1);
	};
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

