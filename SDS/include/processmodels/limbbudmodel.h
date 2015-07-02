#ifndef LIMBBUDMODEL_H
#define LIMBBUDMODEL_H

#include "processmodel.h"
#include "organism.h"
#include "bstreamable.h"
#include "random.h"

#include <ostream>
#include "libconfig.h++"

/* The limb bud development rules */
class LimbBudModel: public ProcessModel
{
public:
	LimbBudModel();
	virtual void setup();

	void step(Organism* o, double dt); // simulate diffusion and activate rules
	void updateDiffusionParams();

	// virtual CellContents* newCellContents(){return new CellContents(3);}
	virtual std::string name(){return "LimbBudModel";}

	virtual void writeStatic(std::ostream& o);
	virtual void saveToSetting(libconfig::Setting& setting);

	double diffusionFgf8, diffusionFgf10, decayFgf8, decayFgf10,
			aerS, aerR, pzSE, pzSM, pzR, rE, rM, drdtE, drdtM;

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

protected:
	virtual void readStaticParams(std::istream&);
	virtual void loadParamsFromSetting(libconfig::Setting& s);
};



class LimbBudModelWithGrowth: public LimbBudModel
{
public:
	LimbBudModelWithGrowth();

	void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "LimbBudModelWithGrowth";}

	virtual void saveToSetting(libconfig::Setting& setting);

protected:
	virtual void loadParamsFromSetting(libconfig::Setting& s);

	double mPassiveCellGrowthRate;
};

class TentacleWithAttractor: public LimbBudModelWithGrowth
{
public:
	TentacleWithAttractor();

	virtual void beforePhysicalStep(Organism* o, double dt);
	void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "TentacleWithAttractor";}

	virtual void saveToSetting(libconfig::Setting& setting);

protected:
	virtual void loadParamsFromSetting(libconfig::Setting& s);

	double mAttractionStrength;
	Vector3d mAttractor;
};

/********************/

class LimbGrowth: public ProcessModel
{
public:
	LimbGrowth(int nm = 1, int nv = 0);
	virtual void setup();

	virtual ~LimbGrowth(){}
	virtual void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "LimbGrowth";}

	virtual void writeStatic(std::ostream& o);
	virtual void readStaticParams(std::istream& i);

protected:

	double rE, rM, drdtE, drdtM; // etc.
	//double mPassiveCellGrowthRate;
};

/********************/

class LimbGrowthWithFreeze: public ProcessModel
{
public:
	LimbGrowthWithFreeze();
	virtual void setup();

	virtual ~LimbGrowthWithFreeze(){}
	virtual void step(Organism* o, double dt);

	// parameter interface
	virtual std::list<std::string> parameters();
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual std::string name(){return "LimbGrowthWithFreeze";}

	virtual void writeStatic(std::ostream& o);
	virtual void readStaticParams(std::istream& i);

protected:

	double rE, rM, drdtE, drdtM; // etc.
};

#endif
