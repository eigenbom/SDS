/*
 * Creates a simulation file with a static segment and a single frame.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/foreach.hpp>

#include "simulationio.h"
#include "segmentio.h"
#include "organism.h"
#include "organismtools.h"
#include "meshtools.h"
#include "oworld.h"
#include "cell.h"

void makeSimulationOutput(std::string output, OWorld* ow)
{
	Organism* o = ow->organism();

	SimulationIO_Base::SimulationHeader header;

	header.kD = 10;
	header.kV = 10;
	header.kDamp = 0.05;
	header.gravity = 1;
	header.dt = 0.01;
	header.worldBounds = ow->bounds();
	header.frameDataFileName = output + ".bin";
	header.comments = std::string("Static Segment Test");
	header.processModel = o->processModel();

	// build the segment writers
	MeshSegmentIO mio(o->mesh());
	OrganismSegmentIO oio(o);
	ProcessModelSegmentIO pio(o);

	std::list<SegmentWriter*> segmentWriters;
	segmentWriters.push_back(&mio);
	segmentWriters.push_back(&oio);
	segmentWriters.push_back(&pio);

	// build the static segment writes
	std::list<SegmentWriter*> staticSegmentWriters;
	BOOST_FOREACH(Mesh* m, ow->getStaticMeshes())
	{
		staticSegmentWriters.push_back(new MeshSegmentIO(m));
	}
	SimulationWriter::writeSimulationHeader(output+".cfg", header, segmentWriters, staticSegmentWriters);

	// write the simulation framedata file
	std::ofstream out;
	out.open(header.frameDataFileName.c_str(), std::ios::binary);
	if (out)
	{
		SimulationWriter::writeFrameDataHeader(out);
		SimulationWriter::writeStaticSegments(out, staticSegmentWriters);
		SimulationWriter::writeFrame(out, 0, 0, 0, segmentWriters);
	}
	else
	{
		std::cout << "Can't open output file \"" << header.frameDataFileName << "\"\n";
	}
}

int main(int argc, char** argv)
{
	std::string mesh = "lattice111.1";

	try
	{
		Organism* o = OrganismTools::load(mesh);
		ProcessModel* pm = ProcessModel::create("NoProcessModel");
		o->setProcessModel(pm);

		Mesh* staticMesh = MeshTools::Load(mesh);
		staticMesh->move(Vector3d(.12,-1.1,.05)); // perturb slightly

		OWorld* ow = new OWorld();
		ow->addOrganism(o);
		ow->addStaticMesh(staticMesh);
		ow->calculateBounds();

		makeSimulationOutput("static",ow);
	}
	catch(std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}

