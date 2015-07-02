/*
 * SDS Unit Test: Transform::DivideTetra
 *
 */

#include "organism.h"
#include "transform.h"

#include "mesh.h"
#include "vertex.h"
#include "meshtester.h"
#include "test.h"

#include <boost/tuple/tuple.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <cfloat>
#include <string>
#include <sstream>

const double DOUBLE_EPSILON = 2*DBL_EPSILON;



/*
inline void sdstest(bool condition, std::string fail = "")
{
	if (condition == false)
	{
		std::cout << "fail at " << __LINE__ << ": " << fail << std::endl;
		exit(0);
	}
}
*/

void divideTetraTest()
{
	std::cout << "Test: SDS::Transform::DivideTetra" << std::endl;


	// Create a single tetrahedra with edge lengths = 1
	Mesh* m = MeshTester::CreateTestMesh("1tet");
	double oldMeshMass = m->mass();

	Organism* o = new Organism(m);

	Cell* cells[4];
	Vertex* verts[4];

	std::list<Tetra*>::const_iterator it = m->tetras().begin();
	sdstest(it!=m->tetras().end(),"");
	Tetra* t = *it;

	for(int i=0;i<4;i++)
	{
		verts[i] = &t->v(i);
		cells[i] = new Cell(verts[i],.5);
		o->addCell(cells[i]);
	}

	// ********* Test Organism basic functions **************
	for(int i=0;i<4;i++)
	{
		sdstest(o->getAssociatedCell(verts[i])==cells[i],"");
	}

	for(int i=0;i<4;i++)
	for(int j=1;j<4;j++)
	{
		// also validates topology of 1tet
		sdstest(o->edge(cells[i],cells[(i+j)%4]),"");
		sdstest(o->edge(cells[(i+j)%4],cells[i]),"");
	}

	// ********* Test basic tetrahedron info
	sdstest(m->vertices().size()==4,"");
	sdstest(m->edges().size()==6,"");
	//sdstest(m->faces().size()==4,"");
	sdstest(m->outerFaces().size()==4,"");
	sdstest(m->tetras().size()==1,"");

	// *********** DIVIDE THIS TETRA AT CELL 0
	boost::tuple<Cell*,Cell*> result = Transform::DivideTetra(cells[0],t,o);
	// t is now invalid, so be safe and NULL it
	t = NULL;

	Cell *daughter1 = result.get<0>(), *daughter2 = result.get<1>();
	Cell* cx = daughter1;

	if (cx==cells[0])
		cx = daughter2;

	Vertex* vx = cx->v();

	// cx is in the center of the old tetra t

	// ********* Test transformed tetrahedra info
	sdstest(m->vertices().size()==5,"");
	sdstest(m->edges().size()==10,"");
	//sdstest(m->faces().size()==10,"");
	sdstest(m->outerFaces().size()==4,"");
	sdstest(m->tetras().size()==4,"");

	BOOST_FOREACH(Tetra* t, m->tetras())
	{
		sdstest(
		(t->isVertex(vx) and t->isVertex(verts[1]) and t->isVertex(verts[2]) and t->isVertex(verts[3])) or
		(t->isVertex(verts[0]) and t->isVertex(vx) and t->isVertex(verts[2]) and t->isVertex(verts[3])) or
		(t->isVertex(verts[0]) and t->isVertex(verts[1]) and t->isVertex(vx) and t->isVertex(verts[3])) or
		(t->isVertex(verts[0]) and t->isVertex(verts[1]) and t->isVertex(verts[2]) and t->isVertex(vx)),"");
	}

	// get each tetra and name it
	Tetra* tets[4];
	BOOST_FOREACH(Tetra* t, m->tetras())
	{
		tets[t->vIndex(vx)] = t;

		for(int i=0;i<4;i++)
		{
			if (i==t->vIndex(vx)) continue;
			sdstest(t->vIndex(verts[i])==i,"");
		}
	}

	// ********* check tetra connectedness
	for(int i=0;i<4;i++)
	for(int j=0;j<4;j++)
	{
		Tetra* tn = tets[i]->neighbour(j);
		if (i==j)
			sdstest(tn == NULL,""); //outer face
		else
			sdstest(tn == tets[j],"");
	}

	// ********* check edge connectedness
	for(int i=0;i<5;i++)
	for(int j=0;j<5;j++)
	{
		Cell *c1 = NULL, *c2 = NULL;
		if (i==4)
			c1 = cx;
		else
			c1 = cells[i];
		if (j==4)
			c2 = cx;
		else
			c2 = cells[j];

		if (i==j)
			sdstest(o->edge(c1,c2)==NULL,"");
		else
			sdstest(o->edge(c1,c2)!=NULL,"");
	}

	std::cout << "Testing: Topology Tests Passed!" << std::endl;

	// ********* Test transformed tetrahedra rest lengths, volumes and cell mass

	// 1. old mesh mass = new mesh mass
	double newMeshMass = m->mass();
	std::ostringstream oss;
	oss << "old mass: " << oldMeshMass << ", new mass: " << newMeshMass;
	sdstest(fabs(oldMeshMass - newMeshMass) < DOUBLE_EPSILON,oss.str());

	// 2. for all edges e. e.rest() = e.a().r() + e.b().r()
	BOOST_FOREACH(Edge* e, m->edges())
	{
		double rest = e->rest();
		double ar = o->getAssociatedCell(e->v(0))->r();
		double br = o->getAssociatedCell(e->v(1))->r();

		oss.str("");
		oss << "edge (" << e << ") rest: " << rest << ", a.r: " << ar << ", b.r: " << br;
		sdstest(fabs(rest - (ar+br)) < DOUBLE_EPSILON,oss.str());
	}

	// 3. sum of tetra current volume = mesh volume
	double sumoftetravol = 0;
	BOOST_FOREACH(Tetra* t, m->tetras())
	{
		sumoftetravol += t->volume();
	}

	oss.str("");
	oss << "Sum(Tetra.vol): " << sumoftetravol << ", m->volume(): " << m->volume();
	sdstest(fabs(sumoftetravol - m->volume()) < DOUBLE_EPSILON,oss.str());

	// XXX: 4. spring coefficients?


	std::cout << "Testing: Physical Tests Passed!" << std::endl;
}







