/*
 * shrinkmodel.cpp
 *
 *  Created on: 22/12/2009
 *      Author: ben
 */

#include "shrinkmodel.h"
#include "organism.h"
#include "transform.h"
#include "cell.h"

#include "random.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>
/*
ShrinkModel::CD::CD():morphogenInfo(),type(0){}
void ShrinkModel::CD::write(std::ostream& f){
	::write(f,type);
	morphogenInfo.write(f);
}

void ShrinkModel::CD::read(std::istream& f)
{
	::read(f,type);
	morphogenInfo.read(f);
}

void ShrinkModel::CD::setMorphogen(int morphogenIndex, double value)
{
	if (morphogenIndex >=0 && morphogenIndex < 1)
		morphogenInfo.array[morphogenIndex] = value;
}

double ShrinkModel::CD::getMorphogen(int morphogenIndex)
{
	if (morphogenIndex >=0 && morphogenIndex < 1)
		return morphogenInfo.array[morphogenIndex];
	else return -1;
}

void ShrinkModel::CD::setType(int type)
{
	this->type = type;
}
int ShrinkModel::CD::getType(){return type;}

int ShrinkModel::CD::numMorphogens(){return 1;}

ShrinkModel::ShrinkModel()
{
	diffusion = 0;
	decay = 1;
	rM = 1;
	rE = 1;
	drdtM = 0;
	drdtE = 0;
	shrinkRate = 0;
	minTipRadius = 0;
}

void ShrinkModel::saveToSetting(libconfig::Setting& setting)
{
	ProcessModel::saveToSetting(setting);
	setting.add("diffusion",libconfig::Setting::TypeFloat) = diffusion;
	setting.add("decay",libconfig::Setting::TypeFloat) = decay;
	setting.add("rM",libconfig::Setting::TypeFloat) = rM;
	setting.add("rE",libconfig::Setting::TypeFloat) = rE;
	setting.add("drdtM",libconfig::Setting::TypeFloat) = drdtM;
	setting.add("drdtE",libconfig::Setting::TypeFloat) = drdtE;
	setting.add("shrinkRate",libconfig::Setting::TypeFloat) = shrinkRate;
	setting.add("minTipRadius",libconfig::Setting::TypeFloat) = minTipRadius;
}

void ShrinkModel::loadParamsFromSetting(libconfig::Setting& setting)
{
	setting.lookupValue("diffusion", diffusion);
	setting.lookupValue("decay", decay);
	setting.lookupValue("rM", rM);
	setting.lookupValue("rE", rE);
	setting.lookupValue("drdtM", drdtM);
	setting.lookupValue("drdtE", drdtE);
	setting.lookupValue("shrinkRate", shrinkRate);
	setting.lookupValue("minTipRadius", minTipRadius);

	updateDiffusionParams();
}

std::list<std::string> ShrinkModel::parameters()
{
	std::string arr[] = {
			"diffusion",
			"decay",
			"rM",
			"rE",
			"drdtM",
			"drdtE",
			"shrinkRate",
			"minTipRadius",
	};
	return std::list<std::string>(arr,arr+8);
}

/// set a parameter
void ShrinkModel::set(std::string str,double val)
{
	if (str=="diffusion") diffusion  = val;
	if (str=="decay") decay = val;
	if (str=="rM") rM = val;
	if (str=="rE") rE = val;
	if (str=="drdtM") drdtM = val;
	if (str=="drdtE") drdtE = val;
	if (str=="shrinkRate") shrinkRate = val;
	if (str=="minTipRadius") minTipRadius = val;
	updateDiffusionParams();
}

/// set a parameter
double ShrinkModel::get(std::string str)
{
	if (str=="diffusion") return diffusion ;
	else if (str=="decay") return decay;
	else if (str=="rM") return rM;
	else if (str=="rE") return rE;
	else if (str=="drdtM") return drdtM;
	else if (str=="drdtE") return drdtE;
	else if (str=="shrinkRate") return shrinkRate;
	else if (str=="minTipRadius") return minTipRadius;
	else return ProcessModel::get(str);
}

void ShrinkModel::updateDiffusionParams()
{
	double diffrates[1] = {diffusion};
	double decayrates[1] = {decay};
	std::copy(diffrates,diffrates+1,diffusionRates);
	std::copy(decayrates,decayrates+1,decayRates);
}

struct DivisionInfo
{
	Cell* cell;
	bool isInternal;
	Vector3d direction;
	DivisionInfo(){}
	DivisionInfo(Cell* c, bool i, Vector3d d):cell(c),isInternal(i),direction(d){}
};


void ShrinkModel::step(Organism* o, double dt) // simulate diffusion and activate rules
{
	// foreach morphogen:
	//   simulate diffusion-decay
	//   flag all conditions satisfied
	double radiusCap = 10000;
	for(int i=0;i<1;i++)
	{
		double D = diffusionRates[i];
		double C = decayRates[i];

		// copy newvalue to old value
		BOOST_FOREACH(Cell* c, o->cells())
		{
			CD* cd = static_cast<CD*>(c->getCellContents());
			Morphogen& m = cd->morphogenInfo.array[i];
			m.oldvalue = m.value;
		}

		// for each cell, calculate the discrete laplacian
		BOOST_FOREACH(Cell* c, o->cells())
		{
			double cm = c->getCellContents()->getMorphogen(i);
			double cvol = c->vol();

			// growing tip constantly produces morphogens
			if (c->getCellContents()->getType() > 0)
			{
				c->getCellContents()->setMorphogen(i,cvol);
				radiusCap = c->r();
				continue;
			}

			// discrete laplacian is the weighted sum
			// of the difference between morphogen levels of neighbours
			// see e.g., http://en.wikipedia.org/wiki/Laplace_operator
			double laplacian = 0;
			BOOST_FOREACH(Cell* cn, o->getNeighbours(c))
			{
				double cnm = cn->getCellContents()->getMorphogen(i);
				double distance = dist(c->v()->x(),cn->v()->x());
				laplacian += (1/distance)*(std::min(cnm,cvol) - std::min(cm,cn->vol()));
			}

			// calculate new morphogen amount for this cell, and assert that it is valid
			double dcmdt = D*laplacian - C;
			double newval = std::max(0., std::min(cm + dcmdt * dt, cvol));
			c->getCellContents()->setMorphogen(i,newval);
		}
	}

	std::list<DivisionInfo> divisionsToBePerformed;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		CellContents* cd = c->getCellContents();
		double m = cd->getMorphogen(0);
		int type = cd->getType();

		// compute the radius multiplier and relative rates
		static double defRadius = c->r();
		static const double RE = rE * defRadius;
		static const double RM = rM * defRadius;
		static const double DRDTE = drdtE * defRadius;
		static const double DRDTM = drdtM * defRadius;

		if (type > 0)
		{
			c->setDrdt(-shrinkRate);
			if (c->r() < minTipRadius)
			{
				// deactivate
				cd->setType(0);
				cd->setMorphogen(0,0);
			}
		}
		else // type = 0
		{
			c->setDrdt(0);

			if (m > 0.00001)
			{
				double radiusUponSplit;
				double radiusGrowthRate;

				if (c->isBoundary())
				{
					radiusUponSplit = RE;
					radiusGrowthRate = DRDTE;
				}
				else // internal
				{
					radiusUponSplit = RM;
					radiusGrowthRate = DRDTM;
				}

				radiusUponSplit *= radiusCap;
				radiusGrowthRate *= m;

				if (c->r() > radiusUponSplit)
				{
					// divide towards the source
					Vector3d dir = Vector3d::ZERO;
					std::list<Cell*> cn = o->getNeighbours(c);
					for(std::list<Cell*>::iterator it = cn.begin(); it!=cn.end(); it++)
					{
						Cell* nei = *it;
						CD* neid = static_cast<CD*>(nei->getCellContents());
						double chem = neid->getMorphogen(0);
						dir += chem*((nei->x() - c->x()).normalise());
					}
					dir.normaliseInPlace();
					divisionsToBePerformed.push_back(DivisionInfo(c,false,dir));
				}
				else
				{
					c->setDrdt(radiusGrowthRate);
				}
			}
		}
	}

	// perform divisions
	if (divisionsToBePerformed.size()>0)
		LOG("ShrinkModel::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		// Use the new higher-level Divide()
		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR) break;

		static_cast<CD*>(res.get<0>()->getCellContents())->type = 0;
		static_cast<CD*>(res.get<1>()->getCellContents())->type = 0;

		res.get<0>()->setDrdt(0);
		res.get<1>()->setDrdt(0);

		// distribute morphogens from old cell to new cells
		Morphogen (&mo)[3] = static_cast<CD*>(d.cell->getCellContents())->morphogenInfo.array;
		for(int i=0;i<1;i++)
		{
			double m = mo[i]/2;
			static_cast<CD*>(res.get<1>()->getCellContents())->morphogenInfo.array[i] = m;
			static_cast<CD*>(res.get<0>()->getCellContents())->morphogenInfo.array[i] = m;
		}
	}
}

*/
