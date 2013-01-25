LDADD =	\
	libeppextensions.a \
	-lreScript -lreParsing -lreTypes -lreSys -lreDB -lpq \
	-L$(NAMEADDON_PATH)/lib -lnameaddon -lnamedata \
	-L$(COMNETADDON_PATH)/lib -lcomnetaddon -lcomnetdata \
	-L$(EPPRTK_PATH)/lib -lepprtk -ldata -ltransport -lnpssl \
	-L$(DOMTOOLS_PATH)/lib -ldomtools \
	$(XERCES_LIB) $(SSL_LIBS) \
	$(LLVM_LDADD)

bin_PROGRAMS = reClient reBomber
reClient_SOURCES = reClient.cpp EPP.cpp ModEPP.cpp
reClient_LDFLAGS = -rdynamic

reBomber_SOURCES = reBomber.cpp EPP.cpp

check_PROGRAMS = testEPP
testEPP_SOURCES = testEPP.cpp EPP.cpp

lib_LIBRARIES = libeppextensions.a
libeppextensions_a_SOURCES = DomainTrademark.cc
