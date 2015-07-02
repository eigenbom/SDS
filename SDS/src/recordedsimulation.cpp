#include "recordedsimulation.h"
#include "bstreamable.h"

#include <sstream>
#include <iostream>
#include <cstring>
#include <boost/foreach.hpp>

/**
 * Simulation File
 *
 * Is a binary file
 *
 * Format:
 * MAGIC NUMBER
 * VERSION
 *
 * 0 FRAME0
 * 1 FRAME1
 * 2 FRAME2
 * ...
 * N FRAMEN
 * MAGICNUMBER  <-- magic number again verifies that the file is complete
 * <EOF>
 */

const int RecordedSimulation::MAGICNUMBER;
const int RecordedSimulation::VERSION;

std::string RecordedSimulation::version()
{
	std::ostringstream oss;
	oss << "v" << mSimulationVersionNumber;
	return oss.str();
}

std::string RecordedSimulation::filename()
{
	return mSimulationFile;
}

std::string RecordedSimulation::date()
{
	const char* time = std::ctime(&mSimulationDate);
	return std::string(time,std::strlen(time)-1);
}

IRecordedSimulation::IRecordedSimulation(std::string filename) 
:mCancelLoading(false)
,mNumberOfBytesRead(0)
{
	mSimulationFile = filename;
	std::string msgPrefix = std::string("File \"" + filename + "\"");
	mFile.open(filename.c_str(),std::ios::binary);
	if (!mFile)
		throw(msgPrefix + " cannot be found/read.");

	// calculate the size of the file
	mFileStart = mFile.tellg();
	mFile.seekg(0,std::ios::end);
	long end = mFile.tellg();
	mSizeOfFileInBytes = end - mFileStart;
	mFile.seekg(mFileStart);

	int magicnumber;
	int version;
	read(mFile,magicnumber);
	if (magicnumber!=MAGICNUMBER)
	{
		mFile.close();
		throw(msgPrefix + " is not a simulation file.");
	}

	read(mFile,version);
	if (version!=VERSION)
	{
		mFile.close();
		std::ostringstream oss;
		oss << msgPrefix << " is the wrong version. It is version " << version << ", but should be version " << VERSION;
		throw(oss.str());
	}
	mSimulationVersionNumber = version;
}

void IRecordedSimulation::startLoading() 
{
	// at this point the file seems to have checked out fine, so lets load it!

	// read file
	// global simulation information
	// including ...
	// time
	read(mFile,mSimulationDate);
	std::cout << "IRecordedSimulation: " << "Read header" << std::endl;

	// parameters of the process model
	mProcessModel = ProcessModel::readStatic(mFile);

	std::cout << "IRecordedSimulation: Read static simulation info " << std::endl;
	std::cout << "IRecordedSimulation: " << "Reading frames..." << std::endl;
	// read in frame by frame
	int frameCounter = 0;
	mBadFrameRead = false;
	while(not (mCancelLoading or mFile.eof()))
	{
		int frameNumber;
		read(mFile,frameNumber);
		if (frameNumber==frameCounter)
		{
			IFrame* frame = new IFrame(mFile,mProcessModel);
			if (!frame->isValid())
			{
				mBadFrameRead = true;
				delete frame;
			}
			else
			{
				mFrames.push_back(frame);
			}
		}
		else
		{
			mBadFrameRead = true;
		}

		if (mBadFrameRead)
			break;

		frameCounter++;
		mNumberOfBytesRead = (long)mFile.tellg() - mFileStart;
	}

	mFile.close();
	mNumberOfFrames = frameCounter;
	if (mCancelLoading) mBadFrameRead = true;

	std::cout << "IRecordedSimulation: " << "Read " << mNumberOfFrames << " frames successfully.";
}

void IRecordedSimulation::cancelLoading()
{
	mCancelLoading = true;
}

IRecordedSimulation::~IRecordedSimulation()
{
	BOOST_FOREACH(IFrame* f, mFrames)
		delete f;
}

ORecordedSimulation::ORecordedSimulation(std::string filename, SDSSimulation* sim, Organism* o)
:mFile(filename.c_str(),std::ios::binary)
,numFrames(0),mOrganism(o)
,mSimulation(sim)
{
	assert(mFile.is_open());

	// write the header information
	write(mFile,MAGICNUMBER);
	write(mFile,VERSION);

	// write the global simulation information
	// including...
	// time
	mSimulationDate = std::time(NULL);
	write(mFile,mSimulationDate);

	// write static info about the process model
	mOrganism->processModel()->writeStatic(mFile);
}

/**
 * Append the frame to the output file.
 */
void ORecordedSimulation::writeFrame(OFrame& f)
{
	// for each frame, we write the frame number followed by the frame data
	write(mFile,numFrames);
	f.write(mSimulation,mFile);
	numFrames++;
}

void ORecordedSimulation::writeCurrentState()
{
	OFrame of(mOrganism);
	writeFrame(of);
}

void ORecordedSimulation::finalise()
{
	// write the footer information, then close
	write(mFile,MAGICNUMBER);
	mFile.close();
}
