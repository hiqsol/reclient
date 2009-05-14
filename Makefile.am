LDADD =	\
	-lreScript -lreParsing -lreTypes -lreUtils \
	-L$(COMNETADDON_PATH)/lib -lcomnetaddon -lcomnetdata \
	-L$(EPPRTK_PATH)/lib -lepprtk -ldata -ltransport -lnpssl \
	-L$(DOMTOOLS_PATH)/lib -ldomtools \
	$(XERCES_LIB) $(SSL_LIBS) \
	$(LLVM_LDADD)

bin_PROGRAMS = reClient
reClient_SOURCES = reClient.cpp rePP.cpp rePPMod.cpp
reClient_LDFLAGS = -rdynamic

check_PROGRAMS = testPP
testPP_SOURCES = testPP.cpp rePP.cpp
