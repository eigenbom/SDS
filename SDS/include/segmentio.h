/*
 * segmentio.h
 *
 *  Created on: 15/10/2009
 *      Author: ben
 */

#ifndef SEGMENTIO_H_
#define SEGMENTIO_H_

#include <istream>
#include <string>
#include "libconfig.h++"

#include "mesh.h"
#include "organism.h"
#include "processmodel.h"

/**
 * For each segment in frame spec we construct a segmentloader and pass it any parameters.
 */
class SegmentLoader
{
public:
	virtual bool load(std::istream& bin) = 0;
	virtual std::string getType() = 0;

	// set the parameters of this segment loader
	virtual bool initialise(libconfig::Setting& s) = 0;
};

/**
 * Interface for all segment writers.
 */
class SegmentWriter
{
public:
	/**
	 * Write out the segment data to this binary stream.
	 * Don't need to write out sizeInBytes as this is computed afterwards.
	 */
	virtual void write(std::ostream& bin) = 0;
	virtual std::string getType() = 0;

	// set the libconfig::Setting for this segment
	// assumes that s is empty
	virtual void setSetting(libconfig::Setting& s) = 0;
};

/**
 * Mesh versions:
 * 	"full": initial version, stores all the data every frame
 *  "withspringmultipliers": stores the spring multipliers appended to the mesh data
 */
class MeshSegmentIO: public SegmentLoader, public SegmentWriter
{
public:
	static const std::string type;

	Mesh* mesh;
	std::string version;

	MeshSegmentIO(Mesh* mesh, std::string version="19042010");
	virtual ~MeshSegmentIO(){}

	// version="bare" implies that mesh is in compact form
	// else mesh is in more complete form
	std::string getType(){return MeshSegmentIO::type;}

	// read
	virtual bool load(std::istream& bin);
	virtual bool initialise(libconfig::Setting& s);

	// write
	virtual void write(std::ostream& bin);
	virtual void setSetting(libconfig::Setting& s);
};

/**
 * Handles organism loading and saving. In both cases we assume that a mesh has already been loaded/saved.
 */
class OrganismSegmentIO: public SegmentLoader, public SegmentWriter
{
public:
	static const std::string type;

	Organism* organism;

	OrganismSegmentIO(Organism* o);
	virtual ~OrganismSegmentIO(){}

	std::string getType(){return OrganismSegmentIO::type;}

	// read
	virtual bool load(std::istream& bin);
	virtual bool initialise(libconfig::Setting& s);

	// write
	virtual void write(std::ostream& bin);
	virtual void setSetting(libconfig::Setting& s);
};

/**
 * Handles process model loading and saving.
 *
 * Loading:
 * 	we assume that an organism has been loaded.
 *
 * Saving:
 *  assume that organism->processModel is not NULL
 * 	we output the cell data in the order the cells are stored in organism.
 */
class ProcessModelSegmentIO: public SegmentLoader, public SegmentWriter
{
public:
	static const std::string type;

	Organism* organism;
	ProcessModel* processModel;
	ProcessModelSegmentIO(Organism* o);
	virtual ~ProcessModelSegmentIO(){}

	std::string getType(){return ProcessModelSegmentIO::type;}

	// read
	virtual bool load(std::istream& bin);
	virtual bool initialise(libconfig::Setting& s);

	// write
	virtual void write(std::ostream& bin);
	virtual void setSetting(libconfig::Setting& s);
};

/*
// requires a Mesh*
class TexCoordSegmentLoader
{
public:
	static const std::string type = "texcoords";

	void load(istream&, Mesh*); // specifies UV data for the vertices of a mesh
};
*/

#endif /* SEGMENTLOADER_H_ */
