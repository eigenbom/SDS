/*
 * patternedlimbmodel.cpp
 *
 *  Created on: 10/02/2010
 *      Author: ben
 */

#include "patternedlimbmodel.h"
#include "organism.h"
#include "transform.h"
#include "cell.h"

#include "random.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

StripedLimb::StripedLimb()
:LimbGrowth(),stripeDecay(0),mPassiveCellGrowthRate(0),mStripedCellGrowthRate(0)
{
	this->setNumberOfMorphogens(3);

	setMorphogenDiffusion(1,0);
	setMorphogenDecay(1,0);

	setMorphogenDiffusion(2,0);
	setMorphogenDecay(2,0);
}

void StripedLimb::setup()
{
	LimbGrowth::setup();

	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		CellContents* cc = c->getCellContents();
		if (cc->getType()==3)
			cc->setMorphogen(2,c->vol());
	}
}

void StripedLimb::writeStatic(std::ostream& o)
{
	LimbGrowth::writeStatic(o);
	write(o,stripeDecay);
}

void StripedLimb::readStaticParams(std::istream& o)
{
	LimbGrowth::readStaticParams(o);
	read(o,stripeDecay);

	setMorphogenDecay(1,stripeDecay);
}
std::list<std::string> StripedLimb::parameters()
{
	std::list<std::string> p = LimbGrowth::parameters();
	p.push_back("stripeDecay");
	p.push_back("passiveCellGrowthRate");
	p.push_back("stripedCellGrowthRate");
	return p;
}

/// set a parameter
void StripedLimb::set(std::string str,double val)
{
	LimbGrowth::set(str,val);
	if (str=="stripeDecay")
	{
		stripeDecay = val;
		setMorphogenDecay(1,stripeDecay);
	}
	else if (str=="passiveCellGrowthRate")
		mPassiveCellGrowthRate = val;
	else if (str=="stripedCellGrowthRate")
			mStripedCellGrowthRate = val;

}

/// set a parameter
double StripedLimb::get(std::string str)
{
	if (str=="stripeDecay") return stripeDecay;
	else if (str=="passiveCellGrowthRate") return mPassiveCellGrowthRate;
	else if (str=="stripedCellGrowthRate") return mStripedCellGrowthRate;
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

void StripedLimb::step(Organism* o, double dt)
{
	static const int NORMAL = 0;
	static const int PZ_STIM = 2;
	static const int GROWING_TIP = 3;

	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->getCellContents()->getType() == GROWING_TIP)
			skip.insert(c);
		// c->getCellContents()->setMorphogen(0, c->vol());
	}
	simulateMorphogenDiffusion(o, dt, skip, 0);

	std::list<DivisionInfo> divisionsToBePerformed;

	BOOST_FOREACH(Cell* c, o->cells())
	{
		CellContents* cc = c->getCellContents();
		int type = cc->getType();
		if (type == GROWING_TIP)
		{
			/* Once the morphogen has depleted, immediately stimulate all neighbours (just give them full morphogen 3) */
			if (cc->getMorphogen(1) < 0.01)
			{
				cc->setMorphogen(1,c->vol());

				BOOST_FOREACH(Cell* cn, o->getNeighbours(c))
				{
					cn->getCellContents()->setMorphogen(2,cn->vol());
				}
			}

			continue;
		}

		// normalise
		static double defRadius = c->r();
		static const double RE = rE * defRadius;
		static const double RM = rM * defRadius;
		static const double DRDTE = drdtE * defRadius;
		static const double DRDTM = drdtM * defRadius;

		c->setDrdt(0);

		if (cc->getMorphogen(0) > 0.000000001)
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
		else if (cc->getMorphogen(2) > 0.001)
		{
			cc->setType(NORMAL);
			cc->setMorphogen(2, c->vol());
			c->setDrdt(mStripedCellGrowthRate);
		}
		else
		{
			cc->setType(NORMAL);
			c->setDrdt(mPassiveCellGrowthRate);
		}

		/*
		if (c->isBoundary())
		{

			if (type == GROWING_TIP)
			{
				m[0] = std::min(1.,m[0]);
			}
			else if (m[0] > 0.000000001)
			{
				cc->setType(PZ_STIM);
				c->setDrdt(DRDTE);
				if (c->r() > RE)
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
		}
		else
		{
			if (m[0] > 0.0000000001)
			{
				cc->setType(PZ_STIM);
				c->setDrdt(DRDTM);
				if (c->r() > RM)
				{
					// divide into direction of most morphogen, weighted sum
					Vector3d dir = Vector3d::ZERO;
					std::list<Cell*> cn = o->getNeighbours(c);
					for(std::list<Cell*>::iterator it = cn.begin(); it!=cn.end(); it++)
					{
						Cell* nei = *it;
						double chem = nei->getCellContents()->getMorphogen(0);
						dir += chem*((nei->x() - c->x()).normalise());
					}
					dir.normaliseInPlace();
					divisionsToBePerformed.push_back(DivisionInfo(c,true,dir));
				}
			}
			else
				cc->setType(NORMAL);
		}*/

	}

	if (divisionsToBePerformed.size()>0)
			LOG("LimbBudModel::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		// Use the new higher-level Divide()
		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR) break;

		(res.get<0>()->getCellContents())->setType(NORMAL);
		(res.get<1>()->getCellContents())->setType(NORMAL);

		res.get<0>()->setDrdt(0);
		res.get<1>()->setDrdt(0);

		// distribute morphogens from old cell to new cells
		for(int i=0;i<2;i++)
		{
			double m = d.cell->getCellContents()->getMorphogen(i)/2; //  mo[i]/2;
			(res.get<1>()->getCellContents())->setMorphogen(i,m);
			(res.get<0>()->getCellContents())->setMorphogen(i,m);
		}
	}
}
