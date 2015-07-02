#ifndef RECORDED_SIMULATION_H
#define RECORDED_SIMULATION_H

#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <ctime>

#include "frame.h"
#include "sdssimulation.h"
#include "organism.h"
#include "processmodel.h"

/**
 * Provides facilities for saving and loading a simulation.
 * IRecordedSimulation and ORecordedSimulation provide the input and output versions
 *
 * BP110309
 */
class RecordedSimulation
{
public:
	std::string version();
	std::string filename();
	std::string date();

protected:
	static const int MAGICNUMBER = 26051981;
	static const int VERSION = 1;
protected:
	int mSimulationVersionNumber; 	// a number indicating the version of the simulation file
	std::string mSimulationFile;  	// the associated simulation filename
	time_t mSimulationDate;

	// simulation information
	int mNumberOfFrames;

};


/**
 * @deprecated
 */
class IRecordedSimulation: public RecordedSimulation
{
public:
	/**
	 * Load a RecordedSimulation from a simulation file on disk.
	 * startLoading must be called to start the file streaming process
	 *
	 * Normal use case is to call startLoading in another thread, and repeatedly
	 * poll bytesRead()/totalBytes() to determine the percentage of the file loaded.
	 *
	 */
	IRecordedSimulation(std::string filename);
	long totalBytes(){return mSizeOfFileInBytes;}
	long bytesRead(){return mNumberOfBytesRead;}
	void startLoading() ;
	void cancelLoading();

	~IRecordedSimulation();
	// IFrame* getFrame(){}

	bool isComplete(){return !mBadFrameRead;}
	const std::vector<IFrame*>& frames(){return mFrames;}

private:
	std::ifstream mFile;
	std::vector<IFrame*> mFrames;
	bool mBadFrameRead;
	ProcessModel* mProcessModel;

	bool mCancelLoading;
	long mSizeOfFileInBytes;
	long mFileStart;
	long mNumberOfBytesRead;
};

class ORecordedSimulation: public RecordedSimulation
{
public:
	/**
	 * Create a new blank RecordedSimulation interface to write to a file.
	 */
	ORecordedSimulation(std::string filename, SDSSimulation* sim, Organism* o);
	/**
	 * Append the frame to the output file.
	 */
	void writeFrame(OFrame&);


	/**
	 * Construct a frame from the current simulation state then write it.
	 */
	void writeCurrentState();

	/**
	 * finalise() should be called when finished
	 * writes up the last bits of info and closes the file
	 */
	void finalise();

protected:
	std::ofstream mFile;
	int numFrames;
	Organism* mOrganism;
	SDSSimulation* mSimulation;
};

#endif
