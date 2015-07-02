/*
 * organism.cpp
 *
 *  Created on: 09/10/2008
 *      Author: ben
 */

#include "organism.h"

#include "cell.h"
#include "processmodel.h"

#include <boost/foreach.hpp>
#include <ostream>
#include <stdexcept>
#include <cassert>

#ifdef DEBUG_BINARY
	#define DUMP(what) {std::cout << what << std::endl; }
#else
	#define DUMP(what) ;
#endif

#ifdef DEBUG_MEMORY
	#define DUMPM(what) {std::cout << what << std::endl; }
#else
	#define DUMPM(what) ;
#endif

Organism::Organism(Mesh* m)
:mMesh(m),mProcessModel(NULL)
{
	DUMPM("Organism::Organism(m) for " << this);
}

Organism::~Organism()
{
	DUMPM("Organism::~Organism() for " << this);

	if (mMesh)
		delete mMesh;
	if (mProcessModel)
		delete mProcessModel;
	clear();
}

void Organism::clear()
{
	mVertexToCellMap.clear();
	BOOST_FOREACH(Cell* c, mCells)
	{
		delete c;
	}
	mCells.clear();
	mMesh = NULL;
	mProcessModel = NULL;
}

void Organism::setMesh(Mesh* m) 
{
	if (mMesh!=NULL)
		std::cerr << "Warning: mesh has already been set." << std::endl;
		// throw std::runtime_error("Organism::setMesh(m): The mesh has already been set!");
	mMesh = m;
}

void Organism::setProcessModel(ProcessModel* pm) 
{
	if (mProcessModel==pm)
	{
		return;
	}
	else if (pm!=NULL)
	{
		if (mProcessModel) delete mProcessModel;

		mProcessModel = pm;
		mProcessModel->setOrganism(this);
		BOOST_FOREACH(Cell* c, mCells)
		{
			if (c->getCellContents()==NULL)
				c->setCellContents(pm->newCellContents());
		}
	}
	else if (pm==NULL)
	{
		mProcessModel = NULL;
	}
}

void Organism::addCell(Cell* c)
{
	if (mProcessModel!=NULL)
	{
		if (c->getCellContents()==NULL)
			c->setCellContents(mProcessModel->newCellContents());
	}
	mCells.push_back(c);
	mVertexToCellMap[c->v()]=c;
}

void Organism::removeCell(Cell* c)
{
	mCells.remove(c);
	mVertexToCellMap.erase(c->v());

	delete c;
}

void Organism::connectCells(Cell* a, Cell* b)
{
	Edge* e = new Edge(a->v(),b->v(),a->r()+b->r());
	a->v()->addNeighbour(b->v());
	b->v()->addNeighbour(a->v());
	mMesh->addEdge(e);
}

Cell* Organism::getAssociatedCell(Vertex* v) const
{
	// assert(mVertexToCellMap.count(v)==1);
	return mVertexToCellMap.find(v)->second;
}

std::list<Cell*> Organism::getNeighbours(Cell* c) const
{
	std::list<Cell*> neighbours;
	const std::list<Vertex*>& vl = c->v()->neighbours();
	BOOST_FOREACH(Vertex* v, vl)
		neighbours.push_back(Organism::getAssociatedCell(v));
	return neighbours;
}

Edge* Organism::edge(Cell* a, Cell* b) const
{
	return mMesh->getEdge(a->v(),b->v());
}


/**
 * Write the organism out to a binary stream.
 */
void Organism::bWrite(std::ostream& bin)
{
	assert(mMesh!=NULL);
	DUMP("Organism::bWrite");

	// The process of writing also creates a vertex->id table that we use when outputting a cell's information
	std::map<Vertex*,unsigned int>& vm = mMesh->vertexMapWrite();

	#ifdef DEBUG_BINARY
		DUMP("mMesh->vertexMapWrite");
		std::map<Vertex*,unsigned int>::iterator it = vm.begin();
		for(;it!=vm.end();it++)
		{
			DUMP(it->first << " -> " << it->second);
		}
	#endif

    unsigned int numcells = (unsigned int)mCells.size();
	::write(bin,numcells); // (unsigned int)(mCells.size()));
    DUMP("numCells = " << numcells);

	BOOST_FOREACH(Cell* c, mCells)
	{
		::write(bin,vm[c->v()]);
		::write(bin,c->r());
		::write(bin,c->drdt());

		DUMP("Cell " <<  c);
		DUMP("vm[c->v()] = " << vm[c->v()]);
		DUMP("c->r() = " << c->r());
		DUMP("c->drdt() = " << c->drdt());
	}
}

/**
 * Read organism data from a binary stream.
 * PRE: mesh() has already been assigned.
 * False on error.
 */
bool Organism::bRead(std::istream& bin)
{
	DUMP("Organism::bRead");

	// get the id->vertex map
	std::map<unsigned int,Vertex*>& vm = mMesh->vertexMapRead();

	#ifdef DEBUG_BINARY
		DUMP("mMesh->vertexMapRead");
		std::map<unsigned int, Vertex*>::iterator it = vm.begin();
		for(;it!=vm.end();it++)
		{
			DUMP(it->first << " -> " << it->second);
		}
	#endif

	unsigned int numCells = 0;
	::read(bin,numCells);
	DUMP("numCells = " << numCells);

	if (numCells==0) return false;
	for(unsigned int i=0;i<numCells;i++)
	{
		unsigned int vid = 0;
		::read(bin,vid);

		DUMP("vid " << i << " = " << vid);

		if (vm.count(vid)!=1)
		{
			DUMP("vm.count(" << vid << ") = " << vm.count(vid));
			return false;
		}

		Vertex* v = vm.find(vid)->second;

		DUMP("v = " << v);

		double r, drdt;
		::read(bin, r);
		::read(bin, drdt);

		DUMP("r = " << r);
		DUMP("drdt = " << drdt);

		Cell* c = new Cell(v,r);
		c->setDrdt(drdt);
		addCell(c);

		DUMP("new cell = " << c);
	}

	return true;
}

/*****************************/

void Organism::write(std::ostream& file)
{
	// the file format of an organism is as follows:
	// 1. MESH
	// 2. NUMBER OF CELLS
	// 3. FOR EACH CELL:
	// 3a. CELL VERTEX
	// 3b. COMMON CELL DATA
	// 3b. CELL PROCESS INFORMATION

	// 1.
	assert(mMesh!=NULL);
	mMesh->bWriteNew(file);

	// The process of writing also creates a vertex->id table that we use when outputting a cell's information
	std::map<Vertex*,unsigned int>& vm = mMesh->vertexMapWrite();

	// 2.
	::write(file,(unsigned int)(mCells.size()));

	// 3.
	BOOST_FOREACH(Cell* c, mCells)
	{
		// c->v
		::write(file,vm[c->v()]);

		// c->r, c->drdt
		::write(file,c->r());
		::write(file,c->drdt());

		// c->processInformation
		// assume that the process model is known (i.e., written to the stream beforehand)
		// including all its parameters, etc...
		c->getCellContents()->write(file);
	}
}

Organism* Organism::read(std::istream& file, ProcessModel* pm) 
{
	Organism* o = new Organism;
	Mesh* m = new Mesh();

	// NOTE: We first have to read in the number of bytes
	unsigned int numbytesinmesh;
	::read(file,numbytesinmesh);
	m->bReadNew(file);
	o->setMesh(m);
	o->setProcessModel(pm);

	// get the id->vertex map
	std::map<unsigned int,Vertex*>& vm = m->vertexMapRead();

	unsigned int numCells = 0;
	::read(file,numCells);
	if (numCells==0)
		throw std::runtime_error("Organism has no cells!");
	for(unsigned int i=0;i<numCells;i++)
	{
		int vid;
		::read(file,vid);
		assert(vm.count(vid)==1);
		Vertex* v = vm.find(vid)->second;

		double r, drdt;
		::read(file, r);
		::read(file, drdt);

		Cell* c = new Cell(v,r);
		c->setDrdt(drdt);
		c->setCellContents(pm->readCellContents(file));
		o->addCell(c);
	}

	return o;
}
