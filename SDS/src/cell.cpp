/*
 * cell.cpp
 *
 *  Created on: 09/10/2008
 *      Author: ben
 */

#include "cell.h"
#include <algorithm>

Cell::Cell(Vector3d pos, double m)
:mProcessInformation(NULL),mDrdt(0)
{
	mVertex = new Vertex(pos,m);
	mRadius = Math::radiusOfSphereGivenVolume(m);
	mVolume = m;
}

Cell::Cell(Vertex* v, double r)
:mVertex(v),mProcessInformation(NULL),mDrdt(0)
{
	setR(r);
}


Cell::~Cell() {
	if (mProcessInformation) delete mProcessInformation;
}


Vertex* Cell::v()
{
	return mVertex;
}


double Cell::r()
{
	return mRadius;
}


double Cell::m()
{
	return mVertex->m();
}


double Cell::vol()
{
	return mVolume;
}


double Cell::drdt()
{
	return mDrdt;
}


bool Cell::isBoundary()
{
	return mVertex->surface();
}


Vector3d Cell::x() // position
{
	return mVertex->x();
}


void Cell::setR(double r)
{
	mRadius = r;
	// recompute volume
	mVolume = Math::volumeOfSphereGivenRadius(mRadius);
	// then set mass
	setM(mVolume);
}


void Cell::setM(double m)
{
	mVertex->setM(m);
}


void Cell::setDrdt(double drdt)
{
	mDrdt = drdt;
}

void Cell::setCellContents(CellContents* pi)
{
	if (this->mProcessInformation)
		delete this->mProcessInformation;
	this->mProcessInformation = pi;
}

CellContents* Cell::getCellContents()
{
	return this->mProcessInformation;
}
