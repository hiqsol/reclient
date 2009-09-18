// (c) Andrii Vasyliev
// test rePP

#include "rePP.h"
#include "epp-rtk-cpp/data/epp_Exception.h"
#include "epp-rtk-cpp/data/epp_XMLException.h"
#include "epp-rtk-cpp/testTools.h"

using std::cout;
using std::cerr;
using std::endl;

int main () {
	rePP session("epp-ote.verisign-grs.com",700,"certificate.pem");
	try {
		session.logIn(reValue("username","evoplus1","password","2L9%NX@5%"));
		//reValue ch = session.contactCheck(reValue("ids","asdf,asdf,s"));
		//cout << ch.dump2line() << endl;
		reValue dh = session.domainCheck(reValue("names","example.com,my-ex4.com,advancedhosters.com,evonames.com"));
		cout << dh.dump2line() << endl;
		//reValue dc = session.domainCreate(reValue("name","my-ex5.com","nameservers","ns1.my-example-i3.com,ns2.my-example-i3.com","period",3,"password","my-pw5"));
		//cout << dc.dump2line() << endl;
		/*
		reValue du = session.domainUpdate(reValue(
			"name",		"my-ex5.com",
			"add",		reValue("nameservers","ns1.my-ex4.com"),
			"remove",	reValue("namespaces","ns1.my-example-i3.com"),
			"change",	reValue("password","newpassword")
		));
		cout << du.dump2line() << endl;
		*/
		reValue di = session.domainInfo(reValue("name","my-ex4.com","type","all"));
		cout << di.dump2line() << endl;
		/*
		reValue hh = session.hostCheck(reValue("names","ns1.example.com,ns3.my-example-i3.com,ns1.my-example-i3.com,ns1.asdfsdafd.net,sadfsd.ua_1"));
		cout << hh.dump2line() << endl;
		reValue hi = session.hostInfo(reValue("name","ns1.my-example-i3.com"));
		cout << hi.dump2line() << endl;
		*/
		session.logOut();
	} catch (const epp_TrException &ex) {
		cout << "Transport Exception: " << ex.getString() << endl;
		exit(1);
	} catch (const epp_XMLException &ex) {
		cout << "XML Exception: " << ex.getString() << endl;
		exit(1);
	} catch (const epp_Exception &ex) {
		cout << "EPP Exception: " << endl;
		cerr << "<response>" << endl;
		printResultsSeq(ex.m_details);
		printTransID(ex.m_trans_id);
		cerr << "</response>" << endl;
		exit(1);
	} catch (...) {
		cout << "Other exception " << endl;
		exit(1);
	};
	return 0;
};
