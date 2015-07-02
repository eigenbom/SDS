/**
 * Output: Hooks into mesh simulation to output frames in various formats.
 * See TPSOffline for examples of different output modules.
 *
 * BP 24.01.08
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include <string>

#include "mesh.h"
#include "aabb.h"

class MeshSimulation;
class Output
{
	public:

	Output():mN(1),mNumFrames(0){}
	virtual ~Output(){}

	// all output modules have access to mesh and world bounds
	// and maxframe info etc.
	virtual void setMesh(Mesh* m){ mMesh = m; }
	virtual void setBounds(AABB a){ mAABB = a; }
	virtual void setNumFrames(int n){ mNumFrames = n; }
	virtual void setStepper(MeshSimulation* s){ mStepper = s; }

	// interface to parameterised modules
	virtual bool setParameter(std::string name, std::string value);

	// output functions,
	//
	// start: called before the first simulation step
	// output: is called every n frames, where n is dictated by the output module
	// end: called after the last simulation step
	// n: number of frames between each call of output

	virtual bool start(){return true;}
	virtual bool output(int frameNumber) = 0;
	virtual bool end(){return true;}
	virtual int n() {return mN;}

	protected:

	Mesh* mMesh;
	AABB mAABB;
	int mN; // frame interval between calls
	int mNumFrames;
	MeshSimulation* mStepper;
};

#endif
