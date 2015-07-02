/*
 * organism.h
 *
 *  Created on: 09/10/2008
 *      Author: ben
 */

#ifndef ORGANISM_H_
#define ORGANISM_H_

#include "mesh.h"
#include "vertex.h"

#include <map>
#include <list>
#include <ostream>
#include <istream>

/** Organism: an instance of an SDS form.
 *
 * An organism uses a Mesh internally to specify its geometry.
 * It contains a set of Cells with a 1-1 correspondance between Vertex and Cell.
 *
 * Ownership: Once an organism is created with a mesh, it deletes it upon destruction.
 *
 * Cells and ProcessModels.
 * The specification of ProcessInfo of a cell is deferred until a process model is set via setProcessModel.
 * e.g.,
 * <code>
 * Organism o(m);
 * o->addCell(new Cell(..));
 * o->setProcessModel(Foo);
 * - setProcessModel will initialise all cells added so far with the right processinfo
 * o->addCell(new Cell(..));
 * - a processmodel has been set so this cell will have its processinfo set
 * - an _exception_ to this is if the cell's processinfo is already set
 * </code>
 *
 * TODO: Organism interface should encapsulate mesh functionality,
 *       e.g., org->addCell() should also add a vertex to the mesh underneath.
 */

class Cell;
class ProcessModel;
class Organism {
public:
	/**
	 * Construction functions
	 * NOTE: Constructing an organism with a mesh will not create cells.
	 * This has to be done manually, or by one of the organism factories.
	 */
	Organism(Mesh* m = NULL);
	~Organism();

	/**
	 * Clear the contents of this organism.
	 */
	void clear();

	void setMesh(Mesh* m) ;
	void setProcessModel(ProcessModel* pm) ;
	ProcessModel* processModel(){return mProcessModel;}

	/// add/remove cell
	void addCell(Cell* c);
	/// remove (and delete) a cell

	// TODO: should removeCell remove vertices too?
	void removeCell(Cell* c);

	/**
	 * organism manipulation
	 * connect two cells together, adds an edge of the appropriate length, etc
	 * also updates neighbourlinks etc...
	 */
	void connectCells(Cell* a, Cell* b);



	/// return the unique cell attached to v
	// PRE: Cell must have been added via addCell and already attached to a vertex
	Cell* getAssociatedCell(Vertex* v) const;

	/// TODO: protected?
	std::list<Cell*> getNeighbours(Cell* c) const;

	/// TODO: hide?
	// the edge between a and b
	// or NULL if none exist
	// XXX: O(|E|) at the moment
	Edge* edge(Cell* a, Cell* b) const;
	const std::list<Cell*>& cells() {return mCells;}
	Mesh* mesh() {return mMesh;}

	/**
	 * Write the organism out to a binary stream.
	 */
	void bWrite(std::ostream& bin);

	/**
	 * Read organism data from a binary stream.
	 * PRE: mesh() has already been assigned.
	 * False on error.
	 */
	bool bRead(std::istream& bin);

	/// Stream to a binary file
	// PRE file has its binary bit set
	// @deprecated
	void write(std::ostream& file);
	/// Create a new organism from a binary input stream
	// throws a runtime_error if the file is corrupt
	// requires:
	//   The process model for this simulation.
	// @deprecated
	static Organism* read(std::istream& file, ProcessModel* pm) ;

protected:
	std::list<Cell*> mCells;
	std::map<Vertex*,Cell*> mVertexToCellMap;

	Mesh* mMesh;
	ProcessModel* mProcessModel;
};

#endif /* ORGANISM_H_ */
