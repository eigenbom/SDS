#ifndef PHYSICS_H
#define PHYSICS_H

#include <string>

#include "mesh.h"

class Edge;
class Face;
class Tetra;

/**
 * Physics handles the simulation of a mesh object.
 * It performs basic verlet integration with an adaptive step size
 * (see http://www.gamedev.net/reference/articles/article2200.asp )
 *
 * The primary method is step() which takes a single time-step forward.
 * Different parameters of the simulation can be set and modified.
 *
 */
class Physics
{
	public:

	/**
	 * Take a step forward by dt time units.
	 * This method simply calls calculateForces() and the takeVerletStep() in sequence.
	 *
	 * If checkForStability = true then the method checks if the mesh is becoming unstable
	 * and returns false if it has.
	 */
	static bool step(Mesh* m, double dt, bool checkForStability = false);

	static void zeroForces(Mesh* m);
	static void calculateForces(Mesh* m, bool zeroForcesFirst=true, bool dirty=false);
	static bool takeVerletStep(Mesh* m, double dt, bool checkForStability = false, bool dirty=false);

	/**
	 * Goes through all the edges in m and sets their spring coefficients to the correct values.
	 */
	static void SetAllEdgeSpringStrengths(Mesh* m);

	/**
	 * Notify the simulator that the previous step was modified.
	 * This is called by e.g., SDSSimulation when it rewinds the simulation to handle
	 * cell movement.
	 */
	static void setLastStepSize(double dt){Physics::sLastStepSize = dt;}

	/**
	 * DENSITY: The density multiplier of the mesh.
	 * VISCOUS_DAMPING_COEFFICIENT: The damping applied to moving particles 0 = no damping, 1 = solid fluid (no movement of particles!)
	 * kD: Default spring strength
	 * kSM: Surfac strength multiplier (edges on the surface has kSM*kD spring strength)
	 * kDamp: Default spring damping
	 * kV: Default simplex spring strength
	 * GRAVITY: Gravity!
	 */
	static double DENSITY;
	static double VISCOSITY;
	static double kD;
	static double kSM;
	static double kDamp;
	static double kV;
	static double GRAVITY;

	protected:

	struct FDInfo
	{
		double length;
		double extension;
		double energy;
	};

	struct FVInfo
	{
		double vol;
		double energy;
	};

	// compute spring force from edge, returns false if a bad number is computed
	static bool FD(Edge* e, FDInfo* fd = NULL);
	// compute spring force from tet
	static bool FV(Tetra* t, FVInfo* fv = NULL);

	// string-based parameter interface to the physics parameters
	static bool setParam(std::string param, std::string value);

	static void setKD(double d){kD = d;}
	static void setKDamp(double d){kDamp = d;}
	static void setKV(double d){kV = d;}
	static void setGravity(double g){GRAVITY = g;}

	static double sLastStepSize;
};

#endif

