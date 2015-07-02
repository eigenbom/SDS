#include <iostream>
#include <iterator>
#include <list>
#include <cassert>
#include <fstream>

#include "test.h"

#include "transform.h"
#include "mesh.h"
#include "meshloader.h"
#include "organism.h"
#include "vertex.h"
#include "Random.h"

#include "Vector3.h"

Tetra* tet(int num, Mesh* m)
{
	const std::list<Tetra*>& t = m->tetras();
	std::list<Tetra*>::const_iterator tit = t.begin();
	std::advance(tit,num);
	if (tit==t.end()) test(false,"no such tet!");
	return *tit;
}

void transformtests()
{
	std::cout << "transformtests()\n";

	std::cout << "Testing getTetraInDirection()\n";

	// load lattice222.1
	Mesh* m = MeshLoader::Load("tests/lattice222.1"); //note CWD
	test(m!=NULL,"Mesh didn't load correctly!");
	Organism* o = new Organism(m);
	BOOST_FOREACH(Vertex* v,m->vertices())
		o->addCell(new Cell(v));

	// the cell in the middle is the 14th cell, i.e., the cell corresponding to m->verts[13]
	const std::list<Vertex*>& v = m->vertices();
	std::list<Vertex*>::const_iterator vit = v.begin();
	std::advance(vit,13);
	Cell* c = o->getAssociatedCell(*vit);
	test(c!=NULL,"Cell doesn't exist .. oops!");

	// for a set of directions, check that getTetraInDirection() returns the correct tetrahedron

	// 1. gtid(c,vec(1,1,1),m)==t35
	Tetra* t35 = tet(35,m);
	test(Transform::getTetraInDirection(c,Vector3d(1,1,1),o)==t35,"test1 failed!");

	// 2. gtid(c,vec(x,y,z),m)==t35 for all x,y,z positive
	Random r;
	for(int i=0;i<100;i++)
	{
		double x = 10*r.getDouble(), y = 10*r.getDouble(), z = 10*r.getDouble();
		test(Transform::getTetraInDirection(c,Vector3d(x,y,z),o)==t35,"test2 failed!");
	}

	// 3. gtid(c,vec(x,-1,z),m)==t4 for x=(-1,0), z=(-1,0), and (-x)+(-z)<.5
	Tetra* t4 = tet(4,m);
	for(int i=0;i<100;i++)
	{
		double x, z;
		do
		{
			x = -r.getDouble();
			z = -r.getDouble();
		}
		while(not ((-x)+(-z)<.5));
		test(Transform::getTetraInDirection(c,Vector3d(x,-1,z),o)==t4,"test3 failed!");
	}

	// 4. we sample a set of points within a unit cube and test for intersection with a specific tetra
	//    we output the points to be visually analysed (using e.g., mathematica)
	//    (see mathematica->open->sds_transformtests_test4.nb for results)
	std::ofstream test4output("tests/test4output.dat");
	test(test4output,"Can't open tests/test4output.dat for writing!");

	Tetra* t27 = tet(27,m);
	for(int i=0;i<10000;i++)
	{
		double x = r.getDouble(), y = -r.getDouble(), z = r.getDouble();
		if (Transform::getTetraInDirection(c,Vector3d(x,y,z),o)==t27)
		{
			test4output << x << " " << y << " " << z << "\n";
		}
	}


	std::cout << "Passed.\n";
}
