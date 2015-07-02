#include "oworld.h"

#include "organism.h"

#include <boost/foreach.hpp>
#include <cassert>

#ifdef DEBUG_MEMORY
	#define DUMPM(what) {std::cout << what << std::endl; }
#else
	#define DUMPM(what) ;
#endif


OWorld::OWorld(bool own)
:mOwnsOrganisms(own)
{
	DUMPM("OWorld::OWorld for " << this);

	// create a new world
	mWorld = new World(own);
}

OWorld::~OWorld()
{
	DUMPM("OWorld::~OWorld for " << this);

	// delete the world, but first remove all meshes so we don't
	// delete them twice (as ~Organism also deletes a mesh)

	if (mOwnsOrganisms)
	{
		std::list<Mesh*> meshes = mWorld->meshes();
		BOOST_FOREACH(Mesh* m, meshes)
			mWorld->removeMesh(m);
	}

	delete mWorld;

	if (mOwnsOrganisms)
	{
		// now delete all organisms
		BOOST_FOREACH(Organism* o, mOrganisms)
			delete o;
	}

	BOOST_FOREACH(Mesh* m, mStaticMeshes)
		delete m;
}

void OWorld::addOrganism(Organism* o)
{
	if (o==NULL) return;
	mOrganisms.push_back(o);
	mWorld->addMesh(o->mesh());
}

void OWorld::removeOrganisms()
{
	BOOST_FOREACH(Organism* o, mOrganisms)
	{
		mWorld->removeMesh(o->mesh());
	}
	mOrganisms.clear();
}

Organism* OWorld::organism()
{
	if (mOrganisms.empty()) return NULL;
	else return mOrganisms.front();
}

void OWorld::addStaticMesh(Mesh* m) { mStaticMeshes.push_back(m); }
const std::list<Mesh*>& OWorld::getStaticMeshes() const { return mStaticMeshes; }

void OWorld::calculateBounds()
{
	AABB bounds = organism()->mesh()->aabb();
	BOOST_FOREACH(Mesh* m, mStaticMeshes)
		bounds += m->aabb();
	bounds *= 2;

	mWorld->setBounds(bounds);
}
