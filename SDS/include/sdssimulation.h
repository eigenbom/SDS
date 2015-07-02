/** Handles the simulation step for SDS
*
*  Created on: 18/10/2008
*      Author: ben
*/

#ifndef SDSSIMULATION_H_
#define SDSSIMULATION_H_

#include "tetra.h"
#include "log.h"
#include "organism.h"
#include "oworld.h"
#include "propertylist.h"
#include "collision.h"
#include "simulationio.h"

#include <boost/tuple/tuple.hpp>
#include <boost/any.hpp>
#include <vector>

/**
* Simulation encapsulates the entire state of an SDS simulation.
*
* TODO: Add multiple organism support
* TODO: Write fast/release level implementations of transformations
*/
class SDSSimulation {

public:
	SDSSimulation();
	~SDSSimulation();

	/**
	 * Initialise the simulation from a header.
	 */
	void initialiseFromHeader(SimulationIO_Base::SimulationHeader&);

	/**
	 * Write the current simulation params into a header.
	 */
	void writeHeader(SimulationIO_Base::SimulationHeader&);

	/**
	 * deletes world and its inhabitants etc.
	 * POST: world() is null.
	 */
	void cleanup();

	OWorld* world(){return mOWorld;}
	std::list<Mesh*> getStaticMeshes(){return mStaticMeshes;}

	/**
	 * MUST call this before simulating.
	 * ATM only one organism in the world is simulated.
	 * Add any static meshes to the world before calling this function.
	 *
	 * TODO: add multiple organism support
	 */
	void setWorld(OWorld* o);

	/** take a full step, or if something happens, a substep
	* returns TRUE if completed a full step, FALSE otherwise
	* To simulate, just repeatedly call substep.
	*/
	bool substep();

	/**
	* reset the parameters of the simulation
	* NOTE: this will not reset the organism,
	* you must reload the organism if you want to restart the simulation.
	*/
	void reset();

	/**
	* Set the standard step size.
	* The actual step size may be less than this if an event occurs between steps (like a cell move).
	* @param dt (seconds)
	*/
	void setStepSize(double dt);
	double stepSize();

	void setTime(double t){mTime = t;}

	/// total simulation time
	double t();
	/// last step size
	double dt();
	/// number of simulation steps taken
	int steps();

	void setCollisionInterval(int ci){mCollisionInterval = ci;}
	int getCollisionInterval(){return mCollisionInterval;}

	/// state of the simulation
	enum SimulationState{NOT_STARTED, STABLE, UNSTABLE, MOVING};
	SimulationState state();

	// if simulationstate==UNSTABLE then errorMessage may contain some info
	std::string getErrorMessage(){return mErrorMessage;}
	void setErrorMessage(std::string msg){mErrorMessage = msg;}


	/**
	 * Allow the simulator to continue on some errors.
	 * Note this will disable the unstable flag.
	 */
	void setContinueOnError(bool c);
	bool continueOnError();

	/**
	* The interface to inspect the current state of the Simulation
	* A Property is a (string,something) pair that exposes some interesting information about the current step
	* NOTE TO ME: properties hold information for external use only, they aren't used internally by the Simulation class (I use vars underneath the hood
	* After each substep a new list of properties will be available
	* and the old list deleted
	* WARNING: don't hold onto pointers/references in the property list for long as they may become invalid
	* (e.g., Tetras may be deleted for example)
	*/
	PropertyList lastStepProperties;


private:
	// the simulator steps through the following sequence of substeps
	// it may skip some
	enum Step{START,DETECTED_INVERSION,HANDLING_INVERSION,FINISH};

	// update the physical parameters, such as spring stiffness, etc...
	void updatePhysicalParameters();

	// steps the physical simulator and reacts to movements (called by substep())
	bool physicalSubStep();

	/// returns false if simulation has become unstable
	bool stepForward();

	bool detectFirstInversion();
	void handleInversion();

	/**
	 * linearly interpolates the time back when t first inverted
	 * PRE: t has inverted in the last time step [sTime,sTime+dt]
	 * returns: number of seconds back to inversion (>= 0)
	 */
	double timeBackToInversion(Tetra* t, double dt);

	/// determineIntersectionType:
	/// stores in sFirstMove the details of the collapsed tetra t
	/// PRE: t has collapsed into volume 0
	void determineIntersectionType(Tetra* t);

	/// cell movement state
	struct Movement
	{
		enum Type{VERTEX_FACE = 0,EDGE_EDGE = 1};

		Tetra* t;
		Type type; // VERTEX_FACE or EDGE_EDGE

		// v-f
		int v;

		// e-e
		int ea, eb; // edge (t->v(ea),t->v(eb)) collides with other edge
	};

	/// perform the movement given with details move
	/// PRE: movement is first to occur (i.e., no simultaneous moves have occurred)
	void performMove(const Movement& move);

	// mark the simulation as unstable
	void setUnstable();


private:
	double mSuggestedStepSize;
	double mLastStepSize;
	double mTime;
	int mSteps;
	unsigned int mCollisionInterval;
	SimulationState mState;
	std::string mErrorMessage;

	/// if an error occurs and mContinueOnError=true then the simulator will
	/// attempt to continue after (hackily) fixing the error
	bool mContinueOnError;

	Movement mFirstMove;
	// current location in algorithm, corresponds to mProperties["step"], empty if not a substep
	Step mCurrentStep;

	OWorld* mOWorld;
	Organism* mOrganism;
	Collision mCollision;

	std::list<Mesh*> mStaticMeshes;
};

#endif /* SDSSIMULATION_H_ */
