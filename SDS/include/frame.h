#ifndef FRAME_H
#define FRAME_H

#include <ostream>
#include <istream>

#include "mesh.h"
#include "organism.h"
#include "processmodel.h"
#include "sdssimulation.h"

/**
 * Frame encapsulates the state at a particular time step of a simulation.
 * Depending on the FrameSpecification, frames may contain different pieces of data.
 * At a minimum, a frame will contain some basic info and a mesh.
 *
 * Loading frames...
 *   SDSPlayer loads and stores all frames in order to examine them all.
 *   It requires that "mesh" and "organism" are both stored in the frame.
 *
 *   Other hypothetical scenarios:
 *   EnergyPlotter may go through each frame and only extract the energy info segment, if it exists.
 *
 *   BlenderUtil may load every second frame and extract only the "mesh" and "texcoord" data.
 *
 * Storing frames...
 *   Depending on the usage, we may need to store different bits of data in a simulation run. For example...
 *
 *   SDSSimulator may give options to the user to select which components he wants to monitor and output.
 *
 *   BlenderUtil may only require that "mesh" and "texcoord" data is generated.
 *
 * These scenarios imply that for each usecase custom serialisation logic is needed, but this should
 * be quite high-level and hence some supporting routines are necessary.
 *
 * Use IFrame and OFrame to create the different versions required by IRecordedSim and ORecordedSim
 */
class Frame
{
public:

protected:
	double mTime;
	// mOrganismState
	// mCollisionState
	// etc...
};

class IFrame: public Frame
{
public:
	IFrame(std::istream&,ProcessModel*);
	~IFrame();
	bool isValid(){return mValid;}

	double time(){return mTime;}
	double frameNumber(){return mFrameNumber;}
	Mesh* mesh(); // return the mesh associated with the organism state of this frame..
	Organism* organism(){return mOrganism;}
private:
	// contains a copy of mesh state, organism state, etc...
	bool mValid;

	double mTime;
	int mFrameNumber;
	Organism* mOrganism;
};

class OFrame: public Frame
{
public:
	OFrame(Organism*);
	void write(SDSSimulation*sim, std::ostream&);
private:
	// contains a ptr to mesh, organism, etc. used temporarily as a transfer structure to ORecordedSim
	Organism* mOrganism;
};

#endif
