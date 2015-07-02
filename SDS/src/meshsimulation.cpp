#include "meshsimulation.h"

#include "physics.h"
#include "log.h"

#include <iostream>
#include <boost/foreach.hpp>

MeshSimulation::MeshSimulation(World* w)
	:mState(NOT_STARTED),mWorld(w)
	,mColInterval(5)
	,mNumSteps(0)
	,mCheckStability(false)
{
	BOOST_FOREACH(Mesh* m, mWorld->meshes())
		mCollision.addMesh(m);

	mCollision.setWorldBounds(mWorld->bounds());
}

MeshSimulation::~MeshSimulation()
{

}

void MeshSimulation::step()
{
	LOG(__FILE__ << __LINE__);

	mNumSteps++;

	// Apply Forces
	// Handle Collisions
	// Step Simulation
	// Output Data

	BOOST_FOREACH(Mesh* mesh, mWorld->meshes())
		Physics::calculateForces(mesh);

	// Collision Detection and Response
	// May modify mX and mOldX
	if ((mNumSteps%mColInterval) == 0)
	{
		mCollision.estimateCollisionDepthAndDirection();
		mCollision.simpleResponseB();
	}

	// Step the simulation
	// also updates v->velocity
	BOOST_FOREACH(Mesh* mMesh, mWorld->meshes())
	{
		if (mCheckStability)
		{
			bool stable = Physics::takeVerletStep(mMesh,mDT,true);

			if (not stable)
			{
				mState = UNSTABLE;
				return;
			}
			else
			{
				// more checks for stability

				// simulation unstable if the verts of any
				// tetra are simultaneously out of bounds
				// but on different sides of the world
				const AABB& bounds = mWorld->bounds();
				Vector3d bmin = bounds.min();
				Vector3d bmax = bounds.max();

				BOOST_FOREACH(const Tetra* t, mMesh->tetras())
				{
					// for each vertex, tag whether it is
					// < minx, > maxx,
					// < miny, > maxy,
					// < minz, > maxz
					bool minx = false, maxx = false, miny = false, maxy = false, minz = false, maxz = false;

					for(int i=0;i<4;i++)
					{
						const Vector3d p = t->cv(i).x();
						if (not bounds.contains(p))
						{
							if (p.x() < bmin.x()) minx = true;
							if (p.y() < bmin.y()) miny = true;
							if (p.z() < bmin.z()) minz = true;

							if (p.x() > bmax.x()) maxx = true;
							if (p.y() > bmax.y()) maxy = true;
							if (p.z() > bmax.z()) maxz = true;
						}
					}

					if ((minx and maxx) or (miny and maxy) or (minz and maxz))
					{
						mState = UNSTABLE;
						return;
					}

				}

			}
		}
		else
			Physics::takeVerletStep(mMesh,mDT,false);
	}

	mState = STABLE;
}

void MeshSimulation::attachOutput(Output* o)
{
	std::pair<Output*,int> p;
	p.first = o;
	p.second = o->n();
	mOutputModules.push_back(p);
}

void MeshSimulation::output()
{
	typedef std::pair<Output*,int> outputpair;
	BOOST_FOREACH(outputpair o, mOutputModules)
	{
		if ((mNumSteps % o.second)==0)
			o.first->output(mNumSteps);
	}
}
