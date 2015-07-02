#include "world.h"
#include <boost/foreach.hpp>

World::~World()
{
	if (mOwns)
	{
		BOOST_FOREACH(Mesh* m, mMeshes)
			delete m;
	}
}

/// add a new mesh to this world
void World::addMesh(Mesh* m)
{
	mMeshes.push_back(m);
}

void World::removeMesh(Mesh* m)
{
	mMeshes.remove(m);
}

/// calculate a suitable bounding box that contains all meshes
void World::calculateBounds()
{
	if (mMeshes.empty()) return;
	else
	{
		mBounds = AABB::ZERO;
		BOOST_FOREACH(Mesh* m, mMeshes)
		{
			mBounds += m->aabb();
		}

		double dx = mBounds.dx();
		double dy = mBounds.dy();
		double dz = mBounds.dz();

		mBounds[0] -= dx/2;
		mBounds[1] -= dy/2;
		mBounds[2] -= dz/2;
		mBounds[3] += dx/2;
		mBounds[4] += dy/2;
		mBounds[5] += dz/8;
		mBounds.validate();
	}
}

void World::defaultBounds()
{
	mBounds = AABB::ZERO;
	mBounds[0] = -1;
	mBounds[1] = 0;
	mBounds[2] = -1;
	mBounds[3] = 1;
	mBounds[4] = 2;
	mBounds[5] = 1;
	mBounds.validate();
}

double World::energy() const
{
	// the energy of the world is simply the summed energy of all meshes
	double energy = 0;
	BOOST_FOREACH(Mesh* m, mMeshes)
		energy += m->energy();
	return energy;

}
