LDADD =	\
	-lreScript -lreParsing -lreTypes -lreUtils \
	-L$(COMNETADDON_PATH)/lib -lcomnetaddon -lcomnetdata \
	-L$(EPPRTK_PATH)/lib -lepprtk -ldata -ltransport -lnpssl \
	-L$(DOMTOOLS_PATH)/lib -ldomtools \
	$(XERCES_LIB) $(SSL_LIBS) \
	-lpthread -lm -lelf -lLLVMipo /usr/local/lib/LLVMX86.o -lLLVMSelectionDAG /usr/local/lib/LLVMExecutionEngine.o /usr/local/lib/LLVMJIT.o -lLLVMCodeGen -lLLVMScalarOpts -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMCore -lLLVMSupport -lLLVMSystem

bin_PROGRAMS = reClient
reClient_SOURCES = reClient.cpp rePP.cpp rePPMod.cpp
reClient_LDFLAGS = -rdynamic

check_PROGRAMS = testPP
testPP_SOURCES = testPP.cpp rePP.cpp
