#include "testworlds.h"
#include "meshtools.h"
#include "random.h"
#include <cassert>

Random gRandom;

World* TestWorlds::loadWorld(std::string name)
{
	World* w = NULL;
	if (name=="smallspheres")
		w = smallSpheres();
	else if (name=="lotsofsmallspheres")
		w = lotsOfSmallSpheres();
	else if (name=="fewcubes")
		w = fewCubes();

	return w;
}

World* TestWorlds::smallSpheres()
{
	World* w = new World;

	Mesh* m = MeshTools::Load("../TPS/mesh/smallsphere.1");
	assert(m);
	w->addMesh(m);
	Vector3d diff(0,3,0);
	for (int i=0;i<6;i++)
	{
		Mesh* m2 = m->copy();
		m2->move(diff);
		diff += Vector3d(1-2*gRandom.getDouble(),2.5,1-2*gRandom.getDouble());
		w->addMesh(m2);
	}
	w->calculateBounds();
	return w;
}

World* TestWorlds::lotsOfSmallSpheres()
{
	World* w = new World;

	Mesh* m = MeshTools::Load("../TPS/mesh/smallsphere.1");
	assert(m);
	//w->addMesh(m);
	for (int i=0;i<3;i++)
	{
		for (int j=0;j<3;j++)
		{
			for (int k=0;k<2;k++)
			{
				double jitter = (1-2*gRandom.getDouble())*0.1;

				Mesh* m2 = m->copy();
				m2->move(Vector3d(2.2*i+jitter,2*k+jitter,2.2*j+jitter));
				w->addMesh(m2);
			}
		}
	}
	w->calculateBounds();

	delete m;
	return w;
}

World* TestWorlds::fewCubes()
{
	World* w = new World;

	Mesh* m = MeshTools::Load("../TPS/mesh/cube.1");
	assert(m);
	w->addMesh(m);
	Vector3d diff(gRandom.getDouble(),1.5+gRandom.getDouble(),gRandom.getDouble());
	for (int i=0;i<10;i++)
	{
		Mesh* m2 = m->copy();
		m2->move(diff);
		diff += Vector3d(gRandom.getDouble(),1.5 + gRandom.getDouble(),gRandom.getDouble());
		w->addMesh(m2);
	}
	w->calculateBounds();
	return w;
}
