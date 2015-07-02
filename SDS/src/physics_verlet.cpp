#include "physics.h"
#include "vector3.h"
#include "log.h"

#include <cfloat>
#include <boost/foreach.hpp>
#include <iostream>
#include <cmath>

double Physics::DENSITY = 1.0;
double Physics::sLastStepSize = -1;
double Physics::VISCOSITY = 0.01;

const double SOME_LARGE_NUMBER = 1e10; // used for stability checking

bool Physics::step(Mesh* m, double dt, bool check)
{
	calculateForces(m);
	return takeVerletStep(m,dt,check);
}

void Physics::zeroForces(Mesh* m)
{
	BOOST_FOREACH(Vertex* v, m->vertices())
		v->zeroF();
}

void Physics::calculateForces(Mesh* m, bool zero, bool dirty)
{
	// Apply Forces to vertices, edges, and volume elements
	Physics::FDInfo fd;
	Physics::FVInfo fv;

	// record total energy of system for all edges and all tetras
	double totalEdgeEnergy = 0, totalTetraEnergy = 0;

	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		if (v->isFrozen())
			continue;

		if (zero)
			v->zeroF();
		v->addF(Vector3d::Y * -Physics::GRAVITY * (v->m() * Physics::DENSITY)); // gravity
	}

	BOOST_FOREACH(Edge* e, m->edges())
	{
		if (e->v(0)->isFrozen() and e->v(1)->isFrozen())
			continue;

		if (not Physics::FD(e,&fd))
		{
			LOG("Physics::calculatedForces() error computing edge force\n");
			if (dirty)
			{
				e->v(0)->setFrozen(true);
				e->v(1)->setFrozen(true);
			}
			else return;
		}
		totalEdgeEnergy += fd.energy;
	}

	BOOST_FOREACH(Tetra* t, m->tetras())
	{
		if (t->pv(0)->isFrozen() and
				t->pv(1)->isFrozen() and
						t->pv(2)->isFrozen() and
								t->pv(3)->isFrozen())
			continue;

		if (not Physics::FV(t,&fv))
		{
			LOG("Physics::calculatedForces() error computing tetra force\n");
			if (dirty)
			{
				t->pv(0)->setFrozen(true);
				t->pv(1)->setFrozen(true);
				t->pv(2)->setFrozen(true);
				t->pv(3)->setFrozen(true);
			}
			else
				return;
		}
		totalTetraEnergy += fv.energy;
	}

	LOG("Total edge energy: " << totalEdgeEnergy << "\n");
	LOG("Total tetra energy: " << totalTetraEnergy << "\n");
}

bool Physics::takeVerletStep(Mesh* m, double h, bool check, bool dirty)
{
	if (sLastStepSize < 0)
		sLastStepSize = h;

	// Take a VERLET step forward
	// x(t+h) = 2x(t) - x(t-h) + h*h*F(t)/m + O(h^4)
	// v(t+h) = [x(t+h) - x(t-h)]/2h + O(h^2)
	double min[3] = {DBL_MAX,DBL_MAX,DBL_MAX}, max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX};

	BOOST_FOREACH(Vertex* v, m->mVertices)
	{
		if (v->isFrozen())
			continue;

		if (check)
		{
			if (std::isnan(v->f().x()) or std::isnan(v->f().y()) or std::isnan(v->f().z()))
			{
				LOG("Physics::takeVerletStep instability detected"<< "\n");
				LOG("v->f() = " << v->f() << "\n");
				return false;
			}
		}

		Vector3d old_oldx = v->mOldX;

		Vector3d oldx = v->mX;
		Vector3d dx = (v->mX - v->mOldX)*(h/sLastStepSize)+ v->f()*h*h/(v->m()*DENSITY);
		v->mX = v->mX + (1-VISCOSITY)*dx;

		// XXX: v->mV(t) = (v->mX - v->mOldX)/(2*sLastStepSize);
		// v->mV(t+dt) = ... (up to date vel, at cost of accuracy)
		v->mV = (v->mX - oldx)/h; // fixed bug! sLastStepSize;

		v->mOldX = oldx;

		// STABILITY CHECK
		if (check)
		{
			if (
					(std::isnan(v->x().x()) or std::isnan(v->x().y()) or std::isnan(v->x().z()))
					or
					(v->x().mlength() > SOME_LARGE_NUMBER)
					or
					(v->v().mlength() > SOME_LARGE_NUMBER)
				)
			{
				LOG("Physics::takeVerletStep instability detected"<< "\n");
				LOG("old x = " << old_oldx << ", new x = " << oldx << ", mass = " << v->m()<< "\n");
				LOG("Partial sums:"<< "\n");
				LOG("v_x-v_oldx = " << (oldx - old_oldx)<< "\n");
				LOG("(v->mX - v->mOldX)*(h/sLastStepSize) = " << (oldx - old_oldx)*(h/sLastStepSize)<< "\n");
				LOG("(v->m()*DENSITY)= " << (v->m()*DENSITY) << "\n");
				LOG("v->f()*h*h/(v->m()*DENSITY)= " << v->f()*h*h/(v->m()*DENSITY)<< "\n");

				if (dirty)
				{
					v->setFrozen(true);
					continue;
				}
				else return false;
			}
		}

		double x = v->mX.x(), y = v->mX.y(), z = v->mX.z();
		if (x < min[0]) min[0] = x;
		if (x > max[0]) max[0] = x;

		if (y < min[1]) min[1] = y;
		if (y > max[1]) max[1] = y;

		if (z < min[2]) min[2] = z;
		if (z > max[2]) max[2] = z;
	}

	AABB& mAABB = m->mAABB;
	mAABB[0] = min[0];
	mAABB[1] = min[1];
	mAABB[2] = min[2];
	mAABB[3] = max[0];
	mAABB[4] = max[1];
	mAABB[5] = max[2];
	mAABB.valid(true);

	sLastStepSize = h;
	return true;
}
