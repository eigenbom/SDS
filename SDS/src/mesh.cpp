#include "mesh.h"
#include "world.h"
#include "edge.h"

#include <map>
#include <sstream>
#include <cfloat>
#include <iostream>
#include <cassert>

#include <boost/foreach.hpp>

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

Mesh::Mesh(bool tc)
:mTopoChanged(tc)
 {
	DUMPM("Mesh::Mesh() for " << this);
 }

Mesh::~Mesh()
{
	DUMPM("Mesh::~Mesh() for " << this);
	clear();
}

void Mesh::clear()
{
	// delete everything
	BOOST_FOREACH(Vertex* v, mVertices)
		delete v;
	BOOST_FOREACH(Edge* e, mEdges)
		delete e;
	BOOST_FOREACH(Tetra* t, mTetras)
		delete t;

	//XXX: Remove this, and replace with mOuterFaces
	BOOST_FOREACH(Face* f, mFaces)
		delete f;
	/*
	BOOST_FOREACH(Face* f, mOuterFaces)
		delete f;
	*/

	mVertices.clear();
	mEdges.clear();
	mTetras.clear();
	mFaces.clear();
	mOuterFaces.clear();
	mTopoChanged = false;
	mAABB = AABB::ZERO;

	mVertexMapWrite.clear();
	mFaceMapWrite.clear();
	mTetraMapWrite.clear();
	mVertexMapRead.clear();
}


void Mesh::scale(double sx, double sy, double sz)
{
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		double x = v->x().x(), y = v->x().y(), z = v->x().z();
		v->resetX(Vector3d(x*sx,y*sy,z*sz));
	}

	BOOST_FOREACH(Tetra* t, mTetras)
		t->updateAABB();

	// also update the bounding box of the mesh
	updateAABB();
}

std::list<Tetra*> Mesh::getNeighbours(Vertex* v) const
{
	std::list<Tetra*> n;
	BOOST_FOREACH(Tetra* t, mTetras)
	{
		if (t->contains(v)) n.push_back(t);
	}
	return n;
}

bool Mesh::getAdjacentFaces(Edge* e, Face* &f1, Face* &f2) const
{
	bool foundf1 = false, foundf2 = false;
	Vertex *v1 = e->v(0), *v2 = e->v(1);
	BOOST_FOREACH(Face* f, mFaces)
	{
		if (f->contains(v1) and f->contains(v2))
		{
			if (!foundf1)
			{
				f1 = f;
				foundf1 = true;
			}
			else
			{
				f2 = f;
				foundf2 = true;
				break;
			}
		}
	}

	return foundf1 and foundf2;
}

bool Mesh::getAdjacentFaces(Vertex* v1, Vertex* v2, Face* &f1, Face* &f2) const
{
	bool foundf1 = false, foundf2 = false;
	BOOST_FOREACH(Face* f, mFaces)
	{
		if (f->contains(v1) and f->contains(v2))
		{
			if (!foundf1)
			{
				f1 = f;
				foundf1 = true;
			}
			else
			{
				f2 = f;
				foundf2 = true;
				break;
			}
		}
	}

	return foundf1 and foundf2;
}

/// change content of mesh
void Mesh::addVertex(Vertex* v)
{
	mVertices.push_back(v);
	topoChange();
}

void Mesh::addEdge(Edge* e)
{
	mEdges.push_back(e);
	topoChange();
}

/*
void Mesh::addFace(Face* f)
{
	mFaces.push_back(f);
	topoChange();
}
*/

void Mesh::addOuterFace(Face* f)
{
	f->setOuter(true);
	mOuterFaces.push_back(f);
	mFaces.push_back(f);
	topoChange();
}

void Mesh::setAsOuterFace(Face* f)
{
	f->setOuter(true);
	mOuterFaces.push_back(f);
	topoChange();
}

void Mesh::addTetra(Tetra* t)
{
	mTetras.push_back(t);
	topoChange();
}

/// warning: does not delete t, only removes it from this mesh
void Mesh::removeTetra(Tetra* t)
{
	mTetras.remove(t);
	topoChange();
}

void Mesh::removeFace(Face* f)
{
	mFaces.remove(f);
	mOuterFaces.remove(f);
	topoChange();
}

void Mesh::removeEdge(Edge* e)
{
	mEdges.remove(e);
	topoChange();
}

void Mesh::removeVertex(Vertex* v)
{
	mVertices.remove(v);
	topoChange();
}

void Mesh::deleteFace(Face* f)
{
	removeFace(f);
	removeFaceNeighbourLinks(f);
	delete f;
}


void Mesh::deleteEdge(Edge* e)
{
	removeEdge(e);
	e->v(0)->removeNeighbour(e->v(1));
	e->v(1)->removeNeighbour(e->v(0));
	delete e;
}


double Mesh::volume() const
{
	double totalvolume = 0;
	BOOST_FOREACH(Tetra* t, mTetras)
		totalvolume += t->volume();
	return totalvolume;
}

double Mesh::mass() const
{
	double totalmass = 0;
	BOOST_FOREACH(Vertex* v, mVertices)
		totalmass += v->m();
	return totalmass;
}

double Mesh::energy() const
{
	double energy = 0;

	// sum all energy in system
	//double y = mWorld->bounds().min().y();
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		// kinetic
		energy += v->kineticEnergy();

		// potential (not included)
		//energy += v->potentialEnergy(v->x().y() - y);
	}

	BOOST_FOREACH(Edge* e, mEdges)
		energy += e->energy();

	BOOST_FOREACH(Tetra* t, mTetras)
		energy += t->energy();

	return energy;
}

Edge* Mesh::getEdge(Vertex* v1, Vertex* v2) const
{
	BOOST_FOREACH(Edge* e, mEdges)
	{
		if (e->v(0)==v1 and (v2==NULL or e->v(1)==v2)) return e;
		else if (e->v(1)==v1 and (v2==NULL or e->v(0)==v2)) return e;
	}
	return NULL;
}

std::list<Edge*> Mesh::getAllEdges(Vertex* v1) const
{
	std::list<Edge*> edges;
	BOOST_FOREACH(Edge* e, mEdges)
	{
		if (e->v(0)==v1 or e->v(1)==v1) edges.push_back(e);
	}
	return edges;
}

Face* Mesh::getFace(Vertex* v1, Vertex* v2, Vertex* v3) const
{
	BOOST_FOREACH(Face* f, mFaces)
	{
		if (f->contains(v1) and
				(v2==NULL or
						(f->contains(v2) and
								(v3==NULL or f->contains(v3))
						)
				)
			)
			return f;
	}
	return NULL;
}

Tetra* Mesh::getTetra(Vertex* v1, Vertex* v2, Vertex* v3, Vertex* v4) const
{
	BOOST_FOREACH(Tetra* t, mTetras)
	{
		if (t->contains(v1) and
				(v2==NULL or
						(t->contains(v2) and
								(v3==NULL or
										(t->contains(v3) and
												(v4==NULL or
														t->contains(v4)
												)
										)
								)
						)
				)
			)
			return t;
	}
	return NULL;
}

Face* Mesh::getSurfaceFace(Vertex* va, Vertex* vb, Vertex* vc) const
{
	Face* f = getFace(va,vb,vc);
	if (f and f->outer())
		return f;
	else
		return NULL;
}

Face* Mesh::getFace(Tetra* t, int i) const
{
	int j,k,l;
	t->oppositeFaceIndices(i,j,k,l);
	Vertex *vj = &t->v(j), *vk = &t->v(k), *vl = &t->v(l);
	return getFace(vj,vk,vl);
}

bool Mesh::doesTetraContainEdge(Tetra* t, Edge* e)
{
	return t->contains(e->v(0)) and t->contains(e->v(1));
}

void Mesh::removeFaceNeighbourLinks(Face* f)
{
	f->v(0).removeFaceNeighbour(f);
	f->v(1).removeFaceNeighbour(f);
	f->v(2).removeFaceNeighbour(f);
}

void Mesh::addFaceNeighbourLinks(Face* f)
{
	f->v(0).addFaceNeighbour(f);
	f->v(1).addFaceNeighbour(f);
	f->v(2).addFaceNeighbour(f);
}

#define CHECKINV(inv) {if (not (inv)) {std::cerr << "mesh not sane: " << __FILE__ << ":" << __LINE__ << "\n"; return false;}}

#define CHECKINV2(inv,msg) {if (not (inv)) {std::cerr << "mesh not sane: " << __FILE__ << ":" << __LINE__ << " [" << msg << "]\n"; return false;}}

bool Mesh::isSane()
{
	// check that all components are non-null
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		CHECKINV(v);
		BOOST_FOREACH(Vertex* vn, v->neighbours())
		{
			CHECKINV(vn);
			CHECKINV(getEdge(v,vn));
		}

		if (v->surface())
		{
			BOOST_FOREACH(Face* f, v->surfaceFaces())
			{
				CHECKINV(f);
				CHECKINV(f->contains(v));
				CHECKINV(f->outer());
				CHECKINV(std::find(mFaces.begin(),mFaces.end(),f)!=mFaces.end());
				CHECKINV(std::find(mOuterFaces.begin(),mOuterFaces.end(),f)!=mOuterFaces.end());
			}
		}
	}

	BOOST_FOREACH(Edge* e, mEdges)
	{
		CHECKINV(e);
		CHECKINV(e->v(0) && e->v(1));

		const std::list<Vertex*>& v0n = e->v(0)->neighbours();
		const std::list<Vertex*>& v1n = e->v(1)->neighbours();

		CHECKINV(std::find(v0n.begin(),v0n.end(),e->v(1))!=v0n.end());
		CHECKINV(std::find(v1n.begin(),v1n.end(),e->v(0))!=v1n.end());
	}

	BOOST_FOREACH(Face* f, mFaces)
	{
		CHECKINV(f);
		CHECKINV(f->outer());
		CHECKINV(std::find(mOuterFaces.begin(),mOuterFaces.end(),f)!=mOuterFaces.end());

		Vertex *v0 = &f->v(0), *v1 = &f->v(1), *v2 = &f->v(2);

		CHECKINV(v0);
		CHECKINV(v1);
		CHECKINV(v2);

		CHECKINV(getEdge(v0,v1));
		CHECKINV(getEdge(v0,v2));
		CHECKINV(getEdge(v1,v2));

		const std::list<Face*>& sf0 = v0->surfaceFaces(), &sf1 = v1->surfaceFaces(), &sf2 = v2->surfaceFaces();
		CHECKINV(std::find(sf0.begin(),sf0.end(),f)!=sf0.end());
		CHECKINV(std::find(sf1.begin(),sf1.end(),f)!=sf1.end());
		CHECKINV(std::find(sf2.begin(),sf2.end(),f)!=sf2.end());

	}

	BOOST_FOREACH(Face* f, mOuterFaces)
	{
		CHECKINV(f);
		CHECKINV(f->outer());
		CHECKINV(std::find(mFaces.begin(),mFaces.end(),f)!=mFaces.end());

		Vertex *v0 = &f->v(0), *v1 = &f->v(1), *v2 = &f->v(2);

		CHECKINV(v0);
		CHECKINV(v1);
		CHECKINV(v2);

		CHECKINV(getEdge(v0,v1));
		CHECKINV(getEdge(v0,v2));
		CHECKINV(getEdge(v1,v2));

		const std::list<Face*>& sf0 = v0->surfaceFaces(), &sf1 = v1->surfaceFaces(), &sf2 = v2->surfaceFaces();
		CHECKINV(std::find(sf0.begin(),sf0.end(),f)!=sf0.end());
		CHECKINV(std::find(sf1.begin(),sf1.end(),f)!=sf1.end());
		CHECKINV(std::find(sf2.begin(),sf2.end(),f)!=sf2.end());
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		CHECKINV(t);

		Vertex* v[4] = {&t->v(0), &t->v(1), &t->v(2), &t->v(3)};
		CHECKINV2(getEdge(v[0],v[1]),t);
		CHECKINV2(getEdge(v[0],v[2]),t);
		CHECKINV2(getEdge(v[0],v[3]),t);
		CHECKINV2(getEdge(v[1],v[2]),t);
		CHECKINV2(getEdge(v[1],v[3]),t);
		CHECKINV2(getEdge(v[2],v[3]),t);

		// CHECKINV that size is positive, as all tetrahedra are oriented the same way
		CHECKINV2(t->volume() > -0.0001, "volume " << t << " = " << t->volume() << ", vertex positions: " << v[0]->x() << ", " << v[1]->x() << ", " << v[2]->x() << ", " << v[3]->x() << ", ");

		for(int i=0;i<4;i++)
		{
			Tetra* tn = t->neighbour(i);
			if (tn)
			{
				// then tn and t must share all the other verts
				for(int j=1;j<4;j++)
				{
					int k = (i + j)%4;
					CHECKINV2(tn->contains(v[k]),tn << t << v[i] << v[k]);
				}

				// moreover the v[i] neighbour of tn must be t
				bool found = false;
				for(int k=0;k<4;k++)
				{
					if (t==tn->neighbour(k))
						found = true;
				}
				CHECKINV2(found,"tn->t link not found. tn:" << tn << ", t:" << t);

				// and furthermore, FACE(v[i+1],v[i+2],v[i+3]) shouldn't exist
				CHECKINV(NULL==getFace(v[(i+1)%4],v[(i+2)%4],v[(i+3)%4]));
			}
			else
			{
				Face* f = getFace(v[(i+1)%4],v[(i+2)%4],v[(i+3)%4]);
				CHECKINV(f);
				CHECKINV(f->outer());
			}
		}
	}

	return true;
}






























typedef unsigned int uint;

void writeVec(std::ostream& o, Vector3d& v)
{
	write(o,v[0]);
	write(o,v[1]);
	write(o,v[2]);
}

void readVec(std::istream& o, Vector3d& v)
{
	read(o,v[0]);
	read(o,v[1]);
	read(o,v[2]);
}

void writeAABB(std::ostream& o, AABB& aabb)
{
	write(o,aabb[0]);
	write(o,aabb[1]);
	write(o,aabb[2]);
	write(o,aabb[3]);
	write(o,aabb[4]);
	write(o,aabb[5]);
}

void readAABB(std::istream& o, AABB& aabb)
{
	read(o,aabb[0]);
	read(o,aabb[1]);
	read(o,aabb[2]);
	read(o,aabb[3]);
	read(o,aabb[4]);
	read(o,aabb[5]);
	aabb.validate();
}

/*
void Mesh::bWrite(std::ostream& ostr)
{
	// TOPOLOGY CHANGE WRITE

	// XXX: Add up bytes and write count (first!)
	uint szuint = sizeof(uint);
	uint szdbl = sizeof(double);
	uint szaabb = 6*(sizeof mAABB[0]);
	uint szvec = szdbl * 3;
	uint szbool = sizeof(bool);

	uint numbytes =
		5 * szuint + 		// counts
		szaabb + // aabb
		szdbl +			// volume
		mVertices.size()*(3 * szvec + szdbl + szbool); //vertex
	// add vertex top data

	uint vertextopsize = 0;

	BOOST_FOREACH(Vertex* v, mVertices)
	{
		vertextopsize += szbool;

		if (v->surface())
		{
			vertextopsize += (szuint + szuint*v->surfaceFaces().size());
		}

		vertextopsize += (szuint + szuint*v->neighbours().size());
	}

	numbytes += vertextopsize +
		(szuint*2 + szdbl)*mEdges.size() + // edges
		(szuint*3 + szdbl + szbool)*mFaces.size() +
		(szuint*8 + szbool + szdbl + szbool)*mTetras.size();

	write(ostr,numbytes);
	// store num vertices, num edges, num faces, num tetras,
	// also .. num outerfaces, bounding box, and volume

	// num vertices
	uint numverts = mVertices.size(),
			numedges = mEdges.size(),
			numfaces = mFaces.size(),
			numtetras = mTetras.size(),
			numouterfaces = mOuterFaces.size();

	write(ostr,numverts);
	write(ostr,numedges);
	write(ostr,numfaces);
	write(ostr,numtetras);
	write(ostr,numouterfaces);

	writeAABB(ostr,mAABB);

	// SETUP PTR->ID MAPPING
	std::map<Vertex*, unsigned int> vertexMap;
	std::map<Face*, unsigned int> faceMap;
	std::map<Tetra*, unsigned int> tetraMap;

	unsigned int counter = 0;
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		vertexMap[v] = counter;
		counter++;
	}

	counter = 0;
	BOOST_FOREACH(Face* f, mFaces)
	{
		faceMap[f] = counter;
		counter++;
	}

	counter = 0;
	BOOST_FOREACH(Tetra* t, mTetras)
	{
		tetraMap[t] = counter;
		counter++;
	}

	// OUTPUT VERTICES, EDGES, FACES, TETRAS, OUTERFACES
	// using the PTR->ID maps for references

	// vertex continuous data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		writeVec(ostr,v->mX);
		writeVec(ostr,v->mOldX);
		writeVec(ostr,v->mF);
		write(ostr,v->mMass);
		write(ostr,v->mTag);
	}

	// vertex topo data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		// write surface
		if (v->surface()) write(ostr,(char)1);
		else write(ostr, (char)0);

		if (v->surface())
		{
			// write num face neighbours
			write(ostr,v->surfaceFaces().size());
			// write face neighbours
			BOOST_FOREACH(Face* f, v->surfaceFaces())
				write(ostr, faceMap[f]);
		}

		// write num neighbours
		write(ostr,v->neighbours().size());
		// write neighbours
		BOOST_FOREACH(Vertex* v, v->neighbours())
			write(ostr, vertexMap[v]);
	}

	// edge data (topo and cont)
	BOOST_FOREACH(Edge* e, mEdges)
	{
		write(ostr,vertexMap[e->v(0)]);
		write(ostr,vertexMap[e->v(1)]);
		write(ostr,e->rest());
	}

	// face data (all)
	BOOST_FOREACH(Face* f, mFaces)
	{
		write(ostr,vertexMap[&f->v(0)]);
		write(ostr,vertexMap[&f->v(1)]);
		write(ostr,vertexMap[&f->v(2)]);
		write(ostr,f->rest());
		write(ostr,f->outer());
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		write(ostr,vertexMap[&t->v(0)]);
		write(ostr,vertexMap[&t->v(1)]);
		write(ostr,vertexMap[&t->v(2)]);
		write(ostr,vertexMap[&t->v(3)]);

		write(ostr,tetraMap[&t->n(0)]);
		write(ostr,tetraMap[&t->n(1)]);
		write(ostr,tetraMap[&t->n(2)]);
		write(ostr,tetraMap[&t->n(3)]);

		write(ostr,t->isOuter());
		write(ostr,t->rest());
		write(ostr,t->intersected());
	}
}

void Mesh::bRead(std::istream& istr)
{
	// TOPOLOGY CHANGE READ

	// delete ALL current data

	BOOST_FOREACH(Vertex* v, mVertices)
		delete v;
	BOOST_FOREACH(Edge* e, mEdges)
		delete e;
	BOOST_FOREACH(Face* f, mFaces)
		delete f;
	BOOST_FOREACH(Tetra* t, mTetras)
		delete t;

	mVertices.clear();
	mEdges.clear();
	mFaces.clear();
	mTetras.clear();
	mOuterFaces.clear();
	mAABB = AABB::ZERO;

	// read in new data

	uint numverts, numedges, numfaces, numtetras, numouterfaces;

	read(istr,numverts);
	read(istr,numedges);
	read(istr,numfaces);
	read(istr,numtetras);
	read(istr,numouterfaces);

	readAABB(istr,mAABB);

	std::map<uint,Vertex*> vertexMap;
	std::map<uint,Face*> faceMap;
	std::map<uint,Tetra*> tetraMap;

	// make the appropriate empty containers for these....
	for(uint i=0;i<numverts;++i)
	{
		Vertex* v = new Vertex();
		mVertices.push_back(v);
		vertexMap[i] = v;
	}

	for(uint i=0;i<numfaces;++i)
	{
		Face* f = new Face();
		mFaces.push_back(f);
		faceMap[i] = f;
	}

	for(uint i=0;i<numtetras;++i)
	{
		Tetra* t = new Tetra();
		mTetras.push_back(t);
		tetraMap[i] = t;
	}

	// read in vertex continuous data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		readVec(istr,v->mX);
		readVec(istr,v->mOldX);
		readVec(istr,v->mF);
		read(istr,v->mMass);
		read(istr,v->mTag);

		v->computeR();
	}

	// read in vertex topo data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		char c;
		read(istr,c);
		bool surface = (c==1)?true:false;

		v->setSurface(surface);
		if (surface)
		{
			// read in surface neighbour data
			uint numfaceneighbours;
			read(istr,numfaceneighbours);
			for(uint i=0;i<numfaceneighbours;++i)
			{
				uint id;
				read(istr,id);
				v->mFaceNeighbours.push_back(faceMap[id]);
			}
		}

		uint numneighbours;
		read(istr,numneighbours);
		for(uint i=0;i<numneighbours;++i)
		{
			uint id;
			read(istr,id);
			v->mNeighbours.push_back(vertexMap[id]);
		}
	}

	for(uint i=0;i<numedges;++i)
	{
		uint id1, id2;
		read(istr,id1);
		read(istr,id2);
		Vertex *v1 = vertexMap[id1], *v2 = vertexMap[id2];
		double rest;
		read(istr,rest);
		Edge* e = new Edge(v1,v2,rest);
		mEdges.push_back(e);
	}

	BOOST_FOREACH(Face* f, mFaces)
	{
		uint id1, id2, id3;
		read(istr,id1);
		read(istr,id2);
		read(istr,id3);
		Vertex *v1 = vertexMap[id1],
					 *v2 = vertexMap[id2],
					 *v3 = vertexMap[id3];

		f->mV[0] = v1;
		f->mV[1] = v2;
		f->mV[2] = v3;

		double rest;
		read(istr,rest);
		f->rest(rest);

		bool outer;
		read(istr,outer);
		f->setOuter(outer);

		if (outer)
			mOuterFaces.push_back(f);
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		// ...
		uint v1, v2, v3, v4, t1, t2, t3, t4;
		read(istr,v1);
		read(istr,v2);
		read(istr,v3);
		read(istr,v4);
		read(istr,t1);
		read(istr,t2);
		read(istr,t3);
		read(istr,t4);

		t->mV[0] = vertexMap[v1];
		t->mV[1] = vertexMap[v2];
		t->mV[2] = vertexMap[v3];
		t->mV[3] = vertexMap[v4];

		t->mN[0] = tetraMap[t1];
		t->mN[1] = tetraMap[t2];
		t->mN[2] = tetraMap[t3];
		t->mN[3] = tetraMap[t4];

		read(istr,t->mOuter);
		read(istr,t->mRestVolume);
		read(istr,t->mIntersected);

		t->updateAABB();
	}
}
*/

void Mesh::bWriteFull(std::ostream& ostr)
{
	DUMP("Mesh::bWriteFull");

	// num vertices
	uint numverts = mVertices.size(),
			numedges = mEdges.size(),
			//numfaces = mFaces.size(),
			numtetras = mTetras.size(),
			numouterfaces = mOuterFaces.size();

	write(ostr,numverts);
	write(ostr,numedges);
	write(ostr,numtetras);
	write(ostr,numouterfaces);
	writeAABB(ostr,mAABB);

	DUMP("numverts = " << numverts);
	DUMP("numedges = " << numedges);
	DUMP("numtetras = " << numtetras);
	DUMP("numouterfaces = " << numouterfaces);
	DUMP("mAABB = " << mAABB);

	// SETUP PTR->ID MAPPING
	mVertexMapWrite.clear();
	mFaceMapWrite.clear();
	mTetraMapWrite.clear();

	std::map<Vertex*, unsigned int>& vertexMap = mVertexMapWrite;
	std::map<Face*, unsigned int>& faceMap = mFaceMapWrite;
	std::map<Tetra*, unsigned int>& tetraMap = mTetraMapWrite;

	unsigned int counter = 0;
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		vertexMap[v] = counter;
		DUMP("vertexMap[" << v << "] = " << counter);

		counter++;
	}

	counter = 0;
	BOOST_FOREACH(Face* f, mFaces)
	{
		faceMap[f] = counter;
		DUMP("faceMap[" << f << "] = " << counter);

		counter++;
	}

	counter = 0;
	BOOST_FOREACH(Tetra* t, mTetras)
	{
		tetraMap[t] = counter;
		DUMP("tetraMap[" << t << "] = " << counter);

		counter++;
	}

	// OUTPUT VERTICES, EDGES, FACES, TETRAS, OUTERFACES
	// using the PTR->ID maps for references

	// vertex continuous data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		writeVec(ostr,v->mX);
		writeVec(ostr,v->mOldX);
		writeVec(ostr,v->mF);
		write(ostr,v->mMass);
		write(ostr,v->mTag);

		DUMP("v->mX=" << v->mX << "");
		DUMP("v->mOldX=" << v->mOldX << "");
		DUMP("v->mF=" << v->mF << "");
		DUMP("v->mMass=" << v->mMass << "");
		DUMP("v->mTag=" << v->mTag << "");
	}

	// vertex topo data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		// write surface
		if (v->surface()){
			write(ostr,(char)1);
		}
		else write(ostr, (char)0);

		DUMP("v(" << v << ")->surface()==" << v->surface());

		if (v->surface())
		{
			// write num face neighbours
			write(ostr,(unsigned int)(v->surfaceFaces().size()));
			DUMP("v->surfaceFaces().size() = " << (unsigned int)(v->surfaceFaces().size()));

			// write face neighbours
			BOOST_FOREACH(Face* f, v->surfaceFaces())
			{
				DUMP("write(faceMap[" << f << "] = " << faceMap[f]);
				write(ostr, faceMap[f]);
			}
		}

		// write num neighbours
		write(ostr,(unsigned int)(v->neighbours().size()));
		DUMP("(unsigned int)(v->neighbours().size())) = " << (unsigned int)(v->neighbours().size()));

		// write neighbours
		BOOST_FOREACH(Vertex* v, v->neighbours())
		{
			write(ostr, vertexMap[v]);
			DUMP("vertexMap[" << v << "] = " << vertexMap[v]);
		}
	}

	// edge data (topo and cont)
	BOOST_FOREACH(Edge* e, mEdges)
	{
		write(ostr,vertexMap[e->v(0)]);
		write(ostr,vertexMap[e->v(1)]);
		write(ostr,e->rest());
		write(ostr,e->springCoefficient());

		DUMP("vertexMap[e->v(0)] = " << vertexMap[e->v(0)]);
		DUMP("vertexMap[e->v(1)] = " << vertexMap[e->v(1)]);
		DUMP("e->rest() = " << e->rest());
		DUMP("e->springCoefficient() = " << e->springCoefficient());
	}

	// face data (all)
	BOOST_FOREACH(Face* f, mFaces)
	{
		write(ostr,vertexMap[&f->v(0)]);
		write(ostr,vertexMap[&f->v(1)]);
		write(ostr,vertexMap[&f->v(2)]);
		write(ostr,f->rest());

		DUMP("write(ostr,vertexMap[&f->v(0)]) = " << vertexMap[&f->v(0)] << "write(ostr,vertexMap[&f->v(1)]) = " << vertexMap[&f->v(1)] << "write(ostr,vertexMap[&f->v(2)]) = " << vertexMap[&f->v(2)] << "write(ostr,f->rest()) = " << f->rest());

		// assert(f->outer());
		// write(ostr,f->outer());
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		DUMP("t = " << t);

		for(int i=0;i<4;i++)
		{
			write(ostr,vertexMap[t->pv(i)]);
			DUMP("write(ostr,vertexMap[t->pv(i)]) = " << vertexMap[t->pv(i)]);
		}

		for(int i=0;i<4;i++)
		{
			if (t->neighbour(i)==NULL)
			{
				write(ostr,-1);
				DUMP("neighbour(" << i << ") = -1");
			}
			else
			{
				write(ostr,tetraMap[t->neighbour(i)]);
				DUMP("neighbour(" << i << ") = " << tetraMap[t->neighbour(i)]);
			}
		}

		write(ostr,t->springCoefficient());
		write(ostr,t->isOuter());
		write(ostr,t->rest());
		write(ostr,t->intersected());

		DUMP("t->springCoefficient() = " << t->springCoefficient());
		DUMP("t->isOuter() = " << t->isOuter());
		DUMP("t->rest() = " << t->rest());
		DUMP("t->intersected() = " << t->intersected());
	}
}

#define PRINT(what) {if (noisy) {std::cerr << __FILE__ << "(" << __LINE__ << "):" << what << std::endl; }}

void Mesh::bReadFull(std::istream& istr, bool noisy)
{
	DUMP("bReadFull");

	// TOPOLOGY CHANGE READ
	PRINT("Starting to read")

	// delete ALL current data
	BOOST_FOREACH(Vertex* v, mVertices)
		delete v;
	BOOST_FOREACH(Edge* e, mEdges)
		delete e;
	BOOST_FOREACH(Face* f, mFaces)
		delete f;
	BOOST_FOREACH(Tetra* t, mTetras)
		delete t;

	mVertices.clear();
	mEdges.clear();
	mFaces.clear();
	mTetras.clear();
	mOuterFaces.clear();
	mAABB = AABB::ZERO;

	// read in new data

	uint numverts = 0,
		numedges = 0,
		numtetras = 0,
		numouterfaces = 0;

	read(istr,numverts);
	read(istr,numedges);
	read(istr,numtetras);
	read(istr,numouterfaces);
	readAABB(istr,mAABB);

	PRINT("numverts" << numverts)
	PRINT("numedges" << numedges)
	PRINT("numtetras" << numtetras)
	PRINT("numouterfaces" << numouterfaces)

	DUMP("numverts = " << numverts);
	DUMP("numedges = " << numedges);
	DUMP("numtetras = " << numtetras);
	DUMP("numouterfaces = " << numouterfaces);
	DUMP("mAABB = " << mAABB);

	mVertexMapRead.clear();
	std::map<uint,Vertex*>& vertexMap = mVertexMapRead;
	std::map<uint,Face*> faceMap;
	std::map<uint,Tetra*> tetraMap;

	// make the appropriate empty containers for these....
	for(uint i=0;i<numverts;++i)
	{
		Vertex* v = new Vertex();
		mVertices.push_back(v);
		vertexMap[i] = v;

		DUMP("vertexMap[" << i << "] = " << v);
	}

	for(uint i=0;i<numouterfaces;++i)
	{
		Face* f = new Face();
		addOuterFace(f);
		faceMap[i] = f;

		DUMP("faceMap[" << i << "] = " << f);
	}


	for(uint i=0;i<numtetras;++i)
	{
		Tetra* t = new Tetra();
		mTetras.push_back(t);
		tetraMap[i] = t;

		DUMP("tetraMap[" << i << "] = " << t);
	}

	// read in vertex continuous data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		readVec(istr,v->mX);
		readVec(istr,v->mOldX);
		readVec(istr,v->mF);
		read(istr,v->mMass);
		read(istr,v->mTag);
		v->computeR();

		DUMP("v = " << v);
		DUMP("mX = " << v->mX);
		DUMP("mOldX = " << v->mOldX);
		DUMP("mF = " << v->mF);
		DUMP("mMass = " << v->mMass);
		DUMP("mTag = " << v->mTag);
		DUMP("computed radius = " << v->r());
	}

	// read in vertex topo data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		char c;
		read(istr,c);
		bool surface = (c==1)?true:false;

		DUMP("v = " << v);
		DUMP("surface = " << surface);

		v->setSurface(surface);
		if (surface)
		{
			// read in surface neighbour data
			uint numfaceneighbours;
			read(istr,numfaceneighbours);
			DUMP("numfaceneighbours = " << numfaceneighbours);

			for(uint i=0;i<numfaceneighbours;++i)
			{
				uint id;
				read(istr,id);
				v->addFaceNeighbour(faceMap[id]);

				DUMP("faceMap[" << id << "]" << faceMap[id]);
			}
		}

		uint numneighbours;
		read(istr,numneighbours);

		DUMP("numneighbours = " << numneighbours);
		for(uint i=0;i<numneighbours;++i)
		{
			uint id;
			read(istr,id);
			v->mNeighbours.push_back(vertexMap[id]);

			DUMP("vertexMap[" << id << "] = " << vertexMap[id]);
		}
	}

	for(uint i=0;i<numedges;++i)
	{
		uint id1, id2;
		read(istr,id1);
		read(istr,id2);
		Vertex *v1 = vertexMap[id1], *v2 = vertexMap[id2];
		double rest;
		read(istr,rest);
		Edge* e = new Edge(v1,v2,rest);
		mEdges.push_back(e);

		DUMP("new Edge(" << v1 << "," << v2 << "," << rest << "), vertex ids = " << id1 << "," << id2);

		read(istr,e->mSpringCoefficient);

		DUMP("springCoefficient = " << e->mSpringCoefficient);
	}

	BOOST_FOREACH(Face* f, mFaces)
	{
		uint id1, id2, id3;
		read(istr,id1);
		read(istr,id2);
		read(istr,id3);
		Vertex *v1 = vertexMap[id1],
					 *v2 = vertexMap[id2],
					 *v3 = vertexMap[id3];

		f->mV[0] = v1;
		f->mV[1] = v2;
		f->mV[2] = v3;

		double rest;
		read(istr,rest);
		f->rest(rest);

		DUMP("f->mV = [" << v1 << "," << v2 << "," << v3 << "] ids = [" << id1 << "," << id2 << "," << id3 << "], rest = " << rest);
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		DUMP("t = " << t);

		// ...
		uint v[4];
		int n[4];
		for(int i=0;i<4;i++)
		{
			read(istr,v[i]);
			t->mV[i] = vertexMap[v[i]];

			DUMP("vertexMap[v[" << i << "]] = " << vertexMap[v[i]]);
		}
		for(int i=0;i<4;i++)
		{
			read(istr,n[i]);
			if (n[i]==-1)
			{
				t->mN[i] = NULL;
				DUMP("t->mN[" << i << "] = NULL");
			}
			else
			{
				t->mN[i] = tetraMap[n[i]];
				DUMP("t->mN[" << i << "] = " << tetraMap[n[i]] << " (tetraMap[n[i]])");
			}
		}

		read(istr,t->mSpringCoefficient);
		read(istr,t->mOuter);
		read(istr,t->mRestVolume);
		read(istr,t->mIntersected);

		DUMP("springcoeff, out, restvol, intersected = " <<
				t->mSpringCoefficient << "," <<
				t->mOuter << "," <<
				t->mRestVolume << "," <<
				t->mIntersected);

		t->updateAABB();
	}
}

void Mesh::bWriteSpringMultipliers(std::ostream& ostr)
{
    DUMP("bwriteSpringMults");

	BOOST_FOREACH(Edge* e, mEdges)
    {
		write(ostr,e->restMultiplier());
        DUMP("e,mult = " << e << " " << e->restMultiplier());
    }
	BOOST_FOREACH(Tetra* t, mTetras)
    {
		write(ostr,t->restMultiplier());
        DUMP("t,mult = " << t << " " << t->restMultiplier());
    }   
}

void Mesh::bReadSpringMultipliers(std::istream& istr)
{
    DUMP("bReadSpringMults");
	BOOST_FOREACH(Edge* e, mEdges)
	{
		double multiplier;
		read(istr,multiplier);

        DUMP("e,mult = " << e << " " << multiplier);
		double r = e->rest();
		e->setRestMultiplier(multiplier);
		e->setRest(r/multiplier);
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		double multiplier;
		read(istr,multiplier);

        DUMP("t,mult = " << t << " " << multiplier);

		double r = t->rest();
		t->setRestMultiplier(multiplier);
		t->setRest(r/multiplier);
	}
}

void Mesh::bWriteFrozenVerts(std::ostream& ostr)
{
	DUMP("bWriteFrozenVerts");

	BOOST_FOREACH(Vertex* v, mVertices)
	{
		write(ostr,v->isFrozen());
		DUMP("v,frozen = " << v << " " << v->isFrozen());
	}
}

void Mesh::bReadFrozenVerts(std::istream& istr)
{
	DUMP("bReadFrozenVerts");

	BOOST_FOREACH(Vertex* v, mVertices)
	{
		bool frozen;
		read(istr,frozen);

		DUMP("v,frozen = " << v << " " << frozen);
		v->setFrozen(frozen);
	}
}

void Mesh::bWriteNew(std::ostream& ostr)
{
	// TOPOLOGY CHANGE WRITE

	// XXX: Add up bytes and write count (first!)
	uint szuint = sizeof(uint);
	uint szdbl = sizeof(double);
	uint szaabb = 6*(sizeof mAABB[0]);
	uint szvec = szdbl * 3;
	uint szbool = sizeof(bool);

	uint numbytes =
		5 * szuint + 		// counts
		szaabb + // aabb
		szdbl +			// volume
		mVertices.size()*(3 * szvec + szdbl + szbool); //vertex
	// add vertex top data

	uint vertextopsize = 0;

	BOOST_FOREACH(Vertex* v, mVertices)
	{
		vertextopsize += szbool;

		if (v->surface())
		{
			vertextopsize += (szuint + szuint*v->surfaceFaces().size());
		}

		vertextopsize += (szuint + szuint*v->neighbours().size());
	}

	numbytes += vertextopsize +
		(szuint*2 + szdbl)*mEdges.size() + // edges
		(szuint*3 + szdbl + szbool)*mFaces.size() +
		(szuint*8 + szbool + szdbl + szbool)*mTetras.size();

	write(ostr,numbytes);
	// store num vertices, num edges, num faces, num tetras,
	// also .. num outerfaces, bounding box, and volume

	// num vertices
	uint numverts = mVertices.size(),
			numedges = mEdges.size(),
			numfaces = mFaces.size(),
			numtetras = mTetras.size(),
			numouterfaces = mOuterFaces.size();

	write(ostr,numverts);
	write(ostr,numedges);
	write(ostr,numfaces);
	write(ostr,numtetras);
	write(ostr,numouterfaces);

	writeAABB(ostr,mAABB);

	// SETUP PTR->ID MAPPING
	mVertexMapWrite.clear();
	mFaceMapWrite.clear();
	mTetraMapWrite.clear();

	std::map<Vertex*, unsigned int>& vertexMap = mVertexMapWrite;
	std::map<Face*, unsigned int>& faceMap = mFaceMapWrite;
	std::map<Tetra*, unsigned int>& tetraMap = mTetraMapWrite;

	unsigned int counter = 0;
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		vertexMap[v] = counter;
		counter++;
	}

	counter = 0;
	BOOST_FOREACH(Face* f, mFaces)
	{
		faceMap[f] = counter;
		counter++;
	}

	counter = 0;
	BOOST_FOREACH(Tetra* t, mTetras)
	{
		tetraMap[t] = counter;
		counter++;
	}

	// OUTPUT VERTICES, EDGES, FACES, TETRAS, OUTERFACES
	// using the PTR->ID maps for references

	// vertex continuous data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		writeVec(ostr,v->mX);
		writeVec(ostr,v->mOldX);
		writeVec(ostr,v->mF);
		write(ostr,v->mMass);
		write(ostr,v->mTag);
	}

	// vertex topo data
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		// write surface
		if (v->surface()) write(ostr,(char)1);
		else write(ostr, (char)0);

		if (v->surface())
		{
			// write num face neighbours
			write(ostr,v->surfaceFaces().size());
			// write face neighbours
			BOOST_FOREACH(Face* f, v->surfaceFaces())
				write(ostr, faceMap[f]);
		}

		// write num neighbours
		write(ostr,v->neighbours().size());
		// write neighbours
		BOOST_FOREACH(Vertex* v, v->neighbours())
			write(ostr, vertexMap[v]);
	}

	// edge data (topo and cont)
	BOOST_FOREACH(Edge* e, mEdges)
	{
		write(ostr,vertexMap[e->v(0)]);
		write(ostr,vertexMap[e->v(1)]);
		write(ostr,e->rest());
		write(ostr,e->springCoefficient());
	}

	// face data (all)
	BOOST_FOREACH(Face* f, mFaces)
	{
		write(ostr,vertexMap[&f->v(0)]);
		write(ostr,vertexMap[&f->v(1)]);
		write(ostr,vertexMap[&f->v(2)]);
		write(ostr,f->rest());
		write(ostr,f->outer());
	}

	BOOST_FOREACH(Tetra* t, mTetras)
	{
		write(ostr,vertexMap[&t->v(0)]);
		write(ostr,vertexMap[&t->v(1)]);
		write(ostr,vertexMap[&t->v(2)]);
		write(ostr,vertexMap[&t->v(3)]);

		write(ostr,tetraMap[&t->n(0)]);
		write(ostr,tetraMap[&t->n(1)]);
		write(ostr,tetraMap[&t->n(2)]);
		write(ostr,tetraMap[&t->n(3)]);

		write(ostr,t->springCoefficient());
		write(ostr,t->isOuter());
		write(ostr,t->rest());
		write(ostr,t->intersected());
	}
}








void Mesh::move(Vector3d dx)
{
	BOOST_FOREACH(Vertex* v, mVertices)
	{
		v->addX(dx);
		v->zeroV();
	}

	BOOST_FOREACH(Tetra* t, mTetras)
		t->updateAABB();

	// also update the bounding box of the mesh
	mAABB.move(dx);
}

Mesh* Mesh::copy()
{
	Mesh* m = new Mesh;

	m->mAABB = mAABB;
	m->mTopoChanged = mTopoChanged;

	// just use the streaming functionality already implementd
	// not as fast as it could be, but its okay

	std::ostringstream str(std::ostringstream::out);
	this->bWriteFull(str);
	std::istringstream data(str.str());
	//uint numbytes;
	//read(data,numbytes); // eat size info
	m->bReadFull(data);
	//m->setWorld(mWorld);
	return m;
}

void Mesh::updateAABB()
{
	double min[3] = {DBL_MAX,DBL_MAX,DBL_MAX}, max[3] = {-DBL_MAX,-DBL_MAX,-DBL_MAX};

	BOOST_FOREACH(Vertex* v, mVertices)
	{
		Vector3d p = v->x();
		double x = p.x(), y = p.y(), z = p.z();

		if (x < min[0]) min[0] = x;
		if (x > max[0]) max[0] = x;

		if (y < min[1]) min[1] = y;
		if (y > max[1]) max[1] = y;

		if (z < min[2]) min[2] = z;
		if (z > max[2]) max[2] = z;
	}

	mAABB[0] = min[0];
	mAABB[1] = min[1];
	mAABB[2] = min[2];
	mAABB[3] = max[0];
	mAABB[4] = max[1];
	mAABB[5] = max[2];
	mAABB.validate();
}
