#include "meshops.h"
#include "simulationio.h"
#include "segmentio.h"
#include "organism.h"
#include "organismtools.h"
#include "meshtools.h"
#include "oworld.h"
#include "cell.h"
#include "face.h"
#include "transform.h"

#include <boost/foreach.hpp>
#include <iostream>

#define TEST(x,msg) {std::cout << "TEST: " << msg; if (not (x)) {std::cout << " failed\n";} else {std::cout << " passed\n";}}

void printDetails(Organism* o)
{
	Mesh* m = o->mesh();

	std::cout << "num cells: " << o->cells().size() << "\n";
	std::cout << "num verts: " << m->vertices().size() << "\n";
	std::cout << "num edges: " << m->edges().size() << "\n";
	std::cout << "num faces: " << m->faces().size() << "\n";
	std::cout << "num tets : " << m->tetras().size() << "\n";
}

void test1();
void test2();
void test3();

int main()
{
	//test1();
	//test2();
	test3();
	return 0;
}

void test1()
{
	std::cout << "Testing OneTet\n";
	Organism* o = OrganismTools::loadOneTet();
	printDetails(o);

	bool success = Transform::Complexify(o->cells(),o->mesh()->tetras(),o);
	if (success)
	{
		std::cout << "Complexify successful.\n";

		printDetails(o);

		TEST(o->cells().size()==9,"cell count");
		TEST(o->mesh()->isSane(),"mesh sanity");
	}
	else
	{
		std::cout << "Complexify returned false.\n";
	}
	delete o;
}

void test2()
{
	Organism* o = NULL;
	std::cout << "Testing 1Cube\n";
	try
	{
		o = OrganismTools::loadMesh("../data/lattice111.1");
		ProcessModel* p = ProcessModel::create("NoProcessModel");
		o->setProcessModel(p);
	}
	catch(std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return;
	}

	printDetails(o);
	bool success = Transform::Complexify(o->cells(),o->mesh()->tetras(),o);
	if (success)
	{
		std::cout << "Complexify successful.\n";
		printDetails(o);

		TEST(o->mesh()->isSane(),"mesh sanity");

	}
	else
	{
		std::cout << "Complexify returned false.\n";
	}
	delete o;
}

void test3()
{
	Organism* o = NULL;
	std::cout << "Testing 1Cube 1cell\n";
	try
	{
		o = OrganismTools::loadMesh("../data/lattice111.1");
		ProcessModel* p = ProcessModel::create("NoProcessModel");
		o->setProcessModel(p);
	}
	catch(std::runtime_error& e)
	{
		std::cerr << e.what() << std::endl;
		return;
	}

	printDetails(o);

	Cell* a = *(o->cells().begin());
	std::list<Cell*> cells;
	cells.push_back(a);

	std::list<Tetra*> tetras;
	BOOST_FOREACH(Tetra* t, o->mesh()->tetras())
	{
		if (t->contains(a->v()))
			tetras.push_back(t);
	}

	bool success = Transform::Complexify(cells,tetras,o);
	if (success)
	{
		std::cout << "Complexify successful.\n";
		printDetails(o);

		TEST(o->mesh()->isSane(),"mesh sanity");

	}
	else
	{
		std::cout << "Complexify returned false.\n";
	}
	delete o;
}
