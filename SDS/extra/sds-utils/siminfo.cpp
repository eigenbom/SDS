/**
 * Outputs some information from a simulation file.
 * Also demonstrates basic simulation file parsing.
 *
 * Has a mode where it extracts mesh data.
 *
 * bp 2309 sipping tea
 */

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include "simulationio.h"
#include "segmentio.h"
#include "face.h"
#include "cell.h"
#include "processmodel.h"

#include "organismtools.h"

namespace po = boost::program_options;
namespace fi = boost::filesystem;

void dumpMeshAsPly(Mesh* m, std::string filename);

int main(int argc, char** argv)
{
	std::string file;
	std::string directory;
	int skip = 0;
	bool dumpMesh = false;
	bool dumpMorphogens = false;
	bool countFrames = false;

	// set up command line options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("file,f", po::value<std::string>(&file), "input header file")
		("dir,d", po::value<std::string>(&directory)->default_value("data"), "output directory")
		("mesh,m", "Extract mesh surface info for each frame and dump it to a sequence of .ply files")
		("skip,s", po::value<int>(&skip)->default_value(0), "skip N frames per frame") 
		("morphogens", "Dump morphogen values to stdout")
		("count,c", "Count the number of frames in the file.")
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

		if (vm.count("file")==0)
		{
			std::cout << "** File is required. **\n" << desc << "\n";
			return 1;
		}

		if (vm.count("mesh")>0)
			dumpMesh = true;

		if (vm.count("morphogens")>0)
			dumpMorphogens = true;

		if (vm.count("count")>0)
			countFrames = true;
	}
	catch(...) // due to probs with program_options, gcc, and fvisibility, we can't see the exception being thrown
	{
		std::cout << "** Error parsing command line. **\n" << desc;
		return 1;
	}

	try
	{
		SimulationLoader loader(file);
		SimulationIO_Base::SimulationHeader hdr = loader.simulationHeader();

		std::cout << "Loaded " << file << "\n";

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

		if (countFrames)
		{
			std::cout << "Counting frames..." << std::endl;

			/*

			if (!loader.loadData())
			{
				std::cerr << "Couldn't load simulation file" << std::endl;
			}

			// read in static component
			if (not staticMeshes.empty())
			{
				if (not loader.loadStaticSegments(staticSegmentLoaders))
				{
					std::cerr << "error!" << std::endl;
					return -1;
				}
			}


			int numFrames = 1;
			while(loader.nextFrame()) numFrames++;
			*/

			int numFrames = loader.countFrames();
			std::cout << "There are " << numFrames << " frames." << std::endl;
			return 0;
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

				BOOST_FOREACH(Mesh* m, staticMeshes)
				{
					const AABB& aabb = m->aabb();
					std::cout << "Found Static Mesh. Bounds = " << aabb << std::endl;
				}
			}

			// read in frame data
			do
			{
				SimulationIO_Base::FrameHeader frame = loader.currentFrame();
				std::cout << "Frame " << frame.number << "\n"
				
						<< "\n\tt: " << frame.time << "s"
						<< "\n\tsize: " << frame.sizeInBytes << "b"
						<< "\n\tstep: " << frame.step << "\n";
						

				Mesh* m = NULL;
				if (loader.hasSegment("mesh"))
				{
					m = new Mesh();
					MeshSegmentIO ml(m);
					loader.initialiseSegmentLoader(&ml);
					loader.loadSegment(&ml);

					std::cout << "mesh\n";
					std::cout << "# vertices: " << m->vertices().size() << "\n";
					std::cout << "# edges: " << m->edges().size() << "\n";
					std::cout << "# tetras: " << m->tetras().size() << "\n";
					std::cout << "aabb ";
					AABB aabb = m->aabb();
					for(int i=0;i<6;i++)
					{
						std::cout << aabb[i] << ", ";
					}
					std::cout << "\n";

					if (dumpMesh)
					{
						std::ostringstream oss;
						oss << std::setw(5) << std::setfill('0') << frame.number;
						fi::path target = fi::path(directory) / fi::path(oss.str() + ".ply");
						dumpMeshAsPly(m,target.file_string());
					}
				}

				Organism* o = NULL;
				if (loader.hasSegment("organism"))
				{
					o = new Organism(m);
					OrganismSegmentIO oio(o);
					loader.initialiseSegmentLoader(&oio);
					loader.loadSegment(&oio);
				}

				if (loader.hasSegment("processinfo"))
				{
					ProcessModel* p = hdr.processModel;

					if (p==NULL)
					{
						// Something has probably gone wrong, so we'll just ignore the process info

					}
					else
					{
						o->setProcessModel(p);
						ProcessModelSegmentIO pml(o);
						if (loader.initialiseSegmentLoader(&pml))
						{
							loader.loadSegment(&pml);
						}
						else
						{
							std::cout << "Couldn't load process info!\n";
							return 1;
						}

						if (dumpMorphogens)
						{
							std::cout << "morphogen info. type, m0, m1, m2, ....\n";
							BOOST_FOREACH(Cell* c, o->cells())
							{
								CellContents* cc = c->getCellContents();
								std::cout << cc->getType() << " ";									 
								for(int i=0;i<cc->numMorphogens();i++)
								{
									std::cout << cc->getMorphogen(i) << " ";
								}
								std::cout << "\n";
							}
						}
					}
				}

				if (not m->isSane())
				{
					std::cout << "Mesh not sane!\n";
				}	
				
				for(int i=0;i<skip;i++)
				{
					loader.nextFrame();			
				}

			}
			while (loader.nextFrame());
		}
		else
		{
			std::cout << "Couldn't load the frame data.\n";
		}
	}
	catch (SimulationLoader::LoadException& le)
	{
		std::cout << le.why() << "\n";
		return 1;
	}
	catch (std::exception& e)
	{
		std::cout << "Exception caught: " << e.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		std::cout << "An unknown exception was thrown.\n";
		return 1;
	}

	return 0;
}

/*
 * ply
format ascii 1.0           { ascii/binary, format version number }
comment made by Greg Turk  { comments keyword specified, like all lines }
comment this file is a cube
element vertex 8           { define "vertex" element, 8 of them in file }
property float x           { vertex contains float "x" coordinate }
property float y           { y coordinate is also a vertex property }
property float z           { z coordinate, too }
property float nx           { vertex contains float "x" coordinate }
property float ny           { y coordinate is also a vertex property }
property float nz           { z coordinate, too }
element face 6             { there are 6 "face" elements in the file }
property list uchar int vertex_index { "vertex_indices" is a list of ints }
end_header                 { delimits the end of the header }
0 0 0                      { start of vertex list }
0 0 1
0 1 1
0 1 0
1 0 0
1 0 1
1 1 1
1 1 0
4 0 1 2 3                  { start of face list }
4 7 6 5 4
4 0 4 5 1
 *
 */
void dumpMeshAsPly(Mesh* m, std::string filename)
{
	std::ofstream file;
	file.open(filename.c_str(),std::ios::binary);
	if (!file)
		return;

	int countSurfaceVerts = 0;
	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		if (v->surface()) countSurfaceVerts++;
	}

	file << "ply\n"
			<< "format ascii 1.0\n"
			<< "comment made by siminfo\n"
			<< "element vertex " << countSurfaceVerts << "\n"
			<< "property float x\n"
			<< "property float y\n"
			<< "property float z\n"
			<< "property float nx\n"
			<< "property float ny\n"
			<< "property float nz\n"
			<< "element face " << m->outerFaces().size() << "\n"
			<< "property list uchar int vertex_index\n"
			<< "end_header\n";

	std::map<Vertex*,int> vm;
	int index = 0;
	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		if (v->surface())
		{
			vm[v] = index;
			index++;
			Vector3d n = v->n();
			file << v->x().x() << " " << v->x().y() << " " << v->x().z() << " "
				<< n.x() << " " << n.y() << " " << n.z() << "\n";
		}
	}

	BOOST_FOREACH(Face* f, m->outerFaces())
	{
		file << "3 "
			<< vm[&f->v(0)] << " "
			<< vm[&f->v(1)] << " "
			<< vm[&f->v(2)] << "\n";
	}
}
