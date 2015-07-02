/*
 * simulationio.cpp
 *
 *  Created on: 15/10/2009
 *      Author: ben
 */

#include "simulationio.h"
#include "bstreamable.h"

#include <stdexcept>
#include <boost/foreach.hpp>

#include "libconfig.h++"
using namespace libconfig;

#ifdef DEBUG_BINARY
	#define DUMP(what) {std::cout << what << std::endl; }
#else
	#define DUMP(what) ;
#endif

const unsigned int SimulationIO_Base::MAGICNUMBER;
const unsigned int SimulationIO_Base::VERSION;

SimulationIO_Base::StaticMesh::StaticMesh(std::string prefix)
:filePrefix(prefix),pos(),sx(1),sy(1),sz(1)
 {}

SimulationIO_Base::SimulationHeader::SimulationHeader()
:t(0),dt(0),numFrames(-1)
,collisionInterval(1),worldBounds()
,gravity(0),kD(0),kSM(1),kV(0),kDamp(0),density(1),viscosity(0.01)
,frameDataFileName("")
,time(boost::posix_time::second_clock::local_time())
,comments("")
,processModel(NULL)
,mFilePath()
{}

std::string SimulationIO_Base::SimulationHeader::getAbsolutePath(std::string filename) const
{
	return (mFilePath / filename).string();
}

void SimulationWriter::writeSimulationHeader(std::string file, SimulationHeader& sh, std::list<SegmentWriter*> sws, std::list<SegmentWriter*> staticSegments)
{
	Config cfg;
	std::ostringstream oss;
	oss << VERSION;
	cfg.getRoot().add("version", libconfig::Setting::TypeString) = oss.str();

	Setting& sim = cfg.getRoot().add("simulation", Setting::TypeGroup);
	sim.add("t",Setting::TypeFloat) = sh.t;
	sim.add("dt",Setting::TypeFloat) = sh.dt;
	sim.add("numFrames",Setting::TypeInt) = sh.numFrames;
	sim.add("collisionInterval",Setting::TypeInt) = sh.collisionInterval;
	sim.add("gravity",Setting::TypeFloat) = sh.gravity;
	sim.add("kD",Setting::TypeFloat) = sh.kD;
	sim.add("kSM", Setting::TypeFloat) = sh.kSM;
	sim.add("kV",Setting::TypeFloat) = sh.kV;
	sim.add("kDamp",Setting::TypeFloat) = sh.kDamp;
	sim.add("density",Setting::TypeFloat) = sh.density;
	sim.add("viscosity",Setting::TypeFloat) = sh.viscosity;
	sim.add("framedata", Setting::TypeString) = sh.frameDataFileName;
	sim.add("time",Setting::TypeString) = boost::posix_time::to_simple_string(sh.time);
	sim.add("comments", Setting::TypeString) = sh.comments;

	Setting& worldInfo = sim.add("worldInfo",Setting::TypeGroup);
		Setting& bounds = worldInfo.add("bounds",Setting::TypeGroup);
			bounds.add("x",Setting::TypeFloat) = sh.worldBounds.pts()[0];
			bounds.add("y",Setting::TypeFloat) = sh.worldBounds.pts()[1];
			bounds.add("z",Setting::TypeFloat) = sh.worldBounds.pts()[2];
			bounds.add("dx",Setting::TypeFloat) = sh.worldBounds.dx();
			bounds.add("dy",Setting::TypeFloat) = sh.worldBounds.dy();
			bounds.add("dz",Setting::TypeFloat) = sh.worldBounds.dz();
		//worldInfo.add("gravity",Setting::TypeFloat) = sh.gravity;

	// new static mesh support
	if (not staticSegments.empty())
	{
		Setting& frameSpec = worldInfo.add("staticSpecification", Setting::TypeList);
		BOOST_FOREACH(SegmentWriter* sw, staticSegments)
		{
			Setting& set = frameSpec.add(Setting::TypeGroup);
			sw->setSetting(set);
		}
	}

	if (sh.processModel!=NULL)
	{
		Setting& processModelSetting = sim.add("processModel",Setting::TypeGroup);
		sh.processModel->saveToSetting(processModelSetting);
	}

	// old static mesh support
	if (not sh.staticMeshes.empty())
	{
		Setting& staticSetting = sim.add("static",Setting::TypeList);
		BOOST_FOREACH(StaticMesh sm, sh.staticMeshes)
		{
			Setting& mesh = staticSetting.add("mesh",Setting::TypeGroup);
			mesh.add("x",Setting::TypeFloat) = sm.pos.x();
			mesh.add("y",Setting::TypeFloat) = sm.pos.y();
			mesh.add("z",Setting::TypeFloat) = sm.pos.z();
			mesh.add("sx",Setting::TypeFloat) = sm.sx;
			mesh.add("sy",Setting::TypeFloat) = sm.sy;
			mesh.add("sz",Setting::TypeFloat) = sm.sz;
			mesh.add("filePrefix",Setting::TypeString) = sm.filePrefix;
		}
	}

	Setting& frameSpec = sim.add("frameSpecification",Setting::TypeList);
	BOOST_FOREACH(SegmentWriter* sw, sws)
	{
		Setting& set = frameSpec.add(Setting::TypeGroup);
		sw->setSetting(set);
	}

	cfg.writeFile(file.c_str());
}

void SimulationWriter::writeStaticSegments(std::ostream& out, std::list<SegmentWriter*> segmentWriters)
{
	if (segmentWriters.empty())
	{
		return;
	}

	// first compute static segment size by writing the segment data to a temporary buffer
	std::ostringstream oss(std::ios::binary);
	BOOST_FOREACH(SegmentWriter* sw, segmentWriters)
		sw->write(oss);
	std::string os = oss.str();
	unsigned long size = (unsigned long)(os.length());
	::write(out,size);
	out.write(os.c_str(),size);
}

void SimulationWriter::writeFrameDataHeader(std::ostream& o)
{
	write(o,MAGICNUMBER);
	write(o,VERSION);
}

void SimulationWriter::writeFrameHeader(std::ostream& o, FrameHeader& fh)
{
	write(o,fh.sizeInBytes);
	write(o,fh.number);
	write(o,fh.time);
	write(o,fh.step);
}

void SimulationWriter::writeFrame(std::ostream& out, int number, double time, int step, std::list<SegmentWriter*> segmentWriters)
{
    DUMP("writeFrame");
	// first compute frame size by writing the segment data to a temporary buffer
	std::ostringstream oss(std::ios::binary);
	BOOST_FOREACH(SegmentWriter* sw, segmentWriters)
		SimulationWriter::writeSegment(oss, *sw);
	std::string os = oss.str();

	// then construct the appropriate header
	SimulationIO_Base::FrameHeader fhdr;
	//std::cout << "Frame header size: " << SimulationIO_Base::FrameHeader::size() << "b\n";

	fhdr.sizeInBytes = (unsigned int)(os.length());
	fhdr.number = 0;
	fhdr.time = 0;
	fhdr.step = 0;

	SimulationWriter::writeFrameHeader(out,fhdr);
	out.write(os.c_str(), os.length());

    DUMP("writeFrameHeader.sizeInBytes =  " << fhdr.sizeInBytes);
}


void SimulationWriter::writeSegment(std::ostream& o, SegmentWriter& sw)
{
    DUMP("writeSegment");

	// make a temporary buffer to write into
	std::ostringstream oss(std::ios::binary);
	sw.write(oss);
	// NOW WRITE THE SIZE OF THE BUFFER, FOLLOWED BY THE BUFFER ITSELF TO THE OUTPUT STREAM
	std::string os = oss.str();
    unsigned long len = (unsigned long)(os.length());

    // unsigned long segmentStart = o.tellg();
    // DUMP("startSegmentPos = " << segmentStart);

	write(o,len); // unsigned long)os.length());

    /*
    std::ostringstream oss2(std::ios::binary);
    write(oss2,len);            

    std::istringstream iss(oss2.str(), std::ios::binary);
    unsigned long len2 = 666;
    read(iss,len2);

    DUMP("segmentLength as str = [" << len2 << "]");
    */

    DUMP("segmentLength = " << len); // (unsigned long)os.length());

	o.write(os.c_str(),len); // os.length());

    DUMP("dumped entire segment");
}

/*********************************************************************************************
 *********************************************************************************************
 *********************************************************************************************
 *********************************************************************************************/

SimulationLoader::SimulationLoader(std::string header_filename) throw(SimulationLoader::LoadException)
:mSimulationHeader(),mConfig(),mFrameData(NULL)
{
	try
	{
		if (!boost::filesystem::exists(header_filename))
		{
			std::ostringstream oss;
			oss << "Cannot load " << header_filename;
			throw SimulationLoader::LoadException(oss.str());
		}

		mFilePath = boost::filesystem::path(header_filename).parent_path();
		//std::cout << "mFilePath: " << mFilePath << "\n";
		mSimulationHeader.mFilePath = mFilePath;
		//std::cout << "header.mFilePath: " << mSimulationHeader.mFilePath << "\n";

		mConfig.readFile(header_filename.c_str());
		mConfig.setAutoConvert(true);

		Setting& sim = mConfig.lookup("simulation");

		std::string timestamp;

		if (
			sim.lookupValue("t",mSimulationHeader.t) and
			sim.lookupValue("dt",mSimulationHeader.dt) and
			sim.lookupValue("collisionInterval",mSimulationHeader.collisionInterval) and
			sim.lookupValue("gravity",mSimulationHeader.gravity) and
			sim.lookupValue("kD", mSimulationHeader.kD) and
			sim.lookupValue("kV", mSimulationHeader.kV) and
			sim.lookupValue("kDamp", mSimulationHeader.kDamp) and
			sim.lookupValue("density", mSimulationHeader.density) and
			sim.lookupValue("framedata",mSimulationHeader.frameDataFileName) and
			sim.lookupValue("time",timestamp) and
			sim.lookupValue("comments",mSimulationHeader.comments))
		{
			// parse time
			mSimulationHeader.time = boost::posix_time::time_from_string(timestamp);

			sim.lookupValue("kSM", mSimulationHeader.kSM);
		}
		else
		{
			throw SimulationLoader::LoadException("A parameter is missing!");
		}

		if (sim.exists("viscosity"))
		{
			sim.lookupValue("viscosity",mSimulationHeader.viscosity);
		}

		if (sim.exists("numFrames"))
		{
			sim.lookupValue("numFrames",mSimulationHeader.numFrames);
		}

		if (sim.exists("worldInfo"))
		{
			double x, y, z, dx, dy, dz;
			if (mConfig.lookupValue("simulation.worldInfo.bounds.x",x) &&
				mConfig.lookupValue("simulation.worldInfo.bounds.y",y) &&
				mConfig.lookupValue("simulation.worldInfo.bounds.z",z) &&
				mConfig.lookupValue("simulation.worldInfo.bounds.dx",dx) &&
				mConfig.lookupValue("simulation.worldInfo.bounds.dy",dy) &&
				mConfig.lookupValue("simulation.worldInfo.bounds.dz",dz))
			{
				mSimulationHeader.worldBounds = AABB(x,y,z,x+dx,y+dy,z+dz);
			}
			else
				std::cout << "Couldn't read in world bounds values, wrong type?" << std::endl;

			try
			{
				Setting& staticSpecification = mConfig.lookup("simulation.worldInfo.staticSpecification");

				if (!staticSpecification.isList())
				{
					throw SimulationLoader::LoadException("simulation.worldInfo.staticSpecification is not a list");
				}
				for(int i=0; i<staticSpecification.getLength(); i++)
				{
					Setting& f = staticSpecification[i];
					std::string type;
					if (f.lookupValue("type",type))
					{
						mStaticSegmentTypes.push_back(type);
					}
					else
					{
						throw SimulationLoader::LoadException("simulation.worldInfo.staticSpecification item has no type");
					}
				}
			}
			catch(SettingNotFoundException& snfe)
			{
				// pass
			}
		}
		else
		{
			std::cout << "No world bounds!" << std::endl;
		}

		/// process info
		if (sim.exists("processModel"))
		{
			Setting& processSpecification = mConfig.lookup("simulation.processModel");

			try
			{
				mSimulationHeader.processModel = ProcessModel::loadFromSetting(processSpecification);

				if (mSimulationHeader.processModel==NULL)
				{
					throw SimulationLoader::LoadException("simulation.processModel is incorrectly formatted. Is the type missing?");
				}

			}
			catch(ProcessModel::NoProcessModelException& e)
			{
				std::string errorMessage = std::string(e.what()) + "(Attempted to load a process model that is not available.)";
				std::cerr << "Warning: " << errorMessage << std::endl;
				std::cerr << "Continuing with a NULL process model." << std::endl;

				mSimulationHeader.processModel = NULL;
				// throw SimulationLoader::LoadException(errorMessage);
			}
		}
		else
		{
			mSimulationHeader.processModel = NULL;
		}

		if (sim.exists("static"))
		{
			Setting& staticSetting = mConfig.lookup("simulation.static");
			if (!staticSetting.isList())
			{
				std::cerr << "Expected simulation.static to be a list. It isn't, so I'm skipping it..." << std::endl;
			}
			else
			{
				for(int i=0;i<staticSetting.getLength();i++)
				{
					Setting& mesh = staticSetting[i];

					Vector3d pos;
					double x, y, z;
					if (mesh.lookupValue("x",x) &&
						mesh.lookupValue("y",y) &&
						mesh.lookupValue("z",z))
					{
						pos = Vector3d(x,y,z);
					}
					else
					{
						pos = Vector3d::ZERO;
					}

					double sx, sy, sz;
					if (mesh.lookupValue("sx",sx) &&
						mesh.lookupValue("sy",sy) &&
						mesh.lookupValue("sz",sz))
					{ }
					else
					{
						sx = 1;
						sy = 1;
						sz = 1;
					}

					std::string prefix = mesh["filePrefix"];
					StaticMesh sm(prefix);
					sm.pos = pos;
					sm.sx = sx;
					sm.sy = sy;
					sm.sz = sz;
					mSimulationHeader.staticMeshes.push_back(sm);
				}
			}
		}

		Setting& frameSpecification = mConfig.lookup("simulation.frameSpecification");
		if (!frameSpecification.isList())
		{
			throw SimulationLoader::LoadException("simulation.frameSpecification is not a list");
		}
		for(int i=0; i<frameSpecification.getLength(); i++)
		{
			Setting& f = frameSpecification[i];
			std::string type;
			if (f.lookupValue("type",type))
				mSegmentIndexes[type] = i;
		}
	}
	catch (SettingNotFoundException& se)
	{
		std::ostringstream oss;
		oss << "Setting not found: " << se.getPath();
		throw SimulationLoader::LoadException(oss.str());
	}
	catch (SettingTypeException& ste)
	{
		std::ostringstream oss;
		oss << "Setting type error: " << ste.getPath();
		throw SimulationLoader::LoadException(oss.str());
	}
	catch (ParseException& pe)
	{
		std::ostringstream oss;
		oss << "Parse error at (" << header_filename << ":" << pe.getLine() << "). " << pe.getError();
		throw SimulationLoader::LoadException(oss.str());
	}
	catch (FileIOException& fe)
	{
		std::ostringstream oss;
		oss << "Cannot load " << header_filename;
		throw SimulationLoader::LoadException(oss.str());
	}
}

SimulationLoader::~SimulationLoader()
{
	delete mFrameData;
}

int SimulationLoader::countFrames()
{
	if (!loadData()) return 0;

	// skip to first frame
	if (mHasStaticSegment and not skipStaticSegment()) return 0;

	int count = 1;
	while (nextFrame())
		count++;

	// then reset the data pointer
	loadData();

	return count;
}

/**
 * True if a particular segment type is included in the simulation data.
 */
bool SimulationLoader::hasSegment(std::string type)
{
	for(int i=0;i<mConfig.lookup("simulation.frameSpecification").getLength();i++)
	{
		Setting& f = mConfig.lookup("simulation.frameSpecification")[i];
		std::string _type;
		if (f.lookupValue("type",_type))
		{
			if (type==_type)
			{
				return true;
			}
		}
	}
	return false;
}

/**
 * Returns a list of all segment (types).
 */
std::list<std::string> SimulationLoader::segments()
{
	std::list<std::string> segmentList;
	Setting& framespec = mConfig.lookup("simulation.frameSpecification");
	for(int i=0;i<framespec.getLength();i++)
	{
		Setting& segment = framespec[i];
		std::string type;
		segment.lookupValue("type",type);
		segmentList.push_back(type);
	}
	return segmentList;
}

std::list<std::string> SimulationLoader::staticSegments()
{
	return this->mStaticSegmentTypes;
}

/**
 * Load all the static segments.
 *
 * segmentLoaders must be in the same order as the static segment list returned by staticSegments().
 */
bool SimulationLoader::loadStaticSegments(std::list<SegmentLoader*> segmentLoaders)
{
	// make sure we are at the start of the file
	if (not mAtStartOfStaticSegment)
	{
		mFrameData->seekg(mStaticSegmentBegin);
		if (!*mFrameData) return false;
	}

	int i = 0;
	BOOST_FOREACH(SegmentLoader* sl, segmentLoaders)
	{
		Setting& s = getStaticSpecification(i);
		if (not sl->initialise(s))
		{
			std::cerr << "Error initialising static segment loader \"" << sl->getType() << "\" of index " << i << std::endl;
			return false;
		}

		bool success = sl->load(*mFrameData);
		if (!success)
		{
			std::cerr << "Something went wrong when loading static segment \"" << sl->getType() << "\" of index " << i << std::endl;
			return false;
		}

		i++;
	}

	// we should now be at the start of the frame data
	mAtStartOfStaticSegment = false;
	mCurrentFramePos = mFrameData->tellg();
	readFrameHeader();
	return true;
}

/**
 * Given a segment loader, initialise it with its parameters.
 */
bool SimulationLoader::initialiseSegmentLoader(SegmentLoader* sl)
{
	Setting& s = getFrameSpecification(sl->getType());
	return sl->initialise(s);
}

const SimulationIO_Base::FrameHeader& SimulationLoader::currentFrame() const
{
	return mCurrentFrame;
}

bool SimulationLoader::nextFrame()
{
	if (mAtStartOfStaticSegment)
	{
		if (mHasStaticSegment and not skipStaticSegment()) return false;
	}

	// jump file pointer forward
	mFrameData->seekg(mCurrentFramePos);
	mFrameData->seekg((std::iostream::off_type)(mCurrentFrame.sizeInBytes + SimulationIO_Base::FrameHeader::size()),std::ios_base::cur);
	if (mFrameData->eof() or !(*mFrameData))
		return false;
	else
	{
		mCurrentFramePos = mFrameData->tellg();
		readFrameHeader();
		return ((not mFrameData->eof()) and !!(*mFrameData));
	}
}

bool SimulationLoader::setFrame(int f)
{
	if (not loadData()) return false;
	if (mHasStaticSegment and not skipStaticSegment()) return false;
	for(int i=0;i<f;i++)
		if (not nextFrame()) return false;
	return true;
}

/**
 * For the current frame, use the segment loader to extract the segment data.
 * Throws up if the segment doesn't exist or the segment loader fails for some reason.
 */
void SimulationLoader::loadSegment(SegmentLoader* sl) 
{
    DUMP("loadSegment");    

	// seek to to the start of all the segments
	// each segment starts with a uint that specifies its size in bytes
	// use this to skip segments, until we get the one we want...

	// get index of sl
	if (mSegmentIndexes.count(sl->getType())==0)
		throw(new std::runtime_error("Segment doesn't exist!"));
	int index = mSegmentIndexes[sl->getType()];

    DUMP("index = " << index);
    DUMP("startSegmentPos = " << mStartSegmentPos);

	// skip this many segments
	mFrameData->seekg(mStartSegmentPos);
	for(int i=0;i<index;i++)
	{
		unsigned long segSizeInBytes = 0;
		read(*mFrameData,segSizeInBytes);
		mFrameData->seekg((std::ifstream::off_type)segSizeInBytes, std::ios_base::cur);

        DUMP("i = " << i << ", segSizeInBytes = " << segSizeInBytes);
	}
	mCurrentSegmentPos = mFrameData->tellg();

    // READ THE SIZE OF THE BUFFER, THEN THE BUFFER ITSELF
    DUMP("segmentPos = " << mCurrentSegmentPos);

	// load bits from current segment pos
	unsigned long segSizeInBytes = 666;
	read(*mFrameData, segSizeInBytes);

    DUMP("segSizeInBytes = " << segSizeInBytes);

	sl->load(*mFrameData);
}

std::string SimulationLoader::getAbsolutePath(std::string filename) const
{
	return (mFilePath / filename).string();
}

Setting& SimulationLoader::getFrameSpecification(std::string type)
{
	for(int i=0;i<mConfig.lookup("simulation.frameSpecification").getLength();i++)
	{
		Setting& f = mConfig.lookup("simulation.frameSpecification")[i];
		std::string _type;
		if (f.lookupValue("type",_type))
		{
			if (type==_type)
			{
				return f;
			}
		}
	}
}

Setting& SimulationLoader::getStaticSpecification(int index)
{
	return mConfig.lookup("simulation.worldInfo.staticSpecification")[index];
}

bool SimulationLoader::loadFrameData()
{
	if (mFrameData)
	{
		delete mFrameData;
	}

	std::string absoluteFileName = mSimulationHeader.getAbsolutePath(mSimulationHeader.frameDataFileName);
	mFrameData = new std::ifstream(absoluteFileName.c_str(),std::ios::binary);
	if (mFrameData==NULL or not (*mFrameData))
	{
		mFrameData = NULL;
		return false;
	}
	std::ifstream& mFile = *mFrameData;

	// else, confirm magic number and version
	// calculate the size of the file
	mStartPos = mFile.tellg();
	mFile.seekg(0,std::ios::end);
	long end = mFile.tellg();
	mSizeOfFrameDataInBytes = end - mStartPos;
	mFile.seekg(mStartPos);

	unsigned int magicnumber;
	unsigned int version;
	read(mFile,magicnumber);
	if (magicnumber!=SimulationLoader::MAGICNUMBER)
	{
		mFile.close();
		mFrameData = NULL;
		std::cerr << "Magic number is incorrect!\n";
		return false;
	}

	read(mFile,version);
	if (version!=1)
	{
		mFile.close();
		mFrameData = NULL;
		std::cerr << "loadFrameData() is deprecated! Cannot use it with simulation file of version > 1. Current file version = " << version << "\n";
		return false;
	}
	mVersion = version;

	// read in the info for the first frame
	mCurrentFramePos = mFile.tellg();
	readFrameHeader();
	return true;
}

bool SimulationLoader::loadData()
{
	if (mFrameData)
	{
		delete mFrameData;
	}

	std::string absoluteFileName = mSimulationHeader.getAbsolutePath(mSimulationHeader.frameDataFileName);
	mFrameData = new std::ifstream(absoluteFileName.c_str(),std::ios::binary);
	if (mFrameData==NULL or not (*mFrameData))
	{
		mFrameData = NULL;
		return false;
	}
	std::ifstream& mFile = *mFrameData;

	// else, confirm magic number and version
	// calculate the size of the file
	mStartPos = mFile.tellg();
	mFile.seekg(0,std::ios::end);
	long end = mFile.tellg();
	mSizeOfFrameDataInBytes = end - mStartPos;
	mFile.seekg(mStartPos);

	unsigned int magicnumber;
	unsigned int version;
	read(mFile,magicnumber);
	if (magicnumber!=SimulationLoader::MAGICNUMBER)
	{
		mFile.close();
		mFrameData = NULL;
		std::cerr << "Magic number is incorrect!\n";
		return false;
	}

	read(mFile,version);
	if (version>SimulationLoader::VERSION)
	{
		mFile.close();
		mFrameData = NULL;
		std::cerr << "Wrong version number. It is version " << version
				<< ", but should be a version less than " << VERSION
				<< ".\nPlease update this library.\n";
		return false;
	}
	mVersion = version;

	if (version==1)
	{
		// there is no static data
		mHasStaticSegment = false;
	}
	else if (version > 1)
	{
		// there could be static data
		mHasStaticSegment = not mStaticSegmentTypes.empty();
	}

	if (mHasStaticSegment)
	{
		read(*mFrameData, mSizeOfStaticSegmentInBytes);
		if (mSizeOfStaticSegmentInBytes==0)
		{
			std::cerr << "static segment size = 0, should be > 0" << std::endl;
			return false;
		}

		mStaticSegmentBegin = mFile.tellg();
		mAtStartOfStaticSegment = true;
	}
	else
	{
		mAtStartOfStaticSegment = false;
		mCurrentFramePos = mFile.tellg();
		readFrameHeader();
	}

	return true;
}

void SimulationLoader::readFrameHeader()
{
	read(*mFrameData,mCurrentFrame.sizeInBytes);
	read(*mFrameData,mCurrentFrame.number);
	read(*mFrameData,mCurrentFrame.time);
	read(*mFrameData,mCurrentFrame.step);

	mStartSegmentPos = mFrameData->tellg();
	mCurrentSegmentPos = mFrameData->tellg();
}

bool SimulationLoader::skipStaticSegment()
{
	if (not mHasStaticSegment)
	{
		std::cerr << "Calling SimulationLoader::skipStaticSegment() o a file with no static segment!" << std::endl;
		return false;
	}

	if (mAtStartOfStaticSegment)
	{
		// skip over the static segment
		mFrameData->seekg((std::ifstream::off_type)mSizeOfStaticSegmentInBytes, std::ios_base::cur);

		// we should now be at the start of the frame data
		mAtStartOfStaticSegment = false;
		mCurrentFramePos = mFrameData->tellg();
		readFrameHeader();
		return true;
	}
	else return false;
}
