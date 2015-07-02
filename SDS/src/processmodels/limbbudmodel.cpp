/*
 * limbbudmodel.cpp
 *
 *  Created on: 24/11/2009
 *      Author: ben
 */

#include "limbbudmodel.h"
#include "organism.h"
#include "transform.h"
#include "cell.h"

#include "random.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

LimbBudModel::LimbBudModel()
:ProcessModel(3) // has to remain 3 for backwards compatibility
{
	diffusionFgf8 = .1;
	diffusionFgf10 = .1;
	decayFgf8 = 0.001;
	decayFgf10 = 0.001;
	aerS = 0.0001;
	aerR = 0.00008;
	pzSE = 0.000019;
	pzSM = 0.000008;
	pzR = 0.00008;
	rE = 1.2;
	rM = 1.2;
	drdtE = 1.0;
	drdtM = 1.0;
}

void LimbBudModel::setup()
{
	ProcessModel::setup();

	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		CellContents* cc = c->getCellContents();
		double morph = cc->getMorphogen(0);
		if (morph > .5)
		{
			cc->setType(3);
			cc->setMorphogen(2,1);
		}
	}
}

void LimbBudModel::saveToSetting(libconfig::Setting& setting)
{
	ProcessModel::saveToSetting(setting);
	/*
	setting.add("diffusionFgf8",libconfig::Setting::TypeFloat) = diffusionFgf8;
	setting.add("diffusionFgf10",libconfig::Setting::TypeFloat) = diffusionFgf10;
	setting.add("decayFgf8",libconfig::Setting::TypeFloat) = decayFgf8;
	setting.add("decayFgf10",libconfig::Setting::TypeFloat) = decayFgf10;
	setting.add("aerS",libconfig::Setting::TypeFloat) = aerS;
	setting.add("aerR",libconfig::Setting::TypeFloat) = aerR;
	setting.add("pzSE",libconfig::Setting::TypeFloat) = pzSE;
	setting.add("pzSM",libconfig::Setting::TypeFloat) = pzSM;
	setting.add("pzR",libconfig::Setting::TypeFloat) = pzR;
	setting.add("rE",libconfig::Setting::TypeFloat) = rE;
	setting.add("rM",libconfig::Setting::TypeFloat) = rM;
	setting.add("drdtE",libconfig::Setting::TypeFloat) = drdtE;
	setting.add("drdtM",libconfig::Setting::TypeFloat) = drdtM;
	*/
}

void LimbBudModel::loadParamsFromSetting(libconfig::Setting& setting)
{
	ProcessModel::loadParamsFromSetting(setting);
	/*
	setting.lookupValue("diffusionFgf8", diffusionFgf8);
	setting.lookupValue("diffusionFgf10", diffusionFgf10);
	setting.lookupValue("decayFgf8", decayFgf8);
	setting.lookupValue("decayFgf10", decayFgf10);
	setting.lookupValue("aerS", aerS);
	setting.lookupValue("aerR", aerR);
	setting.lookupValue("pzSE", pzSE);
	setting.lookupValue("pzSM", pzSM);
	setting.lookupValue("pzR", pzR);
	setting.lookupValue("rE", rE);
	setting.lookupValue("rM", rM);
	setting.lookupValue("drdtE", drdtE);
	setting.lookupValue("drdtM", drdtM);
	*/

	updateDiffusionParams();
}

void LimbBudModel::writeStatic(std::ostream& o)
{
	ProcessModel::writeStatic(o);

	// write parameters
	write(o,diffusionFgf8);
	write(o,diffusionFgf10);
	write(o,decayFgf8);
	write(o,decayFgf10);
	write(o,aerS);
	write(o,aerR);
	write(o,pzSE);
	write(o,pzSM);
	write(o,pzR);
	write(o,rE);
	write(o,rM);
	write(o,drdtE);
	write(o,drdtM);
}

void LimbBudModel::readStaticParams(std::istream& o)
{
	// read parameters
	read(o,diffusionFgf8);
	read(o,diffusionFgf10);
	read(o,decayFgf8);
	read(o,decayFgf10);
	read(o,aerS);
	read(o,aerR);
	read(o,pzSE);
	read(o,pzSM);
	read(o,pzR);
	read(o,rE);
	read(o,rM);
	read(o,drdtE);
	read(o,drdtM);
}
std::list<std::string> LimbBudModel::parameters()
{
	std::string arr[] = {
			"diffusionFgf8",
			"diffusionFgf10",
			"decayFgf8",
			"decayFgf10",
			"aerS",
			"aerR",
			"pzSE",
			"pzSM",
			"pzR",
			"rE",
			"rM",
			"drdtE",
			"drdtM"
	};
	return std::list<std::string>(arr,arr+13);
}

/// set a parameter
void LimbBudModel::set(std::string str,double val)
{
	ProcessModel::set(str,val);

	if (str=="diffusionFgf8") diffusionFgf8  = val;
	if (str=="diffusionFgf10") diffusionFgf10 = val;
	if (str=="decayFgf8") decayFgf8 = val;
	if (str=="decayFgf10") decayFgf10 = val;
	if (str=="aerS") aerS = val;
	if (str=="aerR") aerR = val;
	if (str=="pzSE") pzSE = val;
	if (str=="pzSM") pzSM = val;
	if (str=="pzR") pzR = val;
	if (str=="rE") rE = val;
	if (str=="rM") rM = val;
	if (str=="drdtE") drdtE = val;
	if (str=="drdtM") drdtM = val;
	updateDiffusionParams();
}

/// set a parameter
double LimbBudModel::get(std::string str)
{
	if (str=="diffusionFgf8") return diffusionFgf8 ;
	if (str=="diffusionFgf10") return diffusionFgf10;
	if (str=="decayFgf8") return decayFgf8;
	if (str=="decayFgf10") return decayFgf10;
	if (str=="aerS") return aerS;
	if (str=="aerR") return aerR;
	if (str=="pzSE") return pzSE;
	if (str=="pzSM") return pzSM;
	if (str=="pzR") return pzR;
	if (str=="rE") return rE;
	if (str=="rM") return rM;
	if (str=="drdtE") return drdtE;
	if (str=="drdtM") return drdtM;
	return ProcessModel::get(str);
}

void LimbBudModel::updateDiffusionParams()
{
	setMorphogenDiffusion(0,diffusionFgf8);
	setMorphogenDiffusion(1,diffusionFgf10);

	setMorphogenDecay(0, decayFgf8);
	setMorphogenDecay(1, decayFgf10);
}

struct DivisionInfo
{
	Cell* cell;
	bool isInternal;
	Vector3d direction;
	DivisionInfo(){}
	DivisionInfo(Cell* c, bool i, Vector3d d):cell(c),isInternal(i),direction(d){}
};

void LimbBudModel::step(Organism* o, double dt) // simulate diffusion and activate rules
{
	// make sure the growing tip is replenished
	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->getCellContents()->getType() == 3)
			skip.insert(c);
	}

	// simulate diffusion the new way
	simulateMorphogenDiffusion(o, dt, skip);

	// update the internal states, and register the events
	// NOTE: we can't just _divide_ in this loop because division adds cells to o->cells()
	// instead we just log the divisions and perform them all after this loop
	/* limb bud program */
	// const double AER_STIM = LimbBudModel::aerS;
	const double AER_RATE = LimbBudModel::aerR;

	const double PZ_STIMM = LimbBudModel::pzSM;
	const double PZ_STIME = LimbBudModel::pzSE;
	const double PZ_RATE = LimbBudModel::pzR;

	std::list<DivisionInfo> divisionsToBePerformed;

	BOOST_FOREACH(Cell* c, o->cells())
	{
		CellContents* cc = c->getCellContents();
		double m[2] = {cc->getMorphogen(0), cc->getMorphogen(1)};
		int type = cc->getType();

		static double defRadius = c->r();

		const double RE = LimbBudModel::rE * defRadius;
		const double RM = LimbBudModel::rM * defRadius;

		const double DRDTE = LimbBudModel::drdtE * defRadius;
		const double DRDTM = LimbBudModel::drdtM * defRadius;

		c->setDrdt(0);

		if (c->isBoundary())
		{
			if (type == 3)
			{
				m[0] = std::min(1.,m[0]+AER_RATE);
			}
			else if (m[0] > PZ_STIME)
			{
				if (type == 0)
				{
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
					else
					{
						c->setDrdt(DRDTE);
					}
				}
			}
		}
		else
		{
			if (m[0] > PZ_STIMM)
			{
				cc->setType(2);
				c->setDrdt(DRDTM);
				cc->setMorphogen(1,std::min(1.,m[1]+PZ_RATE));

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
				cc->setType(0);
		}
	}

	if (divisionsToBePerformed.size()>0)
			LOG("LimbBudModel::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		// Use the new higher-level Divide()
		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR) break;

		(res.get<0>()->getCellContents())->setType(0);
		(res.get<1>()->getCellContents())->setType(0);

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



LimbBudModelWithGrowth::LimbBudModelWithGrowth()
:LimbBudModel(),mPassiveCellGrowthRate(0)
{

}

void LimbBudModelWithGrowth::saveToSetting(libconfig::Setting& setting)
{
	LimbBudModel::saveToSetting(setting);
	//setting.add("passiveCellGrowthRate",libconfig::Setting::TypeFloat) = mPassiveCellGrowthRate;
}

void LimbBudModelWithGrowth::loadParamsFromSetting(libconfig::Setting& setting)
{
	LimbBudModel::loadParamsFromSetting(setting);
	//setting.lookupValue("passiveCellGrowthRate", mPassiveCellGrowthRate);
}

std::list<std::string> LimbBudModelWithGrowth::parameters()
{
	std::list<std::string> p = LimbBudModel::parameters();
	p.push_back("passiveCellGrowthRate");
	return p;
}

void LimbBudModelWithGrowth::set(std::string p, double v)
{
	if (p=="passiveCellGrowthRate") mPassiveCellGrowthRate = v;
	else
		LimbBudModel::set(p,v);
}

double LimbBudModelWithGrowth::get(std::string p)
{
	if (p=="passiveCellGrowthRate") return mPassiveCellGrowthRate;
	else return LimbBudModel::get(p);
}

void LimbBudModelWithGrowth::step(Organism* o, double dt)
{
	// similar to the limb bud model,
	// with an extra rule:
	// - grow slightly larger if you aren't involved in the limb growth

	// make sure the growing tip is replenished
	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->getCellContents()->getType() == 3)
			skip.insert(c);
	}

	// simulate diffusion the new way
	simulateMorphogenDiffusion(o, dt, skip);

	// update the internal states, and register the events
	// NOTE: we can't just _divide_ in this loop because division adds cells to o->cells()
	// instead we just log the divisions and perform them all after this loop
	std::list<DivisionInfo> divisionsToBePerformed;

	BOOST_FOREACH(Cell* c, o->cells())
	{
		CellContents* cc = c->getCellContents();

		static double defRadius = c->r();

		const double RE = LimbBudModel::rE * defRadius;
		const double RM = LimbBudModel::rM * defRadius;

		const double DRDTE = LimbBudModel::drdtE * defRadius;
		const double DRDTM = LimbBudModel::drdtM * defRadius;

		c->setDrdt(0);

		// true if this cell is currently involved in the limb bud growth
		bool active = false;

		if (c->isBoundary())
		{
			if (cc->getType() == 3)
			{
				cc->setMorphogen(0, std::min(1.,cc->getMorphogen(0)+aerR));
				active = true;
			}
			else if (cc->getMorphogen(0) > pzSE)
			{
				if (cc->getType()==0)
				{
					if (c->r() > RE)
					{
						// divide towards the source
						Vector3d dir = Vector3d::ZERO;
						std::list<Cell*> cn = o->getNeighbours(c);
						for(std::list<Cell*>::iterator it = cn.begin(); it!=cn.end(); it++)
						{
							Cell* nei = *it;
							CellContents* cc = nei->getCellContents();
							double chem = cc->getMorphogen(0);
							dir += chem*((nei->x() - c->x()).normalise());
						}
						dir.normaliseInPlace();
						divisionsToBePerformed.push_back(DivisionInfo(c,false,dir));
						active = true;
					}
					else
					{
						c->setDrdt(DRDTE);
						active = true;
					}
				}
			}
		}
		else
		{
			if (cc->getMorphogen(0) > pzSM)
			{
				active = true;
				cc->setType(2);
				c->setDrdt(DRDTM);
				cc->setMorphogen(1, std::min(1.,cc->getMorphogen(1)+pzR));

				if (c->r() > RM)
				{
					// divide into direction of most morphogen, weighted sum
					Vector3d dir = Vector3d::ZERO;
					std::list<Cell*> cn = o->getNeighbours(c);
					for(std::list<Cell*>::iterator it = cn.begin(); it!=cn.end(); it++)
					{
						Cell* nei = *it;
						CellContents* cc = nei->getCellContents();
						double chem = cc->getMorphogen(0);
						dir += chem*((nei->x() - c->x()).normalise());
					}
					dir.normaliseInPlace();
					divisionsToBePerformed.push_back(DivisionInfo(c,true,dir));
				}
			}
			else
				cc->setType(0);
		}

		if (not active)
		{
			c->setDrdt(mPassiveCellGrowthRate);
		}
	}

	if (divisionsToBePerformed.size()>0)
		LOG("LimbBudModelWithGrowth::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		// Use the new higher-level Divide()
		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR) break;

		(res.get<0>()->getCellContents())->setType(0);
		(res.get<1>()->getCellContents())->setType(0);

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



TentacleWithAttractor::TentacleWithAttractor()
:LimbBudModelWithGrowth(),mAttractionStrength(0),mAttractor()
{

}

void TentacleWithAttractor::saveToSetting(libconfig::Setting& setting)
{
	LimbBudModelWithGrowth::saveToSetting(setting);
	//setting.add("attractionStrength",libconfig::Setting::TypeFloat) = mAttractionStrength;

	libconfig::Setting& s = setting.add("attractor",libconfig::Setting::TypeArray);
	s.add(libconfig::Setting::TypeFloat) = mAttractor.x();
	s.add(libconfig::Setting::TypeFloat) = mAttractor.y();
	s.add(libconfig::Setting::TypeFloat) = mAttractor.z();


	//s[0] = mAttractor.x();
	//s[1] = mAttractor.x();
	//s[2] = mAttractor.x();
}

void TentacleWithAttractor::loadParamsFromSetting(libconfig::Setting& setting)
{
	LimbBudModelWithGrowth::loadParamsFromSetting(setting);
	//setting.lookupValue("attractionStrength", mAttractionStrength);

	libconfig::Setting& s = setting["attractor"];
	if (!s.isArray())
	{
		std::cerr << "Expected \"attractor\" parameter to be an array." << std::endl;
	}

	mAttractor.x(s[0]);
	mAttractor.y(s[1]);
	mAttractor.z(s[2]);
}

std::list<std::string> TentacleWithAttractor::parameters()
{
	std::list<std::string> p = LimbBudModelWithGrowth::parameters();
	p.push_back("attractionStrength");
	return p;
}

void TentacleWithAttractor::set(std::string p, double v)
{
	if (p=="attractionStrength") mAttractionStrength = v;
	else
		LimbBudModelWithGrowth::set(p,v);
}

double TentacleWithAttractor::get(std::string p)
{
	if (p=="attractionStrength") return mAttractionStrength;
	else return LimbBudModelWithGrowth::get(p);
}

void TentacleWithAttractor::beforePhysicalStep(Organism* o, double dt)
{
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->isBoundary())
		{
			if (c->getCellContents()->getType() == 3) // the tip of the growth
			{
				Vector3d realdir = (mAttractor-c->x()).normalise()*mAttractionStrength;
				c->v()->addF(realdir);
			}
		}
	}
}

void TentacleWithAttractor::step(Organism* o, double dt)
{
	// similar to the limb bud model,
	// with an extra rule:
	// - grow slightly larger if you aren't involved in the limb growth

	// make sure the growing tip is replenished
	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->getCellContents()->getType() == 3)
			skip.insert(c);
	}

	// simulate diffusion the new way
	simulateMorphogenDiffusion(o, dt, skip);

	// update the internal states, and register the events
	// NOTE: we can't just _divide_ in this loop because division adds cells to o->cells()
	// instead we just log the divisions and perform them all after this loop
	/* limb bud program */
	// const double AER_STIM = LimbBudModel::aerS;
	const double AER_RATE = LimbBudModel::aerR;

	const double PZ_STIMM = LimbBudModel::pzSM;
	const double PZ_STIME = LimbBudModel::pzSE;
	const double PZ_RATE = LimbBudModel::pzR;

	std::list<DivisionInfo> divisionsToBePerformed;

	BOOST_FOREACH(Cell* c, o->cells())
	{
		CellContents* cc = c->getCellContents();
		double m[2] = {cc->getMorphogen(0), cc->getMorphogen(1)};
		int type = cc->getType();

		static double defRadius = c->r();

		const double RE = LimbBudModel::rE * defRadius;
		const double RM = LimbBudModel::rM * defRadius;

		const double DRDTE = LimbBudModel::drdtE * defRadius;
		const double DRDTM = LimbBudModel::drdtM * defRadius;

		// true if this cell is currently involved in the limb bud growth
		bool active = false;

		c->setDrdt(0);

		if (c->isBoundary())
		{
			if (type == 3)
			{
				m[0] = std::min(1.,m[0]+AER_RATE);
				cc->setMorphogen(0,m[0]);
				active = true;
			}
			else if (m[0] > PZ_STIME)
			{
				if (type == 0)
				{
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
						active = true;
					}
					else
					{
						c->setDrdt(DRDTE);
						active = true;
					}
				}
			}
		}
		else
		{
			if (m[0] > PZ_STIMM)
			{
				active = true;
				cc->setType(2);
				c->setDrdt(DRDTM);
				cc->setMorphogen(1,std::min(1.,m[1]+PZ_RATE));

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
				cc->setType(0);
		}


		if (not active)
		{
			c->setDrdt(mPassiveCellGrowthRate);
		}
	}

	if (divisionsToBePerformed.size()>0)
			LOG("TentacleWithAttractor::step, simultaneous divisions = " << divisionsToBePerformed.size() << "\n");

	// now perform division
	BOOST_FOREACH(DivisionInfo& d, divisionsToBePerformed)
	{
		// Use the new higher-level Divide()
		boost::tuple<Cell*,Cell*> res = Transform::Divide(d.cell,d.direction,o);

		if (Transform::state==Transform::TRANSFORM_ERROR) break;

		(res.get<0>()->getCellContents())->setType(0);
		(res.get<1>()->getCellContents())->setType(0);

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





/************************************************************/


LimbGrowth::LimbGrowth(int nm, int nv)
:ProcessModel(nm,nv) // incompatible with previous model
 ,rE(1),rM(1),drdtE(0),drdtM(0)
 //,mPassiveCellGrowthRate(0)
{

}

void LimbGrowth::setup()
{
	ProcessModel::setup();

	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		CellContents* cc = c->getCellContents();
		double morph = cc->getMorphogen(0);
		if (morph > .5)
		{
			cc->setType(3);
			cc->setMorphogen(0,1);
		}
	}
}

void LimbGrowth::writeStatic(std::ostream& o)
{
	ProcessModel::writeStatic(o);

	// write parameters
	write(o,getMorphogenDiffusion(0));
	write(o,getMorphogenDecay(0));
	write(o,rE);
	write(o,rM);
	write(o,drdtE);
	write(o,drdtM);
}

void LimbGrowth::readStaticParams(std::istream& o)
{
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
}
std::list<std::string> LimbGrowth::parameters()
{
	std::list<std::string> p = ProcessModel::parameters();
	std::string arr[] = {
			"diffusion",
			"decay",
			"rE",
			"rM",
			"drdtE",
			"drdtM"
	};
	p.insert(p.end(),arr,arr+6);
	//p.push_back("passiveCellGrowthRate");
	return p;
}

/// set a parameter
void LimbGrowth::set(std::string str,double val)
{
	ProcessModel::set(str,val);

	if (str=="diffusion") setMorphogenDiffusion(0, val);
	if (str=="decay") setMorphogenDecay(0, val);
	if (str=="rE") rE = val;
	if (str=="rM") rM = val;
	if (str=="drdtE") drdtE = val;
	if (str=="drdtM") drdtM = val;
	// if (str=="passiveCellGrowthRate") mPassiveCellGrowthRate = val;
}

/// set a parameter
double LimbGrowth::get(std::string str)
{
	if (str=="diffusion") return getMorphogenDiffusion(0);
	if (str=="decay") return getMorphogenDecay(0);
	if (str=="rE") return rE;
	if (str=="rM") return rM;
	if (str=="drdtE") return drdtE;
	if (str=="drdtM") return drdtM;
	// if (str=="passiveCellGrowthRate") return mPassiveCellGrowthRate;

	return ProcessModel::get(str);
}

void LimbGrowth::step(Organism* o, double dt)
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
		else
		{
			cc->setType(NORMAL);
			//c->setDrdt(mPassiveCellGrowthRate);
		}
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

		double m = d.cell->getCellContents()->getMorphogen(0)/2; //  mo[i]/2;
		(res.get<1>()->getCellContents())->setMorphogen(0,m);
		(res.get<0>()->getCellContents())->setMorphogen(0,m);
	}
}



/************************************************************/


LimbGrowthWithFreeze::LimbGrowthWithFreeze()
:ProcessModel(1) // incompatible with previous model
 ,rE(1),rM(1),drdtE(0),drdtM(0)
{

}

void LimbGrowthWithFreeze::setup()
{
	ProcessModel::setup();

	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		CellContents* cc = c->getCellContents();
		double morph = cc->getMorphogen(0);
		if (morph > .5)
		{
			cc->setType(3);
			cc->setMorphogen(0,1);
		}
	}
}

void LimbGrowthWithFreeze::writeStatic(std::ostream& o)
{
	ProcessModel::writeStatic(o);

	// write parameters
	write(o,getMorphogenDiffusion(0));
	write(o,getMorphogenDecay(0));
	write(o,rE);
	write(o,rM);
	write(o,drdtE);
	write(o,drdtM);
}

void LimbGrowthWithFreeze::readStaticParams(std::istream& o)
{
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
}
std::list<std::string> LimbGrowthWithFreeze::parameters()
{
	std::list<std::string> p = ProcessModel::parameters();
	std::string arr[] = {
			"diffusion",
			"decay",
			"rE",
			"rM",
			"drdtE",
			"drdtM"
	};
	p.insert(p.end(),arr,arr+6);
	return p;
}

/// set a parameter
void LimbGrowthWithFreeze::set(std::string str,double val)
{
	ProcessModel::set(str,val);

	if (str=="diffusion") setMorphogenDiffusion(0, val);
	if (str=="decay") setMorphogenDecay(0, val);
	if (str=="rE") rE = val;
	if (str=="rM") rM = val;
	if (str=="drdtE") drdtE = val;
	if (str=="drdtM") drdtM = val;
}

/// set a parameter
double LimbGrowthWithFreeze::get(std::string str)
{
	if (str=="diffusion") return getMorphogenDiffusion(0);
	if (str=="decay") return getMorphogenDecay(0);
	if (str=="rE") return rE;
	if (str=="rM") return rM;
	if (str=="drdtE") return drdtE;
	if (str=="drdtM") return drdtM;

	return ProcessModel::get(str);
}

void LimbGrowthWithFreeze::step(Organism* o, double dt)
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
		if (cc->getMorphogen(0) > 0.000000001)
		{
			c->v()->setFrozen(false);

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

			c->v()->setFrozen(true);
		}
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

		double m = d.cell->getCellContents()->getMorphogen(0)/2; //  mo[i]/2;
		(res.get<1>()->getCellContents())->setMorphogen(0,m);
		(res.get<0>()->getCellContents())->setMorphogen(0,m);
	}
}















