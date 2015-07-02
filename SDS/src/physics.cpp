#include "physics.h"

#include "log.h"

#include "edge.h"
#include "face.h"
#include "tetra.h"
#include "mesh.h"

#include <iostream>
#include <sstream>
#include <boost/foreach.hpp>
#include <cmath>
/*
 * calculates forces based on the paper of Teschner et al.
 */

#define PHYSICS_CHECK_PARTIALS

// generalised spring coefficients
double Physics::kD = 1; // dist
double Physics::kSM = 1; // multiplier
double Physics::kDamp = 0.001; // damp
double Physics::kV = 2.0; // volume

//
double Physics::GRAVITY = 10;

bool
Physics::FD(Edge* e, FDInfo* fd)
{
	// Spring Force ... ((a - b)*k*(r - d(a,b)))/(r*r*d(a,b)) (not including damping...)
	// verified with mathematica...springs.nb (near end) 09.10.07

	Vertex* a = e->v(0);
	Vertex* b = e->v(1);
	const double& r = e->rest();

	const Vector3d &ax = a->x(), &bx = b->x(), &av = a->v(), &bv = b->v();

	Vector3d diff = bx - ax;
	double length = diff.length();

	// diff.print(std::cerr);
	Vector3d dCdb = diff/(r*length); // XXX: This .was. a mistake! Fixed now [09.10.07]
	double C = (length - r) / r;

	// Compute adjusted spring coefficient
	double adjSpringCoefficient = e->springCoefficient()*e->rest()*(a->m()+b->m());
	double springPhysics = - adjSpringCoefficient * C;

	/** Removed spring damping, added damping to tetrahedral springs
	// compute damping
	double dampingPhysics = -kDamp * dot(dCdb,bv-av);
	// XXX: Damping from this spring should not exceed the other spring force
	if (springPhysics > 0)
	{
		if (dampingPhysics > springPhysics) dampingPhysics = 0;
		else if (-dampingPhysics > springPhysics) dampingPhysics = -springPhysics;
	}
	else
	{
		if (dampingPhysics < springPhysics) dampingPhysics = 0;
		else if (-dampingPhysics < springPhysics) dampingPhysics = -springPhysics;
	}
	*/

	double totalPhysics = springPhysics; // + dampingPhysics;
	a->addF(-totalPhysics * dCdb);
	b->addF(totalPhysics * dCdb);

#ifdef PHYSICS_CHECK_PARTIALS
	if (std::isnan(totalPhysics)
		or std::isnan(dCdb.x())
		or std::isnan(dCdb.y())
		or std::isnan(dCdb.z())
	)
	{
		LOG("Physics::FD NaN force computed\n");
		LOG("Variables:\n");
		LOG("totalPhysics = " << totalPhysics << "\n");
		LOG("dCdb = " << dCdb << "\n");
		return false;
	}
#endif

	if (fd)
	{
		fd->length = length;
		fd->extension = C;
		fd->energy = .5*e->springCoefficient()*C*C;
	}

	return true;
}

bool Physics::FV(Tetra* t, FVInfo* fv)
{
	Vertex* v0 = &t->v(0);
	Vertex* v1 = &t->v(1);
	Vertex* v2 = &t->v(2);
	Vertex* v3 = &t->v(3);

	const Vector3d& a = v0->x();
	const Vector3d& b = v1->x();
	const Vector3d& c = v2->x();
	const Vector3d& d = v3->x();

	const double &a1 = a.x(), &a2 = a.y(), &a3 = a.z(),
				&b1 = b.x(), &b2 = b.y(), &b3 = b.z(),
				&c1 = c.x(), &c2 = c.y(), &c3 = c.z(),
				&d1 = d.x(), &d2 = d.y(), &d3 = d.z();

	double rest = t->rest();
	double factor = 1/(6*rest);

	double dCdVParts[] =
	{(b3*(c2 - d2) + c3*d2 - c2*d3 + b2*(-c3 + d3)),
	 (-c3*d1 + b3*(-c1 + d1) + b1*(c3 - d3) + c1*d3),
	 (b2*(c1 - d1) + c2*d1 - c1*d2 + b1*(-c2 + d2)),
	 (-c3*d2 + a3*(-c2 + d2) + a2*(c3 - d3) + c2*d3),
	 (a3*(c1 - d1) + c3*d1 - c1*d3 + a1*(-c3 + d3)),
	 (-c2*d1 + a2*(-c1 + d1) + a1*(c2 - d2) + c1*d2),
	 (a3*(b2 - d2) + b3*d2 - b2*d3 + a2*(-b3 + d3)),
	 (-b3*d1 + a3*(-b1 + d1) + a1*(b3 - d3) + b1*d3),
	 (a2*(b1 - d1) + b2*d1 - b1*d2 + a1*(-b2 + d2)),
	 (-b3*c2 + a3*(-b2 + c2) + a2*(b3 - c3) + b2*c3),
	 (a3*(b1 - c1) + b3*c1 - b1*c3 + a1*(-b3 + c3)),
	 (-b2*c1 + a2*(-b1 + c1) + a1*(b2 - c2) + b1*c2)};

	Vector3d dCda = Vector3d(dCdVParts[0],dCdVParts[1],dCdVParts[2]) * factor;
	Vector3d dCdb = Vector3d(dCdVParts[3],dCdVParts[4],dCdVParts[5]) * factor;
	Vector3d dCdc = Vector3d(dCdVParts[6],dCdVParts[7],dCdVParts[8]) * factor;
	Vector3d dCdd = Vector3d(dCdVParts[9],dCdVParts[10],dCdVParts[11]) * factor;

	double vol = (1./6)*dot(b-a,cross(c-a,d-a));
	double C = (vol - rest) / rest;
	double negKC = - t->springCoefficient()*C;

	// We apply damping to these springs
	// double dampingPhysics = -kDamp * dot(dCdb,bv-av);
	// e.g., damp(p0) = -kDamp * sum(dCdp * pv)
	const Vector3d& av = v0->v();
	const Vector3d& bv = v1->v();
	const Vector3d& cv = v2->v();
	const Vector3d& dv = v3->v();
	double damping = -kDamp * (dot(dCda,av) + dot(dCdb,bv) + dot(dCdc,cv) + dot(dCdd,dv));
	//LOG("-kC " << negKC << ",a d " << damping << "\n");
	// double damping = 1;

	// damping cannot be more than negKC
	if (negKC > 0)
	{
		if (damping > negKC) damping = negKC;
		else if (-damping > negKC) damping = -negKC;
	}
	else
	{
		if (damping < negKC) damping = negKC;
		else if (-damping < negKC) damping = -negKC;
	}

	double finalmult = damping+negKC;

	v0->addF(dCda*finalmult);
	v1->addF(dCdb*finalmult);
	v2->addF(dCdc*finalmult);
	v3->addF(dCdd*finalmult);

	#ifdef PHYSICS_CHECK_PARTIALS
	if (std::isnan(finalmult)
		or std::isnan(dCda.x()) or std::isnan(dCda.y()) or std::isnan(dCda.z())
		or std::isnan(dCdb.x()) or std::isnan(dCdb.y()) or std::isnan(dCdb.z())
		or std::isnan(dCdc.x()) or std::isnan(dCdc.y()) or std::isnan(dCdc.z())
		or std::isnan(dCdd.x()) or std::isnan(dCdd.y()) or std::isnan(dCdd.z())
	)
	{
		LOG("Physics::FV NaN force computed\n");
		LOG("Variables:\n");
		LOG("finalmult = " << finalmult << "\n");
		LOG("damping    = " << damping << "\n");
		LOG("(expanded) = " << -kDamp << "*" << "(" << dot(dCda,av) << "+" << dot(dCdb,bv) << "+" << dot(dCdc,cv) << "+" << dot(dCdd,dv) << ")\n");
		LOG("dCda = " << dCda << "\n");
		LOG("av = " << av << "\n");
		LOG("dCdb = " << dCdb << "\n");
		LOG("bv = " << bv << "\n");
		LOG("dCdc = " << dCdc << "\n");
		LOG("cv = " << cv << "\n");
		LOG("dCdd = " << dCdd << "\n");
		LOG("dv = " << dv << "\n");
		LOG("negKC = " << negKC << "\n");
		LOG("vol = " << vol << "\n");
		LOG("C = " << C << "\n");


		LOG("dCda = " << dCda << "\n");
		LOG("dCdb = " << dCdb << "\n");
		LOG("dCdc = " << dCdc << "\n");
		LOG("dCdd = " << dCdd << "\n");

		return false;
	}
	#endif

	if (fv)
	{
		fv->energy = .5*t->springCoefficient()*C*C;
	}

	return true;
}

bool Physics::setParam(std::string param, std::string value)
{
	std::istringstream iss(value);
	double val;
	iss >> val;

	if (param=="kd")
	{
		setKD(val);
		return true;
	}
	else if (param=="kv")
	{
		setKV(val);
		return true;
	}
	else if (param=="kdamp")
	{
		setKDamp(val);
		return true;
	}
	else if (param=="gravity")
	{
		setGravity(val);
		return true;
	}
	else return false;
}

void Physics::SetAllEdgeSpringStrengths(Mesh* m)
{
	BOOST_FOREACH(Edge* e, m->edges())
	{
		if (m->getSurfaceFace(e->v(0),e->v(1)))
		{
			e->setSpringCoefficient(Physics::kD * Physics::kSM);
		}
		else
			e->setSpringCoefficient(Physics::kD);

	}
}
