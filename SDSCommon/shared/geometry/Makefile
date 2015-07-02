all: libgeom testgeom

objs: geometry.o

geometry.o: geometry.C geometry.H
	g++ -c -O3 -Wall -o geometry.o geometry.C

libgeom: geometry.o
	ar r libgeom.a geometry.o

geometry_prb.o: geometry_prb.C geometry.H
	g++ -c -o geometry_prb.o geometry_prb.C

testgeom: libgeom geometry_prb.o
	g++ -o testgeom.exe geometry_prb.o libgeom.a
	
