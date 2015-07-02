/*
 * simulationio.h
 *
 *  Created on: 15/10/2009
 *      Author: ben
 */

#ifndef SIMULATIONIO_H_
#define SIMULATIONIO_H_

#include <string>
#include <list>
#include <fstream>
#include <map>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

#include "aabb.h"

#include "mesh.h"
#include "processmodel.h"
//#include "sdssimulation.h"

#include "segmentio.h"

#include "libconfig.h++"

class SimulationIO_Base
{
public:
	static const unsigned int MAGICNUMBER = 26051981;
	static const unsigned int VERSION = 2; // incremented 15/02/10

	struct StaticMesh
	{
		Vector3d pos;
		double sx, sy, sz;
		std::string filePrefix;

		StaticMesh(std::string filePrefix);
	};

	struct SimulationHeader
	{
		double t;
		double dt;
		int numFrames; // if numFrames is non existant or -1 then we don't know how many frames there are
		int collisionInterval;
		AABB worldBounds;
		double gravity;
		double kD;
		double kSM;
		double kV;
		double kDamp;
		double density;
		double viscosity; // from 0 to 1
		std::string frameDataFileName;
		boost::posix_time::ptime time;
		std::string comments;
		ProcessModel* processModel; // may be NULL
		std::list<StaticMesh> staticMeshes;

		SimulationHeader(); // loads default values

		std::string getAbsolutePath(std::string filename) const;

	// derived:
		boost::filesystem::path mFilePath;
	};

	struct FrameHeader
	{
		unsigned int sizeInBytes; //
		unsigned int number;
		double time;
		unsigned int step;

		static int size(){return 3*sizeof(unsigned int) + sizeof(double);}
	};
};

class SimulationWriter: public SimulationIO_Base
{
public:
	/**
	 * Write the simulation header out to a file in libconfig format.
	 */
	static void writeSimulationHeader(std::string file, SimulationHeader& sh, std::list<SegmentWriter*> sws = std::list<SegmentWriter*>(), std::list<SegmentWriter*> staticSegments = std::list<SegmentWriter*>());
	static void writeStaticSegments(std::ostream& out, std::list<SegmentWriter*> segmentWriters);
	static void writeFrame(std::ostream& out, int number, double time, int step, std::list<SegmentWriter*> segmentWriters);

	static void writeFrameDataHeader(std::ostream& o);
	static void writeFrameHeader(std::ostream& o, FrameHeader& fh);
	static void writeSegment(std::ostream& o, SegmentWriter& sw);
};

/**
 * Handles loading of simulation data.
 * This class has some useful utilities for extracting specific bits of info from a simulation file.
 *
 * A simulation loader is created with a specific file. It parses the header file and
 * stores the basic simulation information.
 *
 * Simulation Loader provides an interface to the frame data through
 * currentFrame() and nextFrame(). If a frame is current
 * then its basic info can be accessed, but also segment loaders can be used to
 * load segment data. It is the user's responsibility to make sure the segment loaders
 * are set up correctly beforehand and contain the proper data.
 *
 * For example, an OrganismSegmentLoader first needs to have an
 * empty Organism* and a Mesh* built by a MeshSegmentLoader.
 */
class SimulationLoader: public SimulationIO_Base
{
public:
	class LoadException: public std::exception
	{
	private:
		std::string _why;
	public:
		LoadException(std::string why):_why(why){}
		virtual ~LoadException() throw() {}
		std::string why(){return _why;}
	};

public:
	SimulationLoader(std::string header_filename) throw(SimulationLoader::LoadException);
	virtual ~SimulationLoader();

	SimulationHeader simulationHeader(){return mSimulationHeader;}

	/**
	 * A simulation header may not specify how many frames it has. Call this function iterate through
	 * the binary data and count all the valid frames.
	 *
	 * After calling it, the current frame is reset to 0.
	 */
	int countFrames();

	/**
	 * True if a particular segment type exists.
	 */
	bool hasSegment(std::string type);

	/**
	 * Returns a list of all segments (types).
	 */
	std::list<std::string> segments();

	/**
	 * Returns a list of all static segments (types).
	 */
	std::list<std::string> staticSegments();

	/**
	 * Load all the static segments.
	 *
	 * segmentLoaders must be in the same order as the static segment list returned by staticSegments().
	 */
	bool loadStaticSegments(std::list<SegmentLoader*> segmentLoaders);

	/**
	 * Given a segment loader, initialise it with its parameters.
	 */
	bool initialiseSegmentLoader(SegmentLoader* sl);

	/**
	 * Load the binary data file and set the current frame to the first frame.
	 * Returns false if we can't.
	 *
	 * DEPRECATED: Use loadData(), then loadStaticSegments()
	 */
	bool loadFrameData();

	/**
	 * Load the binary data file and get ready to laod the static segments if there are any. Otherwise
	 * set the current frame to 0.
	 *
	 * Returns false on fail.
	 */
	bool loadData();

	/**
	 * Access the current frame.
	 *
	 * NOTE: You must call loadData() and then loadStaticSegments() to set the first frame before calling this.
	 * Otherwise currentFrame() will return nonsense.
	 */
	const SimulationIO_Base::FrameHeader& currentFrame() const;
	/**
	 * Iterate to the next frame.
	 * Returns true if there is another frame, or false if there are no more frames.
	 */
	bool nextFrame();

	/**
	 * Choose a specific frame.
	 * Returns false if there is a problem.
	 */
	bool setFrame(int f);

	/**
	 * For the current frame, use the segment loader to extract the segment data.
	 * Should be called when in a frame.
	 *
	 * WARNING:
	 *   Throws up if the segment doesn't exist or the segment loader fails for some reason.
	 *
	 */
	void loadSegment(SegmentLoader* sl) ;

	std::string getAbsolutePath(std::string filename) const;

private:
	libconfig::Setting& getFrameSpecification(std::string type);
	libconfig::Setting& getStaticSpecification(int index);

	bool skipStaticSegment();
	void readFrameHeader();
	// void setCurrentFrameToZero();

private:
	// the simulation header file
	SimulationHeader mSimulationHeader;
	libconfig::Config mConfig;
	std::map<std::string,int> mSegmentIndexes;
	boost::filesystem::path mFilePath;
	std::list<std::string> mStaticSegmentTypes;

	// static data, frame data, various metrics, pointers and bits of frame data
	unsigned int mVersion;
	std::ifstream* mFrameData;

	bool mAtStartOfStaticSegment; // either at start of static segment or somewhere in frame data
	bool mHasStaticSegment;
	unsigned long mSizeOfStaticSegmentInBytes;
	std::ifstream::pos_type mStaticSegmentBegin;

	unsigned long mSizeOfFrameDataInBytes;
	std::ifstream::pos_type mStartPos;
	std::ifstream::pos_type mCurrentFramePos;
	std::ifstream::pos_type mStartSegmentPos;
	std::ifstream::pos_type mCurrentSegmentPos;
	FrameHeader mCurrentFrame;
};

#endif /* SDSSERIAL_H_ */
