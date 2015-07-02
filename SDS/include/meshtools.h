/*
 * meshtools.h
 *
 *  Created on: 08/10/2009
 *      Author: ben
 */

#ifndef MESHTOOLS_H_
#define MESHTOOLS_H_

#include "mesh.h"
#include <exception>

/**
 * Mesh Tools provides loaders, test meshes, readers, etc.
 */
class MeshTools
{
public:
	class ParseException: public std::exception
	{
		std::string file;
		int line;
		std::string error;
		std::string formatted;

	public:
		ParseException(std::string file, int line, std::string error) throw();
		virtual ~ParseException() throw(){}
		virtual const char* what() const throw();

		std::string getFile(){return file;}
		int getLine(){return line;}
		std::string getError(){return error;}
	};

public:
	enum PhysicalPropertiesMode{StaticMesh, CellMesh};
	/**
	 * Load a new mesh from a set of files.
	 *
	 * Tetgen generated files are accepted.
	 * prefix.node, prefix.ele, and prefix.neigh should all exist.
	 *
	 * @param prefix Prefix of the files to load.
	 * @param mode Mode to use to compute physical parameters. StaticMesh uses the current state of
	 *  the mesh to computer rest lengths etc. CellMesh computes appropriate cell sizes and then infers
	 *  rest lengths etc based on the cells.
	 *
	 * @return The loaded mesh, or NULL on error.
	 */
	static Mesh* Load(std::string prefix, PhysicalPropertiesMode mode = StaticMesh);


	/**
	 * Low level.
	 *
	 * Reads in mesh data from a binary input stream i.	 *
	 * Allocates and returns the mesh data chunk (NULL = fail).
	 * Returns the topo changed bit.
	 * Returns the size in number of bytes.
	 *
	 * PRE: MeshSeg has been read already
	 */
	static char* ReadMeshChunk(std::istream& i, bool& topochanged, unsigned int& numBytes);


private:
	/** Assumes current mesh geometry is static and estimates cell mass, etc to preserve this.
	 * Perfect for normal simulation of a deformable object.
	 */
	static void CalculatePhysicalParametersStatic(Mesh* m);

	// supports different file types
	enum FileType{Tetgen,DotTet,Unknown};
	static FileType DetermineFileType(std::string name); // determines file-type based on name [and possibly from contents]

	/** LoadTetgenMesh (name)
	 * PRE: name.node, name.mesh, etc exist
	 * loads the data into a new mesh object and returns it
	 * returns NULL if any errors occur
	 */
	static Mesh* LoadTetgenMesh(std::string name);

	/** LoadTetMesh: broken */
	static Mesh* LoadTetMesh(std::string name);
};

#endif /* MESHTOOLS_H_ */
