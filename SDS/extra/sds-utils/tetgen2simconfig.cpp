/*
 * tetgen2simconfig.cpp
 *
 * Creates a one frame simulation file from a tetgen file.
 * This can be used as an initial configuration for the simulator.
 *
 *  Created on: 21/10/2009
 *      Author: ben
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <boost/program_options.hpp>

#include "simulationio.h"
#include "segmentio.h"
#include "organism.h"
#include "organismtools.h"
#include "oworld.h"
#include "cell.h"

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    std::string input, output, processModel, boundsString;
    float gravity;

    AABB bounds;

    // set up command line options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("input,i", po::value<std::string>(&input), "input tetgen file (*.1) and optionally a (*.1.morphogen) file")
        ("output,o", po::value<std::string>(&output), "output file prefix (generates *.cfg and *.bin files.) Default is \"input.out\"")
        ("processModel,p", po::value<std::string>(&processModel), "process model to use - if any")
        // add bounding box parameters,

        ("bounds,b", po::value<std::string>(&boundsString), "bounds = \"minx miny maxz dx dy dz\"")
        ("gravity,g", po::value<float>(&gravity)->default_value(0), "gravity setting")
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

        if (vm.count("input")==0)
        {
            std::cout << "** Input file is required. **\n" << desc << "\n";
            return 1;
        }

        if (vm.count("output")==0)
        {
            output = input + ".out";
        }

        if (vm.count("processModel")==0)
        {
            processModel = "NoProcessModel";
        }

        if (vm.count("bounds")>0)
        {
            // extract params
            std::istringstream iss(boundsString);
            double x,y,z,dx,dy,dz;
            iss >> x >> y >> z >> dx >> dy >> dz;
            bounds = AABB(x,y,z,x+dx,y+dy,z+dz);

            std::cout << "Using bounds = ["
                << x << ", "
                << y << ", "
                << z << ", "
                << dx << ", "
                << dy << ", "
                << dz << "]\n";

        }
    }
    catch(...) // due to probs with program_options, gcc, and fvisibility, we can't see the exception being thrown
    {
        std::cout << "** Error parsing command line. **\n" << desc;
        return 1;
    }

    {
        Organism* o = OrganismTools::load(input);


        std::cout << "Loaded mesh has: \n"
            << "\t" << o->mesh()->vertices().size() << " verts\n"
            << "\t" << o->mesh()->tetras().size() << " tets\n";
        ProcessModel* pm = ProcessModel::create(processModel);
        o->setProcessModel(pm);

		// if there is a .morphogen file, then use it to initialise the
		// morphogens in the process info (if the processmodel is a morphogen model)
		std::ifstream morphfile((input + ".morphogen").c_str());
		if (!morphfile)
		{
			std::cout << "No .morphogen file found. \n";
		}
		else
		{
			std::cout << "Morphogen file found. Initialising cell morphogen concentrations and cell types.\n";
			char buffer[1024];
			morphfile.getline(buffer,1023);
			std::istringstream iss(buffer);
			int numverts, nummorphs;
			iss >> numverts >> nummorphs;
			std::cout << numverts << " cells with " << nummorphs << " morphogens each\n";

			std::vector<Cell*> cells(o->cells().begin(),o->cells().end());
			for(int i=0;i<numverts;i++)
			{
				morphfile.getline(buffer,99);
				std::istringstream iss(buffer);
				int index;
				iss >> index;
				if (index!=i)
				{
					std::cout << "Index " << index << " not expected yet! Expecting " << i << ".\nExitting.\n";
					return 0;
				}
				CellContents* cc = cells[i]->getCellContents();
				if (cc!=NULL)
				{
					for(int m=0;m<nummorphs;m++)
					{
						double morph;
						iss >> morph;
						cc->setMorphogen(m,morph);
					}
				}
			}
		}

		pm->setup();

        OWorld* ow = new OWorld();
        ow->addOrganism(o);

        if (bounds.valid())
        {
            ow->setBounds(bounds);
        }
        else
        {
            ow->calculateBounds();
        }

        assert(o->mesh()->isSane());
        //std::cout << "Num outer faces: " << o->mesh()->outerFaces().size() << "\n";


        // construct a standard header
        SimulationIO_Base::SimulationHeader header;
        header.t = 0;
        header.dt = 0.001;
        header.kD = 10;
        header.kV = 10;
        header.kDamp = 0.05;
        header.density = 1;
        header.viscosity = 0.05;
        header.collisionInterval = 1;
        header.worldBounds = ow->bounds();
        header.gravity = gravity;
        header.frameDataFileName = output + ".bin";
        header.comments = std::string("Automatically generated by tetgen2simconfig.exe");
        std::string comment = "Automatically generated by tetgen2simconfig.exe";
        for (int i = 1; i < argc; i++) {
            comment = comment + " " + argv[i];
        }
        header.comments = comment.c_str();
        header.processModel = o->processModel();

        // build the segment writers
        MeshSegmentIO mio(o->mesh());
        OrganismSegmentIO oio(o);
        ProcessModelSegmentIO pio(o);

        std::list<SegmentWriter*> segmentWriters;
        segmentWriters.push_back(&mio);
        segmentWriters.push_back(&oio);
        segmentWriters.push_back(&pio);

        // write the simulation configuration file

        SimulationWriter::writeSimulationHeader(output+".cfg",header,segmentWriters);

        // write the simulation framedata file
        std::ofstream out;
        out.open(header.frameDataFileName.c_str(),std::ios::binary);
        if (out)
        {
            SimulationWriter::writeFrameDataHeader(out);

            std::ostringstream frame(std::ios::binary);

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
            out << os;

            /*
               std::cout << "hdrsize: " << frame.str().length() << "\n";
               frame << os;
               std::string frameStr = frame.str();
               std::cout << "frameSize: " << frameStr.length() << ", fhdr.size: " << fhdr.sizeInBytes << "\n";
               assert(frameStr.length()==fhdr.sizeInBytes);
               out << frameStr;
             */

            // test mode, output a second duplicate frame
            if (true)
            {
                std::ostringstream oss(std::ios::binary);
                BOOST_FOREACH(SegmentWriter* sw, segmentWriters)
                    SimulationWriter::writeSegment(oss, *sw);
                std::string os = oss.str();
                SimulationIO_Base::FrameHeader fhdr;
                fhdr.sizeInBytes = (unsigned int)(os.length()); // unknown
                fhdr.number = 1;
                fhdr.time = 0.001;
                fhdr.step = 1;

                SimulationWriter::writeFrameHeader(out,fhdr);
                out << os;
            }
        }
        else
        {
            std::cout << "Can't open output file \"" << header.frameDataFileName << "\"\n";
            return 1;
        }
    }

    std::cout << "Successful.\n";
    return 0;
}

