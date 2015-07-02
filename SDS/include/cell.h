/*
 * cell.h
 *
 *  Created on: 09/10/2008
 *      Author: ben
 */

#ifndef CELL_H_
#define CELL_H_

#include "processmodel.h"
#include "vertex.h"
#include "gmath.h"
#include "vector3.h"

class Cell {
public:
	/** Construct a new cell (and vertex) with a given position and mass */
	Cell(Vector3d pos, double m);
	/** Construct a new cell with an existing vertex. */
	Cell(Vertex* v, double r = 1);
	virtual ~Cell();

	Vertex* v(); // vertex
	double r();
	double m();
	double vol();
	double drdt();

	// extract information about geometry
	bool isBoundary();
	Vector3d x(); // position

	void setR(double);
	void setM(double);
	void setDrdt(double);

	// cell contents
	void setCellContents(CellContents*);
	CellContents* getCellContents();

	/**
	 * @deprecated
	 */
	void setProcessInformation(CellContents* cc){setCellContents(cc);}
	/**
	 * @deprecated
	 */
	CellContents* processInformation(){return getCellContents();}

protected:
	Vertex* mVertex;
	double mRadius;
	CellContents* mProcessInformation;

	// common properties of all process models
	double mDrdt;

	// derived
	double mVolume;
};

#endif /* CELL_H_ */
