###############################################################################
#                                                                             #
# makefile for TetGen                                                         #
#                                                                             #
# Type "make" to compile TetGen into an executable program (tetgen).          #
# Type "make tetlib" to compile TetGen into a library (libtet.a).             #
# Type "make distclean" to delete all object (*.o) files.                     #
#                                                                             #
###############################################################################

# CXX should be set to the name of your favorite C++ compiler.
# ===========================================================

CXX = g++
#CXX = icpc
#CXX = CC

# CXXFLAGS is the level of optimiztion, default is -O. One should try
# -O2, -O3 ... to find the best optimization level.
# ===================================================================

CXXFLAGS = -O3

# PREDCXXFLAGS is for compiling J. Shewchuk's predicates. It should
# always be equal to -O0 (no optimization). Otherwise, TetGen may not
# work properly.

PREDCXXFLAGS = -O0

# SWITCHES is a list of switches to compile TetGen.
# =================================================
#
# By default, TetGen uses double precision floating point numbers.  If you
#   prefer single precision, use the -DSINGLE switch. 
#
# The source code of TetGen includes a lot of assertions, which are mainly
#   used for catching bugs at that places.  These assertions somewhat slow
#   down the speed of TetGen.  They can be skipped by define the -DNDEBUG
#   switch.

SWITCHES = -Wall -DSELF_CHECK -DNDEBUG

# SWITCHES = -Wall -Wabi -Wctor-dtor-privacy \
#            -Woverloaded-virtual -Wno-pmf-conversions -Wsign-promo \
#            -Wsynth  -Wchar-subscripts -Wconversion -Wsign-compare \
#            -Wcomment  -Wimplicit -Wmissing-braces -Wparentheses \
#            -Wreturn-type -Wswitch -Wswitch-default \
#            -Wswitch-enum -Wtrigraphs -W -DSELF_CHECK

# RM should be set to the name of your favorite rm (file deletion program).

RM = -del

# The action starts here.

objs: predicates.o tetgen.o

tetgen.o:
	$(CXX) $(CXXFLAGS) $(SWITCHES) -DTETLIBRARY -c tetgen.cxx

tetgen:	tetgen.cxx predicates.o
	$(CXX) $(CXXFLAGS) $(SWITCHES) -o tetgen tetgen.cxx predicates.o -lm

tetlib: tetgen.cxx predicates.o
	$(CXX) $(CXXFLAGS) $(SWITCHES) -DTETLIBRARY -c tetgen.cxx
	ar r libtet.a tetgen.o predicates.o

predicates.o: predicates.cxx
	$(CXX) $(PREDCXXFLAGS) -c predicates.cxx

clean:
	$(RM) *.o *.a tetgen *~




