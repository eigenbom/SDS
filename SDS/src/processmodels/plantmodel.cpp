#include "plantmodel.h"
#include "organism.h"
#include "transform.h"
#include "cell.h"

#include "random.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

struct DivisionInfo
{
	Cell* cell;
	bool isInternal;
	Vector3d direction;
	DivisionInfo(){}
	DivisionInfo(Cell* c, bool i, Vector3d d):cell(c),isInternal(i),direction(d){}
};

PlantModel::PlantModel()
:ProcessModel(4,1)
 ,rE(1),rM(1),drdtE(0),drdtM(0)
 ,budGrowthFactor(1)
{
	this->setMorphogenDiffusion(2,0);
	this->setMorphogenDecay(2,0);

	this->setMorphogenDiffusion(3,0);
	this->setMorphogenDecay(3,0);
}

void PlantModel::setup()
{
	std::cerr << "Setup\n";
	ProcessModel::setup();

	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		// std::cerr << "cell " << c << "\n";
		CellContents* cc = c->getCellContents();
		if (cc->getMorphogen(0) > c->vol()/2)
		{
			cc->setType(3);
			cc->setMorphogen(0,c->vol());
			cc->setMorphogen(2,c->vol());
		}

		if (cc->getMorphogen(1) > .5)
		{
			//std::cerr << "Freezing!\n";
			c->v()->setFrozen(true);
		}

		cc->setVar(0,0);
	}

	// Propogate frozen verts into the volume
	// We do this by setting a cell's frozen status to be the majority of its processed neighbours frozen status
	// let P be the set of all boundary cells
	// compute N(P) / P
	// while N(P) / P:
	//   Q = []
	//   for each c in N(P) / P:
	//     M = N(c) intersect P
	//     let c.frozen = majority in M
	//     add c to Q
	//   P += Q
	//   compute N(P) / P

	std::cerr << "Propagating frozen verts" << std::endl;

	std::list<Cell*> C = organism()->cells();
	std::set<Cell*> P;
	BOOST_FOREACH(Cell* c, C)
		if (c->isBoundary())
			{
				// std::cerr << "b";
				P.insert(c);
			}
	std::set<Cell*> NP;
	BOOST_FOREACH(Cell* c, P)
	{
		std::list<Cell*> Nc = organism()->getNeighbours(c);
		NP.insert(Nc.begin(),Nc.end());
	}
	//std::cerr << "P" << P.size() << "\n";
	//std::cerr << "NP" << NP.size() << "\n";
	std::vector<Cell*> NPrP(NP.size());
	std::vector<Cell*>::iterator it = std::set_difference(NP.begin(),NP.end(),P.begin(),P.end(),NPrP.begin());
	NPrP.resize(int(it - NPrP.begin()));
	//std::cerr << "NPrP" << NPrP.size() << "\n";

	while(NPrP.size()>0)
	{
		//std::cerr << NPrP.size() << std::endl;
		BOOST_FOREACH(Cell* c, NPrP)
		{
			std::list<Cell*> Nc = organism()->getNeighbours(c);
			std::vector<Cell*> M(P.size());
			std::vector<Cell*>::iterator it = std::set_intersection(P.begin(),P.end(),Nc.begin(),Nc.end(),M.begin());
			M.resize(int(it - M.begin()));
			// count majority in M
			int frozens = 0;
			BOOST_FOREACH(Cell* m, M)
			{
				frozens += m->v()->isFrozen()?1:0;
			}
			if ((double)frozens >= 0.7*M.size())
				c->v()->setFrozen(true);
			else
				c->v()->setFrozen(false);
		}

		P.insert(NPrP.begin(),NPrP.end());

		NP.clear();

		BOOST_FOREACH(Cell* c, P)
		{
			std::list<Cell*> Nc = organism()->getNeighbours(c);
			NP.insert(Nc.begin(),Nc.end());
		}

		NPrP = std::vector<Cell*>(NP.size());
		std::vector<Cell*>::iterator it = std::set_difference(NP.begin(),NP.end(),P.begin(),P.end(),NPrP.begin());
		NPrP.resize(int(it - NPrP.begin()));

		std::set_difference(NP.begin(),NP.end(),P.begin(),P.end(),NPrP.end());
	}
}

void PlantModel::writeStatic(std::ostream& o)
{
	assert(false);

	ProcessModel::writeStatic(o);

	// write parameters
	write(o,getMorphogenDiffusion(0));
	write(o,getMorphogenDecay(0));
	write(o,rE);
	write(o,rM);
	write(o,drdtE);
	write(o,drdtM);
	write(o,getMorphogenDecay(2));
}

void PlantModel::readStaticParams(std::istream& o)
{
	assert(false);

	ProcessModel::readStaticParams(o);

	// read parameters
	double diff, decay;
	read(o,diff);
	read(o,decay);

	setMorphogenDiffusion(0, diff);
	setMorphogenDecay(0, decay);

	read(o,rE);
	read(o,rM);
	read(o,drdtE);
	read(o,drdtM);

	double budtimer;

	read(o,budtimer);
	setMorphogenDecay(2,budtimer);

}
std::list<std::string> PlantModel::parameters()
{
	std::list<std::string> p = ProcessModel::parameters();
	std::string arr[] = {
			"diffusion",
			"decay",
			"rE",
			"rM",
			"drdtE",
			"drdtM",
			"budSignalDecay",
			"budActivatorDiff",
			"budActivatorDecay",
			"budGrowthFactor",
			"budSize"
	};
	p.insert(p.end(),arr,arr+11);
	return p;
}

/// set a parameter
void PlantModel::set(std::string str,double val)
{
	ProcessModel::set(str,val);

	if (str=="diffusion") setMorphogenDiffusion(0, val);
	if (str=="decay") setMorphogenDecay(0, val);
	if (str=="rE") rE = val;
	if (str=="rM") rM = val;
	if (str=="drdtE") drdtE = val;
	if (str=="drdtM") drdtM = val;
	if (str=="budSignalDecay") setMorphogenDecay(2,val);
	if (str=="budActivatorDiff") setMorphogenDiffusion(3,val);
	if (str=="budActivatorDecay") setMorphogenDecay(3,val);
	if (str=="budGrowthFactor") budGrowthFactor = val;
	if (str=="budSize") budSize = val;
}

/// set a parameter
double PlantModel::get(std::string str)
{
	if (str=="diffusion") return getMorphogenDiffusion(0);
	if (str=="decay") return getMorphogenDecay(0);
	if (str=="rE") return rE;
	if (str=="rM") return rM;
	if (str=="drdtE") return drdtE;
	if (str=="drdtM") return drdtM;
	if (str=="budSignalDecay") return getMorphogenDecay(2);
	if (str=="budActivatorDiff") return getMorphogenDiffusion(3);
	if (str=="budActivatorDecay") return getMorphogenDecay(3);
	if (str=="budGrowthFactor") return budGrowthFactor;
	if (str=="budSize") return budSize;

	return ProcessModel::get(str);
}

void PlantModel::step(Organism* o, double dt)
{
	static const int NORMAL = 0;
	static const int PZ_STIM = 2;
	static const int GROWING_TIP = 3;
	static const int BUD = 4;
	static const int BUD_TIP = 5;


	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		CellContents* cc = c->getCellContents();
		if (cc->getType()==GROWING_TIP)
		{
			// disable growth once we ahev grown for a while
			LOG("budMorph = " << cc->getMorphogen(2) << "\n");

			if (cc->getMorphogen(2) < 0.01)
			{
				cc->setMorphogen(0,0);
				cc->setType(BUD_TIP);
				cc->setMorphogen(3,c->vol());
				cc->setVar(0,c->r()*budSize);
			}
			else
			{
				cc->setMorphogen(0,c->vol());
			}
		}
		else if (cc->getType()==BUD_TIP)
		{
			if (c->r() > cc->getVar(0))
				cc->setMorphogen(3,0);
			else
				cc->setMorphogen(3,c->vol());
		}

		//if (c->getCellContents()->getType() == GROWING_TIP)
		//	skip.insert(c);
	}
	simulateMorphogenDiffusion(o, dt, skip);

	std::list<DivisionInfo> divisionsToBePerformed;

	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->v()->isFrozen()) continue;

		CellContents* cc = c->getCellContents();
		int type = cc->getType();
		if (type == GROWING_TIP) continue;

		// normalise
		static double defRadius = c->r();
		static const double RE = rE * defRadius;
		static const double RM = rM * defRadius;
		static const double DRDTE = drdtE * defRadius;
		static const double DRDTM = drdtM * defRadius;

		c->setDrdt(0);

		// first check to see if we are in bud form
		if (cc->getMorphogen(3) > 0.1)
		{
			// ACTIVATE BUD PROGRAM
			// IN THIS CASE, JUST GROW SLOWLY

			if (cc->getType()!=BUD_TIP and cc->getType()!=BUD)
			{
				cc->setType(BUD);
				cc->setVar(0, c->r()*budSize);
			}

			// don't grow past a certain size
			if (c->r() >= cc->getVar(0)) continue;

			c->setDrdt(budGrowthFactor); //  * cc->getMorphogen(3));
		}
		// else, perform limb growth
		else if (cc->getMorphogen(0) > 0.000000001)
		{
			cc->setType(PZ_STIM);

			double drdt, r;
			if (c->isBoundary())
			{
				drdt = DRDTE;
				r = RE;
			}
			else
			{
				drdt = DRDTM;
				r = RM;
			}

			c->setDrdt(drdt);
			if (c->r() > r)
			{
				// divide towards the source
				Vector3d dir = Vector3d::ZERO;
				std::list<Cell*> cn = o->getNeighbours(c);
				for(std::list<Cell*>::iterator it = cn.begin(); it!=cn.end(); it++)
				{
					Cell* nei = *it;
					double chem = nei->getCellContents()->getMorphogen(0);
					dir += chem*((nei->x() - c->x()).normalise());
				}
				dir.normaliseInPlace();
				divisionsToBePerformed.push_back(DivisionInfo(c,false,dir));
			}
		}
		else
		{
			cc->setType(NORMAL);
		}
	}

	if (divisionsToBePerformed.size()>0)
			LOG("LimbBudModel::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		// Use the new higher-level Divide()
		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR)
		{
			// guess that d.cell still exists...
			d.cell->v()->setFrozen(true);
			break;
		}

		(res.get<0>()->getCellContents())->setType(NORMAL);
		(res.get<1>()->getCellContents())->setType(NORMAL);

		res.get<0>()->setDrdt(0);
		res.get<1>()->setDrdt(0);

		double m = d.cell->getCellContents()->getMorphogen(0)/2; //  mo[i]/2;
		(res.get<1>()->getCellContents())->setMorphogen(0,m);
		(res.get<0>()->getCellContents())->setMorphogen(0,m);
	}
}
