#ifndef MESHSIMULATION_H
#define MESHSIMULATION_H

#include "mesh.h"
#include "collision.h"
#include "aabb.h"
#include "output.h"
#include "world.h"

/*
 * MeshSimulation handles the setup and execution of a mesh-only simulation.
 * Useful for testing the physics engine without morphogenesis rules, etc.
 * Hook in an output module to handle the dumping of simulation results.
 *
 * BP1009
 */
class MeshSimulation
{
	public:
	enum SimulationState{NOT_STARTED, STABLE, UNSTABLE, COMPLETE};

	MeshSimulation(World* w);
	~MeshSimulation();

	/// set some simulation parameters
	void setStepSize(double d){mDT = d;}
	void setColInterval(int i){mColInterval = i;}
	void setCheckForStability(){mCheckStability = true;}

	/// attach output modules to this simulation
	/// PRE: o has had it's start() function called
	void attachOutput(Output* o);

	/// deprecated: use World::bounds() instead
	const AABB& bounds(){return mWorld->bounds();}
	double dt(){return mDT;}
	double t(){return mDT * mNumSteps;}

	/// main interface to the simulation stepper
	void step();
	void output();

	/// info retrieval
	SimulationState state(){return mState;}
	unsigned int steps(){return mNumSteps;}

	Collision& collision(){return mCollision;}

	protected:

	SimulationState mState;

	World* mWorld;
	Collision mCollision;

	// simulation parameters
	int mColInterval;
	double mDT;
	unsigned int mNumSteps;

	bool mCheckStability;

	std::list<std::pair<Output*,int> > mOutputModules;
};

#endif /* MESHSIMULATION_H */
