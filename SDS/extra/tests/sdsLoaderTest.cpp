#include "test.h"

#include "aabb.h"

#include "simulationio.h"

#include <iostream>
#include <stdexcept>
using namespace std;

void sdsLoaderTest()
{
	try
	{
		cout << "Testing simulation header reading...\n";

		SimulationLoader loader("example.sim.cfg");
		SimulationIO_Base::SimulationHeader hdr = loader.simulationHeader();

		cout << "frameDataFileName: " << hdr.frameDataFileName << endl;
		cout << "t: " << hdr.t << endl;
		cout << "dt: " << hdr.dt << endl;
		AABB bounds = hdr.worldBounds;
		cout << "bounds: ";
		for(int i=0;i<6;i++) cout << bounds.pts()[i] << " ";
		cout << endl;
		cout << "ci: " << hdr.collisionInterval << endl;
		cout << "gravity: " << hdr.gravity << endl;

		if (hdr.processModel)
		{
			cout << "process model: " << hdr.processModel->name() << endl;
		}
		else
			cout << "no process model\n";

		cout << "Segments:\n";
		BOOST_FOREACH(std::string segtype, loader.segments())
		{
			cout << "\t" << segtype << "\n";
		}

		cout << "Testing simulation header writing...\n";
		SimulationIO_Base::SimulationHeader newhdr = hdr;
		newhdr.comments = "Testing simulation header writing...\nDo multi-line comments work?";

		SimulationWriter::writeSimulationHeader("example.out.cfg",hdr);
	}
	catch(runtime_error& e)
	{
		cout << "Runtime_error: " << e.what() << "\n";
	}
	catch(exception& e)
	{
		cout << "exception: " << e.what() << "\n";
	}
}
