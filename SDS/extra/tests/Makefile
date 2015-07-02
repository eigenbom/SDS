ROOTDIR = ..\..\..
LIBDIRS = $(addprefix -L$(ROOTDIR)\, SDSCommon\Release SDSMath\Release SDS\Release)
ALLLIBS = $(addprefix -l, SDS SDSMath SDSCommon)
INCLUDEDIRS = $(addprefix -I$(ROOTDIR)\, SDSCommon\include SDSMath\include SDS\include)

SRC = dividetetra.cpp main.cpp sdsLoaderTest.cpp

all: test

test: $(SRC)
	$(CXX) -DDEBUG -g -Wall -o test.exe $(SRC) $(LIBDIRS) $(ALLLIBS) $(INCLUDEDIRS)

processtest: processtest.cpp
	$(CXX) -DDEBUG -g -Wall -o processtest.exe processtest.cpp $(LIBDIRS) $(ALLLIBS) $(INCLUDEDIRS)
