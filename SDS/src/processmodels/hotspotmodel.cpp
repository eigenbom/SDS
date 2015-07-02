/*
 * hotspotmodel.cpp
 *
 *  Created on: 12/02/2010
 *      Author: ben
 */

#include "hotspotmodel.h"
#include "organism.h"
#include "transform.h"
#include "cell.h"

#include "random.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

HotSpotModel::HotSpotModel()
:ProcessModel(3)
{
	for(int i=0; i<3; i++)
	{
		mMorphogenGrowthRates[i] = 0;
		setMorphogenDiffusion(i,0);
		setMorphogenDecay(i,0);
	}
}

void HotSpotModel::setup()
{
	BOOST_FOREACH(Cell* c, organism()->cells())
	{
		// tag cells with morphogen as morphogen source
		CellContents* cc = c->getCellContents();
		cc->setType(0);
		if (cc->getMorphogen(0) > 0.001)
		{
			cc->setType(1);
		}
	}
}

void HotSpotModel::writeStatic(std::ostream& o)
{
	ProcessModel::writeStatic(o);

	for(int i=0; i<3; i++)
		write(o, mMorphogenGrowthRates[i]);
}

void HotSpotModel::readStaticParams(std::istream& o)
{
	ProcessModel::readStaticParams(o);

	for(int i=0; i<3; i++)
		read(o, mMorphogenGrowthRates[i]);
}

std::list<std::string> HotSpotModel::parameters()
{
	std::list<std::string> p = ProcessModel::parameters();
	p.push_back("r0");
	p.push_back("r1");
	p.push_back("r2");
	p.push_back("diffusion0");
	p.push_back("decay0");
	p.push_back("diffusion1");
	p.push_back("decay1");
	p.push_back("diffusion2");
	p.push_back("decay2");
	return p;
}

/// set a parameter
void HotSpotModel::set(std::string str,double val)
{
	ProcessModel::set(str,val);

	if (str=="r0") mMorphogenGrowthRates[0] = val;
	if (str=="r1") mMorphogenGrowthRates[1] = val;
	if (str=="r2") mMorphogenGrowthRates[2] = val;

	if (str=="diffusion0") setMorphogenDiffusion(0,val);
	if (str=="decay0") setMorphogenDecay(0,val);
	if (str=="diffusion1") setMorphogenDiffusion(1,val);
	if (str=="decay1") setMorphogenDecay(1,val);
	if (str=="diffusion2") setMorphogenDiffusion(2,val);
	if (str=="decay2") setMorphogenDecay(2,val);
}

/// set a parameter
double HotSpotModel::get(std::string str)
{
	if (str=="r0") return mMorphogenGrowthRates[0];
	if (str=="r1") return mMorphogenGrowthRates[1];
	if (str=="r2") return mMorphogenGrowthRates[2];

	if (str=="diffusion0") return getMorphogenDiffusion(0);
	if (str=="decay0") return getMorphogenDecay(0);
	if (str=="diffusion1") return getMorphogenDiffusion(1);
	if (str=="decay1") return getMorphogenDecay(1);
	if (str=="diffusion2") return getMorphogenDiffusion(2);
	if (str=="decay2") return getMorphogenDecay(2);

	return ProcessModel::get(str);
}

void HotSpotModel::step(Organism* o, double dt)
{
	static const int NORMAL = 0;
	static const int SOURCE = 1;

	// Distribute growth morphogen
	std::set<Cell*> skip;
	BOOST_FOREACH(Cell* c, o->cells())
	{
		if (c->getCellContents()->getType() == SOURCE)
			skip.insert(c);
	}
	simulateMorphogenDiffusion(o, dt, skip);

	// Stimulate growth
	BOOST_FOREACH(Cell* c, o->cells())
	{
		// sum contributions from all morphogens
		double drdt = 0;
		for(int i=0;i<3;i++)
			drdt += mMorphogenGrowthRates[i]*c->getCellContents()->getMorphogen(i);
		c->setDrdt(drdt);
	}
}

