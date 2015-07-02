/*
 * segmentio.cpp
 *
 *  Created on: 16/10/2009
 *      Author: ben
 */

#include "segmentio.h"
#include "cell.h"

const std::string MeshSegmentIO::type = "mesh";
const std::string OrganismSegmentIO::type = "organism";
const std::string ProcessModelSegmentIO::type = "processinfo";


MeshSegmentIO::MeshSegmentIO(Mesh* m, std::string v)
:mesh(m)
,version(v)
{}

bool MeshSegmentIO::load(std::istream& bin)
{
	if (version=="19042010")
	{
		mesh->bReadFull(bin);
		mesh->bReadSpringMultipliers(bin);
		mesh->bReadFrozenVerts(bin);
		return true;
	}
	else if (version=="withspringmultipliers")
	{
		mesh->bReadFull(bin);
		mesh->bReadSpringMultipliers(bin);
		return true;
	}
	else if (version=="full")
	{
		mesh->bReadFull(bin);
		return true;
	}
	else
		return false;
}

bool MeshSegmentIO::initialise(libconfig::Setting& s)
{
	if (s.exists("version"))
	{
		std::string v;
		s.lookupValue("version",v);

		if (v=="19042010")
		{
			version = v;
		}
		else if (v=="withspringmultipliers")
		{
			version = v;
		}
		else if (v=="full")
		{
			version = v;
		}
		else
		{
			std::cerr << "MeshSegmentIO: Unknown \"version\".\n";
			return false;
		}
	}

	return true;
}

void MeshSegmentIO::write(std::ostream& bin)
{
	if (version=="19042010")
	{
		mesh->bWriteFull(bin);
		mesh->bWriteSpringMultipliers(bin);
		mesh->bWriteFrozenVerts(bin);
	}
	else if (version=="withspringmultipliers")
	{
		mesh->bWriteFull(bin);
		mesh->bWriteSpringMultipliers(bin);
	}
	else if (version=="full")
	{
		mesh->bWriteFull(bin);
	}
}

void MeshSegmentIO::setSetting(libconfig::Setting& s)
{
	s.add("type",libconfig::Setting::TypeString) = getType();
	s.add("version",libconfig::Setting::TypeString) = version;
}

OrganismSegmentIO::OrganismSegmentIO(Organism* o)
:organism(o)
{

}

// read
bool OrganismSegmentIO::load(std::istream& bin)
{
	return organism->bRead(bin);
}

bool OrganismSegmentIO::initialise(libconfig::Setting& s)
{
	return true;
}

// write
void OrganismSegmentIO::write(std::ostream& bin)
{
	// assume that organism->mesh() has already been written out
	organism->bWrite(bin);
}

void OrganismSegmentIO::setSetting(libconfig::Setting& s)
{
	s.add("type",libconfig::Setting::TypeString) = getType();
}

ProcessModelSegmentIO::ProcessModelSegmentIO(Organism* o)
:organism(o)
,processModel(o->processModel())
{

}

// read
bool ProcessModelSegmentIO::load(std::istream& bin)
{
	// read in the data ...
	unsigned int numcells;
	::read(bin,numcells);
	unsigned int index = 0;

	BOOST_FOREACH(Cell* c, organism->cells())
	{
		CellContents* pi = processModel->readCellContents(bin);
		c->setCellContents(pi);

		if (index > numcells)
		{
			return false;
		}

		index++;
	}

	return true;
}

bool ProcessModelSegmentIO::initialise(libconfig::Setting& s)
{
	return true;
}

// write
void ProcessModelSegmentIO::write(std::ostream& bin)
{
	// for each cell, we write out the frame-varying process model properties
	// we use the same cell order as the organism cells
	unsigned int numCells = organism->cells().size();
	::write(bin,numCells);
	BOOST_FOREACH(Cell* c, organism->cells())
		c->getCellContents()->write(bin);
}

void ProcessModelSegmentIO::setSetting(libconfig::Setting& s)
{
	// write out the process model global spec to setting
	s.add("type",libconfig::Setting::TypeString) = getType();
}
