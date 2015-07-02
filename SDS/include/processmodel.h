/*
 * ProcessModel.h
 *
 *  Created on: 19/02/2009
 *      Author: ben
 */

#ifndef PROCESSMODEL_H_
#define PROCESSMODEL_H_

#include <ostream>
#include <istream>
#include <set>

#include "organism.h"
#include "bstreamable.h"
#include "random.h"

#include "libconfig.h++"

/**
 * A Process model encapsulates the cell state and behaviour that corresponds to a particular growth model.
 * An instance of CellContents is in every cell. ProcessModel is used as a factory to get the required CellContents.
 *
 * An Organism has a single process model instance which acts on its cells.
 *
 */

/**
 * A morphogen is a concentration value from (0..1).
 */
struct Morphogen
{
	Morphogen();
	void write(std::ostream& f);
	void read(std::istream& is);

	operator double();
	double operator=(double v);

	double value;
	double oldvalue;
};

class CellContents{
public:
	CellContents(int m = 0, int n = 0);
	virtual ~CellContents();

	virtual void write(std::ostream&);
	virtual void read(std::istream&);

	// subclass accessors
	virtual void setMorphogen(int morphogenIndex, double value);
	virtual double getMorphogen(int morphogenIndex) const;
	virtual int numMorphogens() const;

	virtual void setType(int type);
	virtual int getType() const;

	virtual void setVar(int varIndex, double value);
	virtual double getVar(int varIndex) const;
	virtual int numVars() const;

protected:
	int type;

	const int mNumMorphogens;
	const int mNumVars;

	Morphogen* mMorphogens;
	double* mVars;

	friend class ProcessModel;
};

class ProcessModel {
public:
	class NoProcessModelException: public std::exception
	{
	public:
		NoProcessModelException(std::string processmodel) throw();
		virtual ~NoProcessModelException() throw(){}
		virtual const char* what() const throw();

	private:
		std::string message;
	};

public:
	ProcessModel(int numMorphogens = 0, int numVars = 0);
	virtual ~ProcessModel();

	virtual void setOrganism(Organism* o);

	virtual void setNumberOfMorphogens(int nm);

	/**
	 * Use the incoming morphogen concentrations to set up
	 * the cell contents. For example, set cell types based on morphogen concentrations.
	 *
	 * This function is used when exporting data, or converting for example, a painted blender mesh to a initial organism.
	 * NOTE: You shouldn't call this when importing as it may mess up the cell contents.
	 */
	virtual void setup(){}

	/**
	 * Called when the simulator is reset to the beginning.
	 * Returns true if reset is supported, else we can't reset the simulation.
	 */
	virtual bool reset(){return false;}

	virtual void beforePhysicalStep(Organism* o, double dt){}
	virtual void step(Organism* o, double dt){}

	/// skip, cells to skip in morphogen function
	virtual void simulateMorphogenDiffusion(Organism* o, double dt, std::set<Cell*> skip = std::set<Cell*>());
	/// (skip, morph) skip some cells only for a specific morphogen
	virtual void simulateMorphogenDiffusion(Organism* o, double dt, std::set<Cell*> skip, int morph);

	// load or save a new process model from a setting
	static ProcessModel* loadFromSetting(libconfig::Setting& setting) throw(NoProcessModelException);
	virtual void saveToSetting(libconfig::Setting& setting);

	/**
	 * Load a new process model from a string.
	 *
	 * The string must be in libconfig format, e.g.,
	 *
	 * processModel :
	 * {
     *   type = "LimbBudModel";
     *   param1 = .1;
     *   someotherparam = [.3, .4, .5];
	 * };
	 *
	 */
	static ProcessModel* loadFromString(std::string str);

	static ProcessModel* readStatic(std::istream&);
	virtual void writeStatic(std::ostream& binaryOStream);

	static ProcessModel* create(std::string s);

	/// returns a new processinfo instance created from the (binary) inputstream
	virtual CellContents* newCellContents();
	virtual CellContents* readCellContents(std::istream& input);

	virtual std::string name(){return "NoProcessModel";}

	// parameter interface, all parameters are doubles..
	/// returns a list of all parameter names
	virtual std::list<std::string> parameters(){return std::list<std::string>();}

	/// set a parameter
	virtual void set(std::string,double val);
	virtual double get(std::string);

	virtual int numMorphogens() const;
	virtual int numVars() const;

	virtual void setMorphogenDiffusion(int i, double d);
	virtual double getMorphogenDiffusion(int i) const;

	virtual void setMorphogenDecay(int i, double d);
	virtual double getMorphogenDecay(int i) const;

protected:
	/// all subclasses with static parameters must implement this
	virtual void readStaticParams(std::istream&){}
	virtual void loadParamsFromSetting(libconfig::Setting& s);

	Organism* organism();

	int mNumMorphogens;
	int mNumVars;

	double* mDiffusion;
	double* mDecay;

	Organism* mOrganism;
};
typedef ProcessModel NoProcessModel;


#endif /* PROCESSMODEL_H_ */
