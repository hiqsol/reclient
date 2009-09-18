LDADD =	\
	-lreScript -lreParsing -lreTypes -lreSys \
	-L$(COMNETADDON_PATH)/lib -lcomnetaddon -lcomnetdata \
	-L$(EPPRTK_PATH)/lib -lepprtk -ldata -ltransport -lnpssl \
	-L$(DOMTOOLS_PATH)/lib -ldomtools \
	$(XERCES_LIB) $(SSL_LIBS) \
	$(LLVM_LDADD)

bin_PROGRAMS = reClient
reClient_SOURCES = reClient.cpp EPP.cpp ModEPP.cpp
reClient_LDFLAGS = -rdynamic

check_PROGRAMS = testEPP
testPP_SOURCES = testEPP.cpp EPP.cpp
