/*
 * simulation.cpp
 *
 * XXX: IMPORTANT:
 * This file has a debug implementation of Simulation.
 * It is very slow as it logs everything, does sanity checks, etc.
 * When the time comes I'll rewrite it for release mode.
 *
 *  Created on: 18/10/2008
 *      Author: ben
 */

#include "sdssimulation.h"

#include "organism.h"
#include "transform.h"
#include "log.h"
#include "processmodel.h"
#include "sdsutil.h"
#include "meshtools.h"

#include "physics.h"
#include "vertex.h"
#include "edge.h"
#include "tetra.h"

#include "gmath.h"
#include "matrix3.h"
#include "vector3.h"

#include <iostream>
#include <list>
#include <cfloat>
#include <sstream>
#include <algorithm>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

#ifdef DEBUG_MEMORY
	#define DUMPM(what) {std::cout << what << std::endl; }
#else
	#define DUMPM(what) ;
#endif

const double SMALLEST_CELL_RADIUS = 0.000001; // 0.001

// simulation state
SDSSimulation::SDSSimulation()
:mOWorld(NULL),
 mOrganism(NULL),
 mTime(0),
 mSteps(0),
 mState(NOT_STARTED),
 mCurrentStep(START),
 mCollisionInterval(0),
 mContinueOnError(false)
{
	DUMPM("SDSSimulation::SDSSimulation() for " << this);
}

SDSSimulation::~SDSSimulation()
{
	DUMPM("SDSSimulation::~SDSSimulation() for " << this);

	cleanup();
}

void SDSSimulation::initialiseFromHeader(SimulationIO_Base::SimulationHeader& hdr)
{
	//globals
	Physics::kD = hdr.kD;
	Physics::kSM = hdr.kSM;
	Physics::kV = hdr.kV;
	Physics::kDamp = hdr.kDamp;
	Physics::GRAVITY = hdr.gravity;
	Physics::DENSITY = hdr.density;
	Physics::VISCOSITY = hdr.viscosity;

	//locals
	setStepSize(hdr.dt);
	setTime(hdr.t);
	setCollisionInterval(hdr.collisionInterval);

	// load static meshes
	BOOST_FOREACH(SimulationIO_Base::StaticMesh sm, hdr.staticMeshes)
	{
		std::cout << "filepath: " << hdr.mFilePath << std::endl;
		Mesh* m = MeshTools::Load(hdr.getAbsolutePath(sm.filePrefix));
		if (m==NULL)
		{
			std::cerr << "Could not load static mesh: " << sm.filePrefix << std::endl;
			std::cerr << "Continuing without it...\n";
		}
		else
		{
			m->move(sm.pos);
			m->scale(sm.sx,sm.sy,sm.sz);
			mStaticMeshes.push_back(m);
		}
	}

}

void SDSSimulation::writeHeader(SimulationIO_Base::SimulationHeader& header)
{
	header.t = t();
	header.dt = stepSize();
	header.kD = Physics::kD;
	header.kSM = Physics::kSM;
	header.kV = Physics::kV;
	header.kDamp = Physics::kDamp;
	header.density = Physics::DENSITY;
	header.viscosity = Physics::VISCOSITY;
	header.collisionInterval = getCollisionInterval();
	header.worldBounds = world()->bounds();
	header.gravity = Physics::GRAVITY;
	header.processModel = world()->organism()->processModel();
}

void SDSSimulation::cleanup()
{
	if (mOWorld)
	{
		delete mOWorld;
		mOWorld = NULL;
		reset();
	}

	BOOST_FOREACH(Mesh* m, mStaticMeshes)
	{
		delete m;
	}
	mStaticMeshes.clear();
}

void SDSSimulation::setWorld(OWorld* o)
{
	// delete the old ones and add new ones..
	if (mOWorld!=o)
		delete mOWorld;

	mOWorld = o;
	mOrganism = o->organism();

	// copy static meshes
	// mStaticMeshes.clear();
	BOOST_FOREACH(Mesh* m, mOWorld->getStaticMeshes())
	{
		if (std::find(mStaticMeshes.begin(),mStaticMeshes.end(),m)==mStaticMeshes.end())
			mStaticMeshes.push_back(m);
	}

	mCollision.reset();
	mCollision.setWorldBounds(o->bounds());
	mCollision.addMesh(mOrganism->mesh());

	BOOST_FOREACH(Mesh* m, mStaticMeshes)
	{
		mCollision.addStaticMesh(m);
	}
}

void SDSSimulation::setStepSize(double dt)
{
	mSuggestedStepSize = dt;
}

double SDSSimulation::stepSize()
{
	return mSuggestedStepSize;
}

bool SDSSimulation::substep()
{
	LOG("SDSSimulation::substep()\n");
	bool fullStep = SDSSimulation::physicalSubStep();
	if (fullStep)
	{
		// XXX: Quick and dirty -- step the process model
		mOrganism->processModel()->step(mOrganism, mSuggestedStepSize);

		// check to see if any errors occurred...
		// check the result
		if (Transform::state == Transform::TRANSFORM_ERROR)
		{
			if (mContinueOnError)
			{
				Transform::reset();
			}
			else
			{
				setUnstable();
				return true;
			}
		}
		else
		{
			#ifdef DEBUG
				if (!mOrganism->mesh()->isSane())
				{
					setUnstable();
					setErrorMessage("Mesh not sane after simulation substep.");
					return false;
				}
			#endif

			updatePhysicalParameters();

			if (not Transform::allProperties.empty())
				return false;
		}
	}

	return fullStep;
}

void SDSSimulation::updatePhysicalParameters()
{
	LOG("SDSSimulation::updatePhysicalParameters()\n");

	// XXX: Move this elsewhere?

	// Grow the cells.
	BOOST_FOREACH(Cell* c, mOrganism->cells())
	{
		if (c->v()->isFrozen()) continue;

		double newr = c->r() + mSuggestedStepSize*c->drdt();
		if (newr < SMALLEST_CELL_RADIUS)
		{
			LOG("Cell radius is below minimum!\n");
			newr = SMALLEST_CELL_RADIUS;
		}
		c->setR(newr);
	}

	// Update the rest lengths
	BOOST_FOREACH(Edge* e, mOrganism->mesh()->edges())
		e->setRest(mOrganism->getAssociatedCell(e->v(0))->r()+mOrganism->getAssociatedCell(e->v(1))->r());
	BOOST_FOREACH(Tetra* t, mOrganism->mesh()->tetras())
	{
		Cell *c0 = mOrganism->getAssociatedCell(&t->v(0)),
			*c1 = mOrganism->getAssociatedCell(&t->v(1)),
			*c2 = mOrganism->getAssociatedCell(&t->v(2)),
			*c3 = mOrganism->getAssociatedCell(&t->v(3));
		t->setRest(Transform::tetrahedraRestVolume(c0,c1,c2,c3));
	}
}

bool SDSSimulation::physicalSubStep()
{
	LOG("SDSSimulation::physicalSubStep()\n");

	lastStepProperties.clear();

	// DEBUG: check sanity of mesh
	if (not mContinueOnError and not mOrganism->mesh()->isSane())
	{
		setUnstable();
		return true;
	}

	if (mCurrentStep==START)
	{
		Transform::clearAllProperties();

		// step the physical SDSSimulation forward
		bool stepWasStable = stepForward();

		if (stepWasStable)
		{

			mState = STABLE;

			// Detect cell movement and handle it accordingly
			bool hamInversion = detectFirstInversion();

			if (not hamInversion and mContinueOnError)
			{
				mCurrentStep = FINISH;
				return false;
			}

			// if stable
			if (mState!=UNSTABLE)
			{
				if (hamInversion)
				{
					mCurrentStep = DETECTED_INVERSION;
					mState = MOVING;

					lastStepProperties.add("inversion detected");
					std::ostringstream oss;
					oss << "mLastStepSize: " << mLastStepSize;
					lastStepProperties.add(oss.str());
				}
				else
				{
					mLastStepSize = mSuggestedStepSize;
				}

				// else mLastStepSize has been set appropriately
				mTime += mLastStepSize;
				Physics::setLastStepSize(mLastStepSize);
				mSteps++;

				return not hamInversion;
			}
			else
			{
				LOG("Last step caused instability\n");
				setUnstable();
				return true;
			}
		}
		else
		{
			LOG("Last step was not stable\n");

			setUnstable();
			return true;
		}
	}
	else if (mCurrentStep==DETECTED_INVERSION or mCurrentStep==HANDLING_INVERSION)
	{
		handleInversion();
		return mState==UNSTABLE;
	}
	else if (mCurrentStep==FINISH)
	{
		mCurrentStep = START;
		return true;
	}

	// shouldn't reach this point
	setUnstable();
	setErrorMessage("Unknown error in simulation.");
	return true;
}

bool SDSSimulation::stepForward()
{
	LOG("SDSSimulation::stepForward()\n");

	// TODO: only do this only new elements
	Physics::SetAllEdgeSpringStrengths(mOrganism->mesh());

	Physics::zeroForces(mOrganism->mesh());
	mOrganism->processModel()->beforePhysicalStep(mOrganism,mSuggestedStepSize);
	Physics::calculateForces(mOrganism->mesh(),false,mContinueOnError);

	// Collision Detection and Response
	// May modify mX and mOldX
	if (mCollisionInterval>0 and (mSteps%mCollisionInterval) == 0)
	{
		try {
			mCollision.estimateCollisionDepthAndDirection();
			mCollision.simpleResponseB();
		}
		catch (Collision::Exception& e)
		{
			std::cerr << "Simulation became unstable due to the exception: " << e.what() << std::endl;
			setUnstable();
			return false;
		}
	}

	return Physics::takeVerletStep(mOrganism->mesh(),mSuggestedStepSize,true, mContinueOnError);

	//return Physics::step(mOrganism->mesh(),mSuggestedStepSize,true);
}

// XXX: This fails if there is a point EXACTLY on a plane
// in which case the system will not be able to step back
bool SDSSimulation::detectFirstInversion()
{
	LOG("SDSSimulation::detectFirstInversion()\n");

	std::list<Tetra*> allInvertedTets;

	// collect all tetrahedra that have been inverted
	BOOST_FOREACH(Tetra* t, mOrganism->mesh()->tetras())
	{
		// ignore the frozen ones
		if (t->isFrozen()) continue;

		if (t->volume() < 0)
			allInvertedTets.push_back(t);
	}

	if (allInvertedTets.size()==0)
	{
		LOG("No inversions detected\n");
		return false;
	}
	else
	{
		LOG(allInvertedTets.size() << " inversions detected\n");
	}

	// rewind SDSSimulation until first inversion is detected
	//double tn = t() + mSuggestedStepSize;
	double totalr = 0;
	Tetra* tetr = NULL;
	while (allInvertedTets.size() > 0)
	{
		double deltaT = mSuggestedStepSize - totalr; // step size of last SDSSimulation step

		// find the tetrahedra whose inversion happens first
		double tr = -1;
		BOOST_FOREACH(Tetra* t, allInvertedTets)
		{
			double deltaH = timeBackToInversion(t,deltaT);
			// bound deltaH to the appropriate interval
			if (mState==UNSTABLE and mContinueOnError)
			{
				t->setFrozen(true);
				mState = STABLE;
				continue;
			}
			else if (mState==UNSTABLE) break;

			if (deltaH >= 0 and deltaH < deltaT
					and deltaH > tr)
			{
				tr = deltaH;
				tetr = t;
			}
		}

		if (mContinueOnError and tr < 0)
		{
			// all bad tets have been frozen, so let's continue on
			return false;
		}

		if (mState==UNSTABLE) return false;

		// rewind ALL vertices back by (an additional) tr seconds
		SDSUtil::rewindAllVertices(mOrganism->mesh(),tr,deltaT);
		totalr += tr;

		// recompute all tetrahedra volumes, note that tetr's volume = 0
		allInvertedTets.clear();
		BOOST_FOREACH(Tetra* t, mOrganism->mesh()->tetras())
		{
			if (t==tetr) continue;
			if (!t->isFrozen() and t->volume() < 0)
			{
				allInvertedTets.push_back(t);
			}
		}

		// if |allInvertedTets| > 0 then tetr was not the first, so we can continue to rewind
		if (totalr > mSuggestedStepSize)
		{
			setUnstable();
			setErrorMessage("Simulation was rewound back past last time step. This should not happen.");
			return false;
		}
	}

	if(tetr==NULL)
	{
		setUnstable();
		setErrorMessage("Simulation error.");
		return false;
	}

	// we have found the first tetrahedral inversion, and rewound the clock
	// determine the type of intersection
	determineIntersectionType(tetr);

	//add the type of intersection and elements involved to properties
	if (mFirstMove.type==Movement::VERTEX_FACE)
	{
		lastStepProperties.add("intersection","vertex through face");
		lastStepProperties.add("tetrahedra",mFirstMove.t);
		lastStepProperties.add("vertex",&mFirstMove.t->v(mFirstMove.v));
	}
	else if (mFirstMove.type==Movement::EDGE_EDGE)
	{
		lastStepProperties.add("intersection","e1 through opposite edge");
		lastStepProperties.add("tetrahedra",mFirstMove.t);
		// get edge 1
		Edge* e1 = mOrganism->mesh()->getEdge(&mFirstMove.t->v(mFirstMove.ea),&mFirstMove.t->v(mFirstMove.eb));
		lastStepProperties.add("e1",e1);
	}

	// set the clock appropriately
	mLastStepSize = mSuggestedStepSize - totalr;

	return true;
}

void SDSSimulation::handleInversion()
{
	if (mCurrentStep==DETECTED_INVERSION or
			mCurrentStep==HANDLING_INVERSION)
	{
		performMove(mFirstMove);
	}
}

double SDSSimulation::t()
{
	return mTime;
}

int SDSSimulation::steps()
{
	return mSteps;
}

double SDSSimulation::dt()
{
	return mLastStepSize;
}

SDSSimulation::SimulationState SDSSimulation::state()
{
	return mState;
}

void SDSSimulation::reset()
{
	mTime = 0;
	mSteps = 0;
	mState = NOT_STARTED;

	// XXX: reset the mesh to the start mesh
}

double SDSSimulation::timeBackToInversion(Tetra* t, double dt)
{
	LOG("SDSSimulation::timeBackToInversion()\n");

	// conisder linear motion for all vertices
	// for each vertex a,b,c,d let a(t),b(t),c(t),d(t) = lerp(a.xOld,a.x,t), lerp(b.xOld,x,t), ...
	int vFirst = -1;
	double tFirst = dt*2;

	// These calculations are not independant, can improve performance by 3x
	// if we acknowledge this dependence
	// All these (i) are the same! We only need to check one vertex!
	// XXX: Is this correct?
	/*
	for(int i=0;i<4;i++)
	{
		double minT = SDSUtil::doVertexPlaneIntersect(t,i,(i+1)%4,(i+2)%4,(i+3)%4) * dt;
		std::cerr << "i" << i << ": " << minT << "\n";

		if (minT < 0)
			continue; // ignore this vertex (no intersect)
		if (minT < tFirst)
		{
			tFirst = minT;
			vFirst = i;
		}
	}
	 */

	double minT = SDSUtil::doVertexPlaneIntersect(t,0,1,2,3) * dt;
	if (minT < tFirst)
	{
		tFirst = minT;
		vFirst = 0;
	}

	if(vFirst==-1)
	{
		setUnstable();
		setErrorMessage("Couldn't find vertex-plane intersection.");
		return 0;
	}

	// set unstable if tFirst is invalid
	if (not (tFirst>0 and tFirst<=dt))
	{
		LOG(t << " -> tfirst = " << tFirst << "\n");
		lastStepProperties.add("bad tetra",t);
		setUnstable();
	}

	// return time needed to rewind
	return dt-tFirst;
}

void SDSSimulation::performMove(const SDSSimulation::Movement& move)
{
	LOG("SDSSimulation::performMove()\n");

	switch(move.type)
	{
		case Movement::VERTEX_FACE:
		{
			Transform::MoveVertexThroughOppositeFace(move.v,move.t,mOrganism);
			if (Transform::state == Transform::TRANSFORM_ERROR and mContinueOnError)
			{
				// freeze move.t
				move.t->setFrozen(true);
				Transform::reset();
			}
			break;
		}
		case Movement::EDGE_EDGE:
		{
			Transform::MoveEdgeThroughEdge(move.ea,move.eb,move.t,mOrganism);
			if (Transform::state == Transform::TRANSFORM_ERROR and mContinueOnError)
			{
				// freeze move.t
				move.t->setFrozen(true);
				Transform::reset();
			}

			break;
		}
	}

	// check the result
	if (Transform::state == Transform::TRANSFORM_ERROR)
	{
		setUnstable();
	}
	else if (Transform::state == Transform::TRANSFORM_COMPLETED)
	{
		mCurrentStep = FINISH;
	}
	else if (Transform::state==Transform::TRANSFORM_TRANSFORMING)
	{
		mCurrentStep = HANDLING_INVERSION;
	}
}

void SDSSimulation::determineIntersectionType(Tetra* t)
{
	double bary[4][2]; // barycentric coords of all points
	for(int i=0;i<4;i++)
	{
		Vector3d v = t->v(i).x();
		Vector3d v1 = t->v((i+1)%4).x(), v2 = t->v((i+2)%4).x(), v3 = t->v((i+3)%4).x();
		// calculate barycentric coords for v of opposite face (v1,v2,v3)
		Math::baryTri(v1,v2,v3,v,bary[i][0],bary[i][1]);

		// check to see if v is contained entirely within the opposite face
		if ((bary[i][0] + bary[i][1]) <= 1 and bary[i][0] >= 0 and bary[i][1] >= 0)
		{
			// in which case, we immediately have a v-f intersection so can return
			mFirstMove.t = t;
			mFirstMove.type = Movement::VERTEX_FACE;
			mFirstMove.v = i;
			return;
		}
	}

	// a v-f hasn't occurred, and hence an e-e has occurred..
	mFirstMove.t = t;
	mFirstMove.type = Movement::EDGE_EDGE;
	mFirstMove.ea = 0;

	if (bary[0][0] >= 0 and bary[0][1] <= 0)
		mFirstMove.eb = 3;
	else if (bary[0][0] >=0 and bary[0][1] >= 0) // -> sum > 1
		mFirstMove.eb = 1;
	else
		mFirstMove.eb = 2;
}

void SDSSimulation::setUnstable()
{
	mState = UNSTABLE;
	lastStepProperties.add("Simulation State","unstable");
}

void SDSSimulation::setContinueOnError(bool c)
{
	mContinueOnError = c;
}

bool SDSSimulation::continueOnError()
{
	return mContinueOnError;
}
