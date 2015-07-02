/*
 * Created on: 16/10/2008
 *      Author: ben
 */

#include "organismtools.h"

#include "transform.h"
#include "organism.h"
#include "sdssimulation.h"

#include "vertex.h"
#include "tetra.h"
#include "mesh.h"
#include "edge.h"
#include "meshtester.h"
#include "meshtools.h"

#include "gmath.h"
#include "vector3.h"

#include <cassert>
#include <stdexcept>

#include <boost/regex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

Organism* OrganismTools::load(const std::string& name)
{
	static const boost::regex eetest_re("ee.*");

	Organism* newOrganism;

	if (name=="1tet")
		newOrganism = loadOneTet();
	else if (name=="vf")
		throw std::runtime_error("Test not available, sorry!");
		//newOrganism = loadVF();
	else if (name=="eespecial")
		throw std::runtime_error("Test not available, sorry!");
		//newOrganism = loadEESpecial();
	else if (boost::regex_match(name,eetest_re))
		throw std::runtime_error("Test not available, sorry!");
		//newOrganism = loadEETest(name);
	else
		newOrganism = loadMesh(name);

	return newOrganism;

}

Organism* OrganismTools::loadOneTet()
{
	// Create a single tetrahedra with edge lengths = 1
	Mesh* m = MeshTester::CreateTestMesh("1tet");
	Organism* o = new Organism(m);

	std::list<Tetra*>::const_iterator it = m->tetras().begin();
	//assert(it!=m->tetras().end());
	Tetra* t = *it;
	for(int i=0;i<4;i++)
	{
		Cell* c;
		o->addCell(c = new Cell(&t->v(i),.5));
		c->setM(Math::volumeOfSphereGivenRadius(c->r()));
	}
	return o;
}

/*
Organism* OrganismTools::loadEESpecial()
{
	// NOTE: Have to set these up BEFORE loading the mesh
	// set up the appropriate simulation parameters
	double dt = 0.001;

	Physics::kD = 0;
	Physics::kV = 0;
	Physics::kDamp = 0;
	Physics::GRAVITY = 0;
	Simulation::setStepSize(dt);

	Mesh* m = MeshLoader::Load(std::string("..\\SDS\\mesh\\eespecial.1"));
	Organism* o = new Organism(m);
	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		Cell* c = new Cell(v,.5);
		o->addCell(c);
		c->setM(Math::volumeOfSphereGivenRadius(c->r()));
	}

	// send one vertex skyward
	(*m->vertices().begin())->addX(Vector3d(0,0.00278,0));
	return o;
}

Organism* OrganismTools::loadVF()
{
	// NOTE: Have to set these up BEFORE loading the mesh
	// set up the appropriate simulation parameters
	double dt = 0.001;

	Physics::kD = 0;
	Physics::kV = 0;
	Physics::kDamp = 0;
	Physics::GRAVITY = 0;
	Simulation::setStepSize(dt);

	// Create a single tetrahedra with edge lengths = 1
	Mesh* m = MeshTester::CreateTestMesh("1tet");
	Organism* o = new Organism(m);
	std::list<Tetra*>::const_iterator it = m->tetras().begin();
	assert(it!=m->tetras().end());
	Tetra* t = *it;
	for(int i=0;i<4;i++)
	{
		Cell* c;
		o->addCell(c = new Cell(&t->v(i),.5));
		c->setM(Math::volumeOfSphereGivenRadius(c->r()));
	}

	// send one vertex towards the opposite face

	Vertex* vfrom = *m->vertices().begin();
	Vector3d to = Vector3d::ZERO;
	BOOST_FOREACH(Face* f, m->outerFaces())
	{
		if (not f->contains(vfrom)){
			to = f->center();
			break;
		}
	}

	vfrom->addX((to - vfrom->x())*0.00278);
	return o;
}

Organism* OrganismTools::loadEETest(std::string name) 
{
	// NOTE: Have to set these up BEFORE loading the mesh
	// set up the appropriate simulation parameters
	double dt = 0.001;

	Physics::kD = 0;
	Physics::kV = 0;
	Physics::kDamp = 0;
	Physics::GRAVITY = 0;
	Simulation::setStepSize(dt);

	// XXX: Better resource locator/system?
	Mesh* m = MeshLoader::Load(std::string("..\\SDS\\mesh\\")+name+".1");
	if (m==NULL) throw(new std::runtime_error(std::string("Can't load ee test: ")+name));

	Organism* o = new Organism(m);

	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		o->addCell(new Cell(v,.5));
		v->setM(Math::volumeOfSphereGivenRadius(.5));
	}

	// fix the rest lengths of all the edges, etc
	BOOST_FOREACH(Edge* e, m->edges())
	{
		Cell *ea = o->getAssociatedCell(e->v(0)), *eb = o->getAssociatedCell(e->v(1));
		e->setRest(ea->r() + eb->r());
	}

	BOOST_FOREACH(Tetra* t, m->tetras())
	{
		Cell *c0 =o->getAssociatedCell(&t->v(0)),
		*c1 =o->getAssociatedCell(&t->v(1)),
		*c2 =o->getAssociatedCell(&t->v(2)),
		*c3 =o->getAssociatedCell(&t->v(3));
		t->setRest(Transform::tetrahedraRestVolume(c0,c1,c2,c3));
	}

	//squash the central tetrahedron in its vertical axis
	// this is to avoid other types of collisions
	Tetra* t = *m->tetras().begin();
	Vector3d tc = t->center();
	for(int i=0;i<4;i++)
	{
		Vector3d p = t->v(i).x();
		t->v(i).resetX(p + Vector3d(0.0031,(p.y()-tc.y())*-.9,-0.11));
	}

	// perturb vertex 0 so that it causes an e-e collision
	Vertex* v0 = *m->vertices().begin();
	v0->addX(Vector3d(0,-0.00712357,0)); // XXX: some small amount that doesn't result in exact overlap
	return o;
}
*/

Organism* OrganismTools::loadMesh(std::string filename) 
{
	// XXX: Better resource locator/system?
	Mesh* m = MeshTools::Load(filename);
	if (m==NULL) throw(std::runtime_error(std::string("Can't load mesh: ")+filename));

	Organism* o = new Organism(m);
	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		// calculate an approximate radius, given the current lengths of all the edges
		o->addCell(new Cell(v,0));
		// v->setM(Math::volumeOfSphereGivenRadius(.5));
	}

	BOOST_FOREACH(Edge* e, m->edges())
	{
		Cell *c0 =o->getAssociatedCell(e->v(0)),
		*c1 =o->getAssociatedCell(e->v(1));

		c0->setR(e->length()/2.0+c0->r());
		c1->setR(e->length()/2.0+c1->r());
	}

	BOOST_FOREACH(Cell* c, o->cells())
	{
		c->setR(c->r() / c->v()->neighbours().size());
		c->v()->setM(Math::volumeOfSphereGivenRadius(c->r()));
	}

	// set the rest lengths of all the edges, etc
	// note that we preserve the original rest lengths with the rest multiplier
	BOOST_FOREACH(Edge* e, m->edges())
	{
		Cell *ea =o->getAssociatedCell(e->v(0)), *eb =o->getAssociatedCell(e->v(1));

		double d = dist(e->v(0)->x(),e->v(1)->x());
		double sumradii = ea->r() + eb->r();
		e->setRestMultiplier(d/sumradii);
		e->setRest(sumradii);

		//std::cout << "Incoming edge: actual length = " << d << ", sumradii = " << sumradii << ", rest mult = " << e->restMultiplier() << ", e->rest() = " << e->rest() << "\n";
	}

	BOOST_FOREACH(Tetra* t, m->tetras())
	{
		Cell *c0 =o->getAssociatedCell(&t->v(0)),
		*c1 =o->getAssociatedCell(&t->v(1)),
		*c2 =o->getAssociatedCell(&t->v(2)),
		*c3 =o->getAssociatedCell(&t->v(3));

		double currentVolume = t->volume();
		double desiredVolume = Transform::tetrahedraRestVolume(c0,c1,c2,c3);
		t->setRestMultiplier(currentVolume/desiredVolume);
		t->setRest(desiredVolume);
	}
	return o;
}

std::list<std::string> OrganismTools::testnames()
{
	std::list<std::string> files;
	files.push_back("1tet");
	files.push_back("vf");
	files.push_back("eespecial");

	// get all test files in SDS test
	static const boost::regex eleextension("(.*)\\.1\\.ele");


	fs::path path_to_tests("../SDS/mesh");
	fs::directory_iterator end_itr;
	if (fs::exists(path_to_tests))
	{
		for ( fs::directory_iterator itr( path_to_tests );
		itr != end_itr;
		++itr )
		{
			if (not fs::is_directory(itr->status()))
			{
				std::string filename = itr->path().leaf();

				boost::smatch mr;
				if (boost::regex_match(filename,mr,eleextension))
				{
					std::string truncatedfilename = mr[1];
					files.push_back(truncatedfilename);
				}
			}
		}
	}

	return files;
}
