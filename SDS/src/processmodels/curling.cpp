#include "curling.h"

#include "organism.h"
#include "transform.h"
#include "cell.h"

#include "random.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

Curling::Curling()
:LimbGrowth(2,1)
 ,mStim(0)
 ,mCurlingFactor(1)
{
	this->setMorphogenDiffusion(1,0);
	this->setMorphogenDecay(1,0);
}

void Curling::setup()
{
	LimbGrowth::setup();

	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		CellContents* cc = c->getCellContents();
		cc->setVar(0,cc->getMorphogen(1)); // its normalised
	}
}

void Curling::writeStatic(std::ostream& o)
{
	LimbGrowth::writeStatic(o);
	write(o,mStim);
	write(o,mCurlingFactor);
}

void Curling::readStaticParams(std::istream& o)
{
	LimbGrowth::readStaticParams(o);
	read(o,mStim);
	read(o,mCurlingFactor);
}
std::list<std::string> Curling::parameters()
{
	std::list<std::string> l = LimbGrowth::parameters();
	l.push_back("stim");
	l.push_back("curlingFactor");
	return l;
}

/// set a parameter
void Curling::set(std::string str,double val)
{
	if (str=="stim") mStim = val;
	else if (str == "curlingFactor") mCurlingFactor = val;
	else LimbGrowth::set(str,val);
}

/// set a parameter
double Curling::get(std::string str)
{
	if (str=="stim") return mStim;
	else if (str=="curlingFactor") return mCurlingFactor;
	else return LimbGrowth::get(str);
}

struct DivisionInfo
{
	Cell* cell;
	bool isInternal;
	Vector3d direction;
	DivisionInfo(){}
	DivisionInfo(Cell* c, bool i, Vector3d d):cell(c),isInternal(i),direction(d){}
};

void Curling::step(Organism* o, double dt)
{
	static const int NORMAL = 0;
	static const int PZ_STIM = 2;
	static const int GROWING_TIP = 3;

	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->getCellContents()->getType() == GROWING_TIP)
			skip.insert(c);
	}
	simulateMorphogenDiffusion(o, dt, skip);

	std::list<DivisionInfo> divisionsToBePerformed;

	BOOST_FOREACH(Cell* c, o->cells())
	{
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
		if (cc->getMorphogen(0) > mStim)
		{
			cc->setType(PZ_STIM);

			double drdt, r;
			if (c->isBoundary())
			{
				drdt = DRDTE * (1-mCurlingFactor*cc->getVar(0));
				r = RE * (1-mCurlingFactor*cc->getVar(0));
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
			//c->setDrdt(mPassiveCellGrowthRate);
		}
	}

	if (divisionsToBePerformed.size()>0)
			LOG("Curling::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		double m = d.cell->getCellContents()->getMorphogen(0)/2; //  mo[i]/2;
		double curling = d.cell->getCellContents()->getVar(0);

		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR) break;

		Cell *a = res.get<0>(), *b = res.get<1>();
		CellContents *ac = a->getCellContents(), *bc = b->getCellContents();

		ac->setType(NORMAL);
		bc->setType(NORMAL);

		a->setDrdt(0);
		b->setDrdt(0);

		ac->setMorphogen(0,m);
		bc->setMorphogen(0,m);

		// propogate the curling variable
		ac->setVar(0,curling);
		bc->setVar(0,curling);

		// and store the curling morphogen for visualisation
		ac->setMorphogen(1,curling);
		bc->setMorphogen(2,curling);

	}
}
