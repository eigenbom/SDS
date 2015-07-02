/**
 * A basic command-line simulator.
 *
 * BP 301009
 */

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include "simulationio.h"
#include "segmentio.h"
#include "organismtools.h"
#include "sdssimulation.h"
#include "physics.h"

namespace po = boost::program_options;

int main(int argc, char** argv)
{
	std::string input, output;
	int frame;
	int numFrames;
	int stepsPerFrame;
	bool dirty = false;

	AABB bounds;

	std::string fullCommandLine = std::string(argv[0]);
	for(int i=1;i<argc;i++)
	{
		fullCommandLine += argv[i];
		fullCommandLine += " ";
	}

	// set up command line options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("input,i", po::value<std::string>(&input), "input simulation header (*.cfg)")
		("output,o", po::value<std::string>(&output), "output simulation file prefix (generates *.cfg and *.bin files.) Default is \"input.out\"")
		// ("processModel,p", po::value<std::string>(&processModel), "process model to use - if any")
		// add bounding box parameters,
		("startFrame", po::value<int>(&frame)->default_value(0), "frame from input simulation to use as the start of this simulation (-1 indicates the last frame)")
		("numFrames", po::value<int>(&numFrames)->default_value(100), "number of frames to simulate (default = 100)")
		("stepsPerFrame", po::value<int>(&stepsPerFrame)->default_value(1), "number of simulator steps to take per frame (default==1)")
		("dirty", "Try to fix errors in a hacky way -- may result in a longer simulation but with stranger results")
	;

	try
	{
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << "\n";
			return 1;
		}

		if (vm.count("dirty"))
		{
			dirty = true;
		}

		if (vm.count("input")==0)
		{
			std::cout << "** Input file is required. **\n" << desc << "\n";
			return 1;
		}

		if (vm.count("output")==0)
		{
			output = input + ".out";
		}
	}
	catch(...) // due to probs with program_options, gcc, and fvisibility, we can't see the exception being thrown
	{
		std::cout << "** Error parsing command line. **\n" << desc;
		return 1;
	}

	try
	{
		SimulationLoader loader(input);
		SimulationIO_Base::SimulationHeader hdr = loader.simulationHeader();

		if (!(loader.hasSegment("mesh") and
			loader.hasSegment("organism") and
			loader.hasSegment("processinfo")))
		{
			throw(std::runtime_error("Simulation file must contain mesh, organism and processinfo segments."));
		}

		// create the static segment loaders
		// at the moment we just deal with Mesh type static objects
		std::list<SegmentLoader*> staticSegmentLoaders;
		std::list<Mesh*> staticMeshes;

		BOOST_FOREACH(std::string seg, loader.staticSegments())
		{
			if (seg!=MeshSegmentIO::type)
			{
				std::cerr << "Encountered a static object of type " << seg << ". Can only read \"mesh\" objects." << std::endl;
			}
			else
			{
				Mesh* m = new Mesh();
				staticMeshes.push_back(m);

				MeshSegmentIO* io = new MeshSegmentIO(m);
				staticSegmentLoaders.push_back(io);
			}
		}

		if (loader.loadData())
		{
			// read in static component
			if (not staticMeshes.empty())
			{
				std::cout << "Reading in static mesh info ... ";

				if (not loader.loadStaticSegments(staticSegmentLoaders))
				{
					std::cerr << "error!" << std::endl;
					return -1;
				}

				std::cout << "finished." << std::endl;
			}


			if (frame==-1)
			{
				frame = loader.countFrames()-1;
			}

			// skip forward to the required frame
			for(int i=0;i<frame;i++)
			{
				if (!loader.nextFrame()) throw(std::runtime_error("Simulation file doesn't have that many frames."));
			}

			SimulationIO_Base::FrameHeader frame = loader.currentFrame();

			std::cout << "Construction scene...\n";

			// Set up scene
			// First - clean up existing scene, delete meshes etc.,

			SDSSimulation* simulation = new SDSSimulation();
			simulation->cleanup();

			if (dirty)
				simulation->setContinueOnError(true);

			simulation->initialiseFromHeader(hdr);

			std::cout << "Comments\n"
					  << "--------\n"
					  << hdr.comments << "\n"
					  << "--------\n";

			// third, read in organism data
			Mesh* m = new Mesh();
			MeshSegmentIO ml(m);
			loader.initialiseSegmentLoader(&ml);
			loader.loadSegment(&ml);

			Organism* o = new Organism(m);
			OrganismSegmentIO oio(o);
			loader.initialiseSegmentLoader(&oio);
			loader.loadSegment(&oio);

			ProcessModel* p = hdr.processModel;
			o->setProcessModel(p);

			if (p)
			{
				ProcessModelSegmentIO pml(o);
				if (loader.initialiseSegmentLoader(&pml))
				{
					loader.loadSegment(&pml);
				}
				else throw(std::runtime_error("Couldn't load process info!"));
			}

			OWorld* ow = new OWorld();
			ow->addOrganism(o);
			ow->setBounds(hdr.worldBounds);

			// add static meshes to oworld
			BOOST_FOREACH(Mesh* m, staticMeshes)
			{
				ow->addStaticMesh(m);
			}

			// set the world to use
			simulation->setWorld(ow);

			std::cout << "Generating output header...\n";
			std::list<SegmentWriter*> segmentWriters;

			// dump the simulation header and set up the segment writers
			SimulationIO_Base::SimulationHeader header;
			simulation->writeHeader(header);

			header.frameDataFileName = output + ".bin";
			header.comments = fullCommandLine;

			// build the segment writers
			MeshSegmentIO* outmio = new MeshSegmentIO(simulation->world()->organism()->mesh());
			OrganismSegmentIO* outoio = new OrganismSegmentIO(simulation->world()->organism());
			ProcessModelSegmentIO* outpio = new ProcessModelSegmentIO(simulation->world()->organism());

			BOOST_FOREACH(SegmentWriter* sw, segmentWriters) delete sw;
			segmentWriters.clear();

			segmentWriters.push_back(outmio);
			segmentWriters.push_back(outoio);
			segmentWriters.push_back(outpio);

			// build the static segment writers
			std::list<SegmentWriter*> staticSegmentWriters;
			BOOST_FOREACH(Mesh* m, ow->getStaticMeshes())
			{
				staticSegmentWriters.push_back(new MeshSegmentIO(m));
			}

			// dump the header
			SimulationWriter::writeSimulationHeader(output+".cfg", header, segmentWriters, staticSegmentWriters);

			// open the binary file
			std::ofstream outputFile;
			outputFile.open(header.frameDataFileName.c_str(),std::ios::binary);
			if (!outputFile)
			{
				std::cerr << "Cannot open \"" << header.frameDataFileName << "\" for output!\nQuitting...";
				return 1;
			}
			else
			{
				SimulationWriter::writeFrameDataHeader(outputFile);
				if (not staticSegmentWriters.empty())
					SimulationWriter::writeStaticSegments(outputFile, staticSegmentWriters);
			}

			// start the simulation
			std::cout << "Simulating...\n";

			int frameNumber = 0;
			int stepNumber = 0;
			while(true)
			{
				bool hasCompletedAStep = simulation->substep();
				if (simulation->state()==simulation->UNSTABLE)
				{
					std::cout << "Simulation has become unstable. Quitting...\n";
					return 0;
				}

				if (hasCompletedAStep)
				{
					if ((stepNumber)%stepsPerFrame == 0)
					{
						// output current frame
						// first compute frame size by writing the segment data to a temporary buffer
						std::cout << "Computed frame " <<  frameNumber << std::endl;
						std::ostringstream oss(std::ios::binary);
						BOOST_FOREACH(SegmentWriter* sw, segmentWriters)
							SimulationWriter::writeSegment(oss, *sw);
						std::string os = oss.str();

						// then construct the appropriate header
						SimulationIO_Base::FrameHeader fhdr;
						fhdr.sizeInBytes = (unsigned int)(os.length());
						fhdr.number = frameNumber;
						fhdr.time = simulation->t();
						fhdr.step = simulation->steps();

						// now write the frame header and frame data to out
						SimulationWriter::writeFrameHeader(outputFile,fhdr);
						outputFile << os;

						// increase frame number
						frameNumber++;
					}

					stepNumber++;
				}

				if (frameNumber>=numFrames)
				{
					break;
				}
			}

			std::cout << "Complete.\n";
		}
	}
	catch(std::runtime_error& e)
	{
		std::cerr << "Runtime Error: " << e.what() << std::endl;
        return 1;
	}
	catch(std::exception& e)
	{
		std::cerr << "Unknown Error: " << e.what() << std::endl;
        return 1;
	}


	return 0;
}
