/*
 * processmodel.cpp
 *
 *  Created on: 19/02/2009
 *      Author: ben
 */

#include "processmodel.h"
#include "limbbudmodel.h"
#include "shrinkmodel.h"
#include "patternedlimbmodel.h"
#include "hotspotmodel.h"
#include "plantmodel.h"
#include "curling.h"

#include "organism.h"
#include "transform.h"
#include "cell.h"
#include "random.h"
#include "bstreamable.h"

#include <cfloat>
#include <iostream>
#include <algorithm>
#include <boost/foreach.hpp>

#include "libconfig.h++"
using namespace libconfig;

Morphogen::Morphogen():value(0),oldvalue(0){}

Morphogen::operator double(){return value;}
double Morphogen::operator=(double v){return value = v;}
void Morphogen::write(std::ostream& f)
{
	::write(f,value);
	::write(f,oldvalue);
}
void Morphogen::read(std::istream& is)
{
	::read(is,value);
	::read(is,oldvalue);
}

CellContents::CellContents(int m, int n)
:type(0),mNumMorphogens(m),mMorphogens(NULL),mNumVars(n),mVars(NULL)
{
	if (m>0)
		mMorphogens = new Morphogen[m];
	if (n>0)
		mVars = new double[n];
}
CellContents::~CellContents()
{
	if (mMorphogens)
		delete[]mMorphogens;
	if (mVars)
		delete[]mVars;
}

void CellContents::write(std::ostream& o)
{
	::write(o,type);
	for(int i=0;i<mNumMorphogens;++i)
		mMorphogens[i].write(o);
	for(int i=0;i<mNumVars;++i)
		::write(o,mVars[i]);
}

void CellContents::read(std::istream& f)
{
	::read(f,type);
	for(int i=0;i<mNumMorphogens;++i)
		mMorphogens[i].read(f);
	for(int i=0;i<mNumVars;++i)
		::read(f,mVars[i]);
}

// subclass accessors
void CellContents::setMorphogen(int morphogenIndex, double value){
	if (morphogenIndex < 0 || morphogenIndex >= mNumMorphogens) return;
	else mMorphogens[morphogenIndex] = value;
}

double CellContents::getMorphogen(int morphogenIndex) const
{
	if (morphogenIndex < 0 || morphogenIndex >= mNumMorphogens) return -1;
	else return mMorphogens[morphogenIndex];
}

void CellContents::setType(int type)
{
	this->type = type;
}

int CellContents::getType() const
{
	return type;
}

int CellContents::numMorphogens() const
{
	return mNumMorphogens;
}

void CellContents::setVar(int varIndex, double value)
{
	if (varIndex < 0 || varIndex >= mNumVars) return;
	else mVars[varIndex] = value;
}

double CellContents::getVar(int varIndex) const
{
	if (varIndex < 0 || varIndex >= mNumVars) return 0;
	else return mVars[varIndex];
}

int CellContents::numVars() const
{
	return mNumVars;
}

ProcessModel::NoProcessModelException::NoProcessModelException(std::string processmodel) throw()
{
	message = std::string("NoProcessModelException: Process Model \"") + processmodel + "\" was not found. ";
}

const char* ProcessModel::NoProcessModelException::what() const throw()
{
	return message.c_str();
}

ProcessModel::ProcessModel(int _numMorphogens, int numvars)
:mNumMorphogens(_numMorphogens),mNumVars(numvars),mDiffusion(NULL),mDecay(NULL),mOrganism(NULL)
{
	if (mNumMorphogens > 0)
	{
		mDiffusion = new double[mNumMorphogens];
		mDecay = new double[mNumMorphogens];

		for(int i=0;i<mNumMorphogens;i++)
		{
			mDiffusion[i] = 0;
			mDecay[i] = 0;
		}
	}
}

ProcessModel::~ProcessModel()
{
	if (mDiffusion) delete[]mDiffusion;
	if (mDecay) delete[]mDecay;
}

void ProcessModel::setOrganism(Organism* o)
{
	mOrganism = o;
}

void ProcessModel::setNumberOfMorphogens(int nm)
{
	if (mNumMorphogens > 0)
	{
		delete[]mDiffusion;
		delete[]mDecay;
	}

	if (nm <= 0)
	{
		mDiffusion = NULL;
		mDecay = NULL;
		return;
	}

	mDiffusion = new double[nm];
	mDecay = new double[nm];
	mNumMorphogens = nm;

	for(int i=0;i<mNumMorphogens;i++)
	{
		mDiffusion[i] = 0;
		mDecay[i] = 0;
	}
}

void ProcessModel::simulateMorphogenDiffusion(Organism* o, double dt, std::set<Cell*> skip)
{
	// foreach morphogen:
	//   simulate diffusion-decay
	//   flag all conditions satisfied
	for(int i=0;i<mNumMorphogens;i++)
	{
		double D = mDiffusion[i];
		double C = mDecay[i];

		// copy newvalue to old value
		BOOST_FOREACH(Cell* c, o->cells())
		{
			CellContents* cc = c->getCellContents();
			Morphogen* m = cc->mMorphogens;
			m[i].oldvalue = m[i].value;
		}

		// XXX: Can speed this up by calculating each laplacian term per edge
		// we are actually doing DOUBLE the work here... at the expense of readibility

		// for each cell, calculate the discrete laplacian
		BOOST_FOREACH(Cell* c, o->cells())
		{
			if (skip.count(c)>0) continue;

			CellContents* cc = c->getCellContents();
			Morphogen* m = cc->mMorphogens;
			double cm = m[i];
			double cvol = c->vol();

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
			// TEMPORARY FIX!!
			// double dcmdt = D*laplacian - C*cm;
			double dcmdt = D*laplacian - C;

			double newval = std::max(0., std::min(cm + dcmdt * dt, cvol));
			c->getCellContents()->setMorphogen(i,newval);
		}
	}
}

void ProcessModel::simulateMorphogenDiffusion(Organism* o, double dt, std::set<Cell*> skip, int morph)
{
	// foreach morphogen:
	//   simulate diffusion-decay
	//   flag all conditions satisfied
	for(int i=0;i<mNumMorphogens;i++)
	{
		double D = mDiffusion[i];
		double C = mDecay[i];

		// copy newvalue to old value
		BOOST_FOREACH(Cell* c, o->cells())
		{
			CellContents* cc = c->getCellContents();
			Morphogen* m = cc->mMorphogens;
			m[i].oldvalue = m[i].value;
		}

		// XXX: Can speed this up by calculating each laplacian term per edge
		// we are actually doing DOUBLE the work here... at the expense of readibility

		// for each cell, calculate the discrete laplacian
		BOOST_FOREACH(Cell* c, o->cells())
		{
			if (i==morph and skip.count(c)>0) continue;

			CellContents* cc = c->getCellContents();
			Morphogen* m = cc->mMorphogens;
			double cm = m[i];
			double cvol = c->vol();

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
			// TEMPORARY FIX!!
			// double dcmdt = D*laplacian - C*cm;
			double dcmdt = D*laplacian - C;

			double newval = std::max(0., std::min(cm + dcmdt * dt, cvol));
			c->getCellContents()->setMorphogen(i,newval);
		}
	}

}

ProcessModel* ProcessModel::readStatic(std::istream& is)
{
	std::string name;
	read(is,name);
	ProcessModel* pm = create(name);
	if (pm==NULL) return NULL;
	pm->readStaticParams(is);
	return pm;
}

// load a new process model from a string
ProcessModel* ProcessModel::loadFromString(std::string str)
{
	libconfig::Config config;

	// config.readString(str); // not available in 1.3.2
	// instead wrap string in a file object...

	char buffer[L_tmpnam];
	std::tmpnam(buffer);

	//std::cout << "Writing to temporary file: \"" << buffer << "\"\n";

	FILE* f = std::fopen(buffer,"wt"); //  std::fopen("C:\\tmp\\whatever.tmp","w");
	std::fprintf(f,"%s",str.c_str());
	std::fclose(f);

	try {
		config.readFile(buffer);
		config.setAutoConvert(true);
		std::remove(buffer);
		Setting& sim = config.lookup("processModel");
		ProcessModel* pm = loadFromSetting(sim);
		if (pm==NULL)
		{
			std::cerr << "No process model with that \"type\"." << std::endl;
			return NULL;
		}
		else
			return pm;
	}
	catch (FileIOException& e)
	{
		std::cerr << "Can't load temp file!";
		return NULL;
	}
	catch (NoProcessModelException& e)
	{
		std::cerr << "Can't find a process model by that name.";
		return NULL;
	}
	catch (SettingNotFoundException& e)
	{
		std::cerr << "No \"processModel\" setting in config";
		return NULL;
	}
	catch (...)
	{
		std::cerr << "Can't parse process model config for some unknown reason!";
		return NULL;
	}

	return NULL;
}

/**
 * Creates a process model given a libconfig::Setting describing the model.
 */
ProcessModel* ProcessModel::loadFromSetting(libconfig::Setting& setting) throw(NoProcessModelException)
{
	std::string type;
	if (setting.lookupValue("type",type))
	{
		ProcessModel* pm = create(type);
		if (pm==NULL)
		{
			throw NoProcessModelException(type);
		}
		else
		{
			pm->loadParamsFromSetting(setting);
			return pm;
		}
	}
	else return NULL;
}

void ProcessModel::saveToSetting(libconfig::Setting& setting)
{
	setting.add("type",libconfig::Setting::TypeString) = name();

	BOOST_FOREACH(std::string p, this->parameters())
	{
		setting.add(p,libconfig::Setting::TypeFloat) = this->get(p);
	}
}

void ProcessModel::loadParamsFromSetting(libconfig::Setting& s)
{
	BOOST_FOREACH(std::string p, this->parameters())
	{
		double value = 0;
		s.lookupValue(p, value);
		this->set(p, value);
	}
}

CellContents* ProcessModel::newCellContents()
{
	return new CellContents(mNumMorphogens,mNumVars);
}

CellContents* ProcessModel::readCellContents(std::istream& bin)
{
	// nothing to read here
	CellContents* pi = newCellContents();
	pi->read(bin);
	return pi;
}

void ProcessModel::writeStatic(std::ostream& binaryOStream)
{
	// write name
	std::string n = this->name();
	write(binaryOStream,n);
}

int ProcessModel::numMorphogens() const
{
	return mNumMorphogens;
}

int ProcessModel::numVars() const
{
	return mNumVars;
}

void ProcessModel::setMorphogenDiffusion(int i, double d){
	mDiffusion[i] = d;
}

double ProcessModel::getMorphogenDiffusion(int i) const
{
	return mDiffusion[i];
}

void ProcessModel::setMorphogenDecay(int i, double d)
{
	mDecay[i] = d;
}

double ProcessModel::getMorphogenDecay(int i) const
{
	return mDecay[i];
}

void ProcessModel::set(std::string, double val)
{
	// pass
}

double ProcessModel::get(std::string)
{
	return 0;
}

Organism* ProcessModel::organism()
{
	return mOrganism;
}


ProcessModel* ProcessModel::create(std::string name)
{
	ProcessModel* pm = NULL;
	if (name=="NoProcessModel")
		pm = new NoProcessModel();
	else if (name=="LimbBudModel")
		pm = new LimbBudModel();
	else if (name=="LimbGrowth")
		pm = new LimbGrowth();
	else if (name=="LimbGrowthWithFreeze")
			pm = new LimbGrowthWithFreeze();
	else if (name=="StripedLimb")
		pm = new StripedLimb();
	else if (name=="HotSpotModel")
		pm = new HotSpotModel();
	else if (name=="PlantModel")
		pm = new PlantModel();
	else if (name=="LimbBudModelWithGrowth")
		pm = new LimbBudModelWithGrowth();
	else if (name=="TentacleWithAttractor")
		pm = new TentacleWithAttractor();
	else if (name=="Curling")
		pm = new Curling();
	/*
	else if (name=="ShrinkModel")
		pm = new ShrinkModel();
		*/

	return pm;
}
