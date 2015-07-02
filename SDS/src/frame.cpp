#include "frame.h"
#include "organism.h"
#include "sdssimulation.h"

#include <sstream>

IFrame::IFrame(std::istream& is,ProcessModel* pm)
:mValid(false),mOrganism(NULL)
{
	read(is,mTime);
	read(is,mFrameNumber);

	// read organism information, but don't do anything about it...!
	// NOTE: each interesting chunk is preceded by a size, so we can consume the organism chunk without actually loading it
	unsigned int numBytes = 0;
	read(is,numBytes);
	char* tmpBuffer = new char[numBytes];
	is.read(tmpBuffer,numBytes);
	if (!is)
		mValid = false;
	else
		mValid = true;

	// load the mesh data
	// TODO: should we cache this / load on demand / etc
	std::istringstream iss(std::string(tmpBuffer,numBytes),std::ios::binary);

	try{
		mOrganism = Organism::read(iss,pm);
	} catch (std::exception& e)
	{
		std::cerr << "Error loading organism in frame " << mFrameNumber << "\n\t" << e.what() << std::endl;
		mValid = false;
	}

	delete[]tmpBuffer;
}

IFrame::~IFrame()
{
	// delete organism chunk/mesh/whatever/etc.
	delete mOrganism;
}

Mesh* IFrame::mesh()
{
	return mOrganism->mesh();
}

OFrame::OFrame(Organism* o):mOrganism(o)
{

}

void OFrame::write(SDSSimulation* s, std::ostream& f)
{
	// frame data includes...
	// current time step
	::write(f,s->t());
	// total number of steps taken
	::write(f,s->steps());

	// entire state of organism (including processmodel info)

	// WRITE INTO A TEMPORARY BUFFER FIRST SO WE CAN COMPUTE THE SIZE
	std::ostringstream oss(std::ios::binary);
	mOrganism->write(oss);
	// NOW WRITE THE SIZE OF THE BUFFER, FOLLOWED BY THE BUFFER ITSELF TO THE OUTPUT STREAM
	std::string os = oss.str();
	::write(f,(unsigned int)os.length());
	f.write(os.c_str(),os.length());

	// TODO: all this stuff..?
	// simulation information (what happened (e.g., moves) just before this time step)
	// process model information (what happened in the process model just before this time step)
	// collision module information (what collisions occurred, what forces, blah, etc.)
}
