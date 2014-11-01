ObjSuf        = o
SrcSuf        = cc
ExeSuf        =
DllSuf        = so
OutPutOpt     = -o
HeadSuf       = h

#ROOFIT_INCLUDE := $(shell cd $(CMSSW_BASE); scram tool info roofitcore | grep INCLUDE= | sed 's|INCLUDE=||')

#ROOFIT_LIBDIR := $(shell cd $(CMSSW_BASE); scram tool info roofitcore | grep LIBDIR= | sed 's|LIBDIR=||')

#INCLUDES = -I$(ROOFIT_INCLUDE)/ -I$(CMSSW_BASE)/src/JetMETCorrections/GammaJetFilter/bin/ -I$(boost_header_LOC_INCLUDE)
#INCLUDES = -I$(shell root-config --incdir)

ROOTSYS  ?= ERROR_RootSysIsNotDefined

ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS   = $(shell root-config --noldflags --libs)

CXX           = g++
CXXFLAGS	    = -g -Wall -fPIC --std=c++0x
LD			      = g++
LDDIR         = -L$(shell root-config --libdir) -Lexternal/lib -L$(BOOST_ROOT)/lib/
LDFLAGS		    = -fPIC $(shell root-config --ldflags) $(LDDIR)
SOFLAGS		    = 
AR            = ar
ARFLAGS       = -cq

CXXFLAGS	   += $(ROOTCFLAGS) $(INCLUDES) -Iinclude/ -Iexternal/include/ -I$(shell echo $(BOOST_ROOT))/include
LIBS  		    = $(ROOTLIBS) -lboost_filesystem -lboost_regex -lboost_system
GLIBS	    	  = $(ROOTGLIBS)
#------------------------------------------------------------------------------
SOURCES		= $(wildcard src/*.$(SrcSuf))
OBJECTS		= $(SOURCES:.$(SrcSuf)=.$(ObjSuf))
DEPENDS		= $(SOURCES:.$(SrcSuf)=.d)
SOBJECTS	= $(SOURCES:.$(SrcSuf)=.$(DllSuf))

.SUFFIXES: .$(SrcSuf) .$(ObjSuf)

###

all: plotIt

clean:
	@rm -f $(OBJECTS);
	@rm -f $(DEPENDS);

plotIt: $(OBJECTS)
	$(LD) $(SOFLAGS) $(LDFLAGS) $+ -o $@ -Wl,-Bstatic -lyaml-cpp -Wl,-Bdynamic $(LIBS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Make the dependencies
%.d: %.cc
	@echo "Generating dependencies for $<"
	@ $(CXX) $(CXXFLAGS) -MM $< | sed -e 's@^\(.*\)\.o:@src/\1.d src/\1.o:@' > $@

ifneq ($(MAKECMDGOALS), clean) 
-include $(DEPENDS) 
endif
