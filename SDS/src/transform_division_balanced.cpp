/*
 * transform_division_balanced.cpp
 *
 *  Created on: 17/09/2009
 *      Author: ben
 */

#include "transform.h"

#include "hmath.h"
#include "vector3.h"
#include "gmath.h"

#include "vertex.h"

#include <cfloat>
#include <cassert>
#include <set>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

boost::tuple<Cell*,Cell*> Transform::DivideBalanced(Cell* c, Vector3d dir, Organism* o)
{
	reset();
	called("DivideBalanced");
	Transform::properties.add("cell",c);

	dir.normaliseInPlace();
	Mesh* m = o->mesh();

	if (c->isBoundary())
		{
			Transform::properties.add("boundary division");

			// get the neighbour cell closest to direction dir
			Cell* nc = NULL;
			double ncang = DBL_MAX;
			BOOST_FOREACH(Cell* n, o->getNeighbours(c))
			{
				if (n->isBoundary())
				{
					double ang = Math::acos(dot((n->x() - c->x()).normalise(),dir));
					if (ang<ncang)
					{
						ncang = ang;
						nc = n;
					}
				}
			}

			assert(nc);

			if (ncang < sDivideInternalAngleThreshold) // divide an edge
			{
				return Transform::DivideSurfaceEdge(c,o->edge(c,nc),o);
			}
			else // divide a face
			{
				// Find the face in the direction given
				Face* f = Transform::getFaceInDirection(c,dir,o);
				if (f!=NULL)
				{
					return Transform::DivideTetra(c,f,o);
				}

				// else, must be an internal divide, so do it!
				// TODO: What about DivideAway?
			}
		}
	else // is not boundary
	{
		Transform::properties.add("internal division");

		// If the direction lies along an edge, then we split the edge.
		// Otherwise we place the new cell inside the tetrahedra that lies in the direction dir

		Cell* nc = NULL;
		double ncang = DBL_MAX;
		BOOST_FOREACH(Cell* n, o->getNeighbours(c))
		{
			double ang = Math::acos(dot((n->x() - c->x()).normalise(),dir));
			if (ang<ncang)
			{
				ncang = ang;
				nc = n;
			}
		}

		if (ncang < sDivideInternalAngleThreshold)
			return Transform::DivideInternalEdge(c,o->edge(c,nc),o);
		else /*** The balanced division algorithm happens! ***/
		{
			Transform::properties.add("balanced tetrahedralisation");

			// Create F = {t}

			Tetra* t = Transform::getTetraInDirection(c,dir,o);
			if (t==NULL)
			{
				// this shouldn't happen, but if it does, then just choose any tetra
				t = *m->getNeighbours(c->v()).begin();
				LOG("c.x: " << c->x() << " dir: " << dir);
			}

			Vector3d pc = t->center();

			// compute new mass and radius of daughter cells
			double oldMass = c->m();
			double newMass = oldMass/2;
			double newRadius = Math::radiusOfSphereGivenVolume(newMass);

			c->setR(newRadius);
			c->setM(newMass);

			Vertex* vdash = new Vertex(pc,newMass);
			Cell* cdash = new Cell(vdash, newRadius);
			m->addVertex(vdash);
			o->addCell(cdash);

			// THE ALGORITHM STARTS HERE!
			// Just use a brute-force retetrahedralisation of the hull around c.
			// This is obviously a lot slower, but may provide us with a nicer
			// tetrahedralisation, and hence nicer mesh in the long run.
			// plus its easier to implement than what

			// Enumerate all the points, and all the outer hull faces
			std::vector<Vector3d> points;
			std::vector<Vertex*> verts;
			std::vector<Cell*> cells;
			std::vector<boost::tuple<int,int,int> > trianglesOnHull;

			// c and pc
			points.push_back(c->x());
			points.push_back(pc);
			verts.push_back(c->v());
			verts.push_back(vdash);
			cells.push_back(c);
			cells.push_back(cdash);

			// enumerate the hull and retetrahedralise it
			std::map<Vertex*, int> vIndexMap;
			std::list<Tetra*> incidentTets = m->getNeighbours(c->v());

			vIndexMap[c->v()] = 0;
			vIndexMap[vdash] = 1;

			// keep a record of the neighbours of each hull face
			std::vector<Tetra*> hullN;
			std::vector<int> hullI;
			std::map<long,int> hullFaceIndexMap; // maps from unique faceid to index into hullN
			std::set<long> hullEdgeSet; // hash(vertexpairs) if the vertex pair is on the hull

			double totalMass = 0;
			totalMass += c->v()->m();
			int pIndex = verts.size();
			BOOST_FOREACH(Tetra* it, incidentTets)
			{
				// for each incident tetrahedra
				// store the hull face and points
				// NOTE: because c is internal, there is a one-one correspondance between
				// each hull face and each incident tetra

				int f1,f2,f3;
				it->oppositeFaceIndices(it->vIndex(c->v()),f1,f2,f3);
				Vertex *v1 = it->pv(f1), *v2 = it->pv(f2), *v3 = it->pv(f3);

				if (vIndexMap.count(v1)==0)
				{
					vIndexMap[v1] = pIndex++;
					points.push_back(v1->x());
					totalMass += v1->m();
					verts.push_back(v1);
					cells.push_back(o->getAssociatedCell(v1));
				}
				if (vIndexMap.count(v2)==0)
				{
					vIndexMap[v2] = pIndex++;
					points.push_back(v2->x());
					totalMass += v2->m();
					verts.push_back(v2);
					cells.push_back(o->getAssociatedCell(v2));
				}
				if (vIndexMap.count(v3)==0)
				{
					vIndexMap[v3] = pIndex++;
					points.push_back(v3->x());
					totalMass += v3->m();
					verts.push_back(v3);
					cells.push_back(o->getAssociatedCell(v3));
				}

				trianglesOnHull.push_back(boost::make_tuple(vIndexMap[v1],
															vIndexMap[v2],
															vIndexMap[v3]));

				Transform::properties.add("hull", boost::make_tuple(v1,v2,v3));
				LOG ("hull face " << vIndexMap[v1] <<
						" " << vIndexMap[v2] <<
						" " << vIndexMap[v3] << "\n");

				// for each tet on the other side of the face, store its neighbour information
				Tetra* tn = it->neighbour(c->v());
				hullN.push_back(tn);
				if (tn!=NULL)
				{
					int index = tn->getNeighbourIndex(it);
					assert(index!=-1);
					hullI.push_back(index);
				}
				else
					hullI.push_back(-1);
			}

			typedef boost::tuple<int,int,int,int> INT4;
			typedef boost::tuple<int,int,int> INT3;
			typedef boost::tuple<int,int> INT2;

			int i = 0;
			BOOST_FOREACH(INT3& face, trianglesOnHull)
			{
				int f0 = face.get<0>(), f1 = face.get<1>(), f2 = face.get<2>();

				long id = MAPFACEID_I(f0,f1,f2,vIndexMap);
				assert (hullFaceIndexMap.count(id)==0);
				hullFaceIndexMap[id] = i++;

				hullEdgeSet.insert(MAPEDGEID_I(f0,f1,vIndexMap));
				hullEdgeSet.insert(MAPEDGEID_I(f1,f2,vIndexMap));
				hullEdgeSet.insert(MAPEDGEID_I(f0,f2,vIndexMap));

				LOG("edge " << f0 << " " << f1 << " mapped to " << MAPEDGEID_I(f0,f1,vIndexMap) << "\n");
				LOG("edge " << f1 << " " << f2 << " mapped to " << MAPEDGEID_I(f1,f2,vIndexMap) << "\n");
				LOG("edge " << f2 << " " << f0 << " mapped to " << MAPEDGEID_I(f2,f0,vIndexMap) << "\n");

				/*
				std::ostringstream oss;
				oss << "face [" << face.get<0>() << "," << face.get<1>() << "," << face.get<2>() << "]" << " maps to " << id;
				Transform::properties.add(oss.str());
				*/
			}

			// find a valid tetrahedralisation of the new hull and two points
			std::vector<INT4> newTetrahedra;
			std::vector<INT4> newTetrahedraNeighbours;
			std::vector<INT2> newEdges;

			int pSizeBefore = points.size();
			double averageMass = totalMass / (pSizeBefore-1);
			try
			{
				Math::tetrahedralise(points,
								 trianglesOnHull,
								 newTetrahedra,
								 newTetrahedraNeighbours,
								 newEdges);
			}
			catch(int i)
			{
				return error("Can't tetrahedralise point set.");
			}

			// remove all internal edges, which happend to be all edges connected to c
			BOOST_FOREACH(Edge* e, m->getAllEdges(c->v()))
				m->deleteEdge(e);

			// remove all internal tetrahedra (but keep outer faces)
			BOOST_FOREACH(Tetra* x, incidentTets)
			{
				m->removeTetra(x);
				delete x;
			}

			LOG("pointsBefore: " << pSizeBefore << ", new verts: " << points.size() << ", new tets: " << newTetrahedra.size() << ", new edges: " << newEdges.size() << "\n");
			Transform::properties.addStringAndValue("number of (new) vertices: ", points.size()-pSizeBefore);
			Transform::properties.addStringAndValue("number of tets: ", newTetrahedra.size());
			Transform::properties.addStringAndValue("number of edges: ", newEdges.size());
			assert(newTetrahedra.size()==newTetrahedraNeighbours.size());

			// integrate the new tetrahedra and possibly new vertices into
			// the empty space within the hull

			// add new points
			std::vector<Vertex*> newPoints;
			for(unsigned int i=pSizeBefore;i<points.size();i++)
			{
				Vertex* v = new Vertex(points[i][0],points[i][1],points[i][2],averageMass);
				Cell* c = new Cell(v,Math::radiusOfSphereGivenVolume(averageMass));
				m->addVertex(v);
				o->addCell(c);

				newPoints.push_back(v);
				verts.push_back(v);
				cells.push_back(c);

				Transform::properties.add("new cell", c);
			}

			// add new vertices, edges, and tetras to mesh
			std::vector<Tetra*> newTets;
			BOOST_FOREACH(INT4& tet, newTetrahedra)
			{
				// add this tetra to mesh
				// switch vertex indices 2 and 3
				int t0 = tet.get<0>(), t1 = tet.get<1>(), t2 = tet.get<3>(), t3 = tet.get<2>();

				Vertex *vs[] = {
						verts[t0],
						verts[t1],
						verts[t2],
						verts[t3]};
				Cell *cs[] = {
						o->getAssociatedCell(vs[0]),
						o->getAssociatedCell(vs[1]),
						o->getAssociatedCell(vs[2]),
						o->getAssociatedCell(vs[3])};

				Tetra* newtet = new Tetra(vs[0],vs[1],vs[2],vs[3],Transform::tetrahedraRestVolume(cs[0],cs[1],cs[2],cs[3]));
				o->mesh()->addTetra(newtet);
				newTets.push_back(newtet);

				Transform::properties.add("new tet", newtet);
				LOG("new tetra @" << newtet << ". " << t0 << " " << t1 << " " << t2 << " " << t3 << "\n");
			}

			/****************** TEST QUIT ********************/
			// return testQuit(); /******************************/
			/*************************************************/

			// fix up all neighbourhood information

			// first connect all the tets together
			// we know that each face of each new tet is either on the hull, or
			// is shared by another tet.
			i = 0;
			BOOST_FOREACH(INT4& nb, newTetrahedraNeighbours)
			{
				Tetra* t = newTets[i];
				Vertex *v0 = &t->v(0), *v1 = &t->v(1), *v2 = &t->v(2), *v3 = &t->v(3);

				// nbi is the index of the newtet that is the i'th neighbour.
				int nbi = nb.get<0>();
				if (nbi==-1)
				{
					// then face nbi lies on the hull...
					// so, retrieve neighbour information and set up neighbour links
					// also, note that i < newTets.size()

					// need to figure out which face is on the hull
					long faceId = MAPFACEID(v1,v2,v3,vIndexMap);
					int faceIndex = hullFaceIndexMap[faceId];

					// set up neighbour links
					Tetra* tn = hullN[faceIndex]; // tetra on other side of face
					int tni = hullI[faceIndex]; // index of face for tet tn

					t->setNeighbour(0,tn);
					if (tn!=NULL)
						tn->setNeighbour(tni,t);

					// sanity check
					if (!checkTetraNeighbour(t,0)){
						Transform::properties.add("Error in neighbour loop.");
						Transform::properties.add("t",t);
						Transform::properties.add("tn",tn);
						return testQuit();
					}
				}
				else // face is internal to the complex...
					t->setNeighbour(0,newTets[nbi]);

				/**** repeat logic for each element in nb ****/
				nbi = nb.get<1>();
				if (nbi==-1)
				{
					long faceId = MAPFACEID(v0,v2,v3,vIndexMap);
					int faceIndex = hullFaceIndexMap[faceId];
					Tetra* tn = hullN[faceIndex];
					int tni = hullI[faceIndex];
					t->setNeighbour(1,tn);
					if (tn!=NULL)
						tn->setNeighbour(tni,t);

					// sanity check
					if (!checkTetraNeighbour(t,1)){
						Transform::properties.add("Error in neighbour loop.");
						Transform::properties.add("t",t);
						Transform::properties.add("tn",tn);
						return testQuit();
					}
				}
				else t->setNeighbour(1,newTets[nbi]);

				// switch indices 2 and 3!
				nbi = nb.get<2>();
				if (nbi==-1)
				{
					long faceId = MAPFACEID(v0,v1,v2,vIndexMap);
					int faceIndex = hullFaceIndexMap[faceId];
					Tetra* tn = hullN[faceIndex];
					int tni = hullI[faceIndex];
					t->setNeighbour(3,tn);
					if (tn!=NULL)
						tn->setNeighbour(tni,t);

					// sanity check
					if (!checkTetraNeighbour(t,3)){
						Transform::properties.add("Error in neighbour loop.");
						Transform::properties.add("t",t);
						Transform::properties.add("tn",tn);
						return testQuit();
					}
				}
				else t->setNeighbour(3, newTets[nbi]);

				nbi = nb.get<3>();
				if (nbi==-1)
				{
					long faceId = MAPFACEID(v0,v1,v3,vIndexMap);
					int faceIndex = hullFaceIndexMap[faceId];
					Tetra* tn = hullN[faceIndex];
					int tni = hullI[faceIndex];
					t->setNeighbour(2,tn);
					if (tn!=NULL)
						tn->setNeighbour(tni,t);

					// sanity check
					if (!checkTetraNeighbour(t,2)){
						Transform::properties.add("Error in neighbour loop.");
						Transform::properties.add("t",t);
						Transform::properties.add("tn",tn);
						return testQuit();
					}
				}
				else t->setNeighbour(2,newTets[nbi]);
				/**** end repeated logic ****/

				i++;
			}

			// then add all the new edges and don't forget the cell neighbourhood data
			i = 0;
			BOOST_FOREACH(INT2& edge, newEdges)
			{
				// ignore any edge lying on the hull
				// XXX some edges may contain two hull verts but aren't on the hull!
				int e0 = edge.get<0>();
				int e1 = edge.get<1>();

				LOG("edge " << i << "/" << newEdges.size() << ": " << e0 << "->" << e1 << "\n");

				if (
						/* edge contains an internal vertex */
						(e0==0 or e0==1 or e0>=pSizeBefore or
						 e1==0 or e1==1 or e1>=pSizeBefore)
					)
				{
					LOG("connecting\n");
					// add edge e0--e1 to mesh
					// and update vertex/cell neighbours
					o->connectCells(cells[e0],cells[e1]);
				}
				else
				{
					long id = MAPEDGEID_I(e0,e1,vIndexMap);
					LOG("id = " << id << "\n");
					/* or edge is not on the hull (note e0,e1 > 1 < pSizeBefore) */
					if (hullEdgeSet.count(id)==0)
					{
						LOG("connecting\n");
						// add edge e0--e1 to mesh
						// and update vertex/cell neighbours
						o->connectCells(cells[e0],cells[e1]);
					}
				}

				i++;
			}

			finaliseProperties();
			return boost::tuple<Cell*,Cell*>(c,cdash);
		}
	}
}

long Transform::MAPFACEID(Vertex* v1, Vertex* v2, Vertex* v3, std::map<Vertex*,int>& index)
{
	int iv1 = index[v1], iv2 = index[v2], iv3 = index[v3];
	return MAPFACEID_I(iv1,iv2,iv3,index);
}

long Transform::MAPFACEID_I(int iv1, int iv2, int iv3, std::map<Vertex*,int>& index)
{
	long numVerts = index.size();
	long ivmin = std::min(iv1,std::min(iv2,iv3));
	long ivmax = std::max(iv1,std::max(iv2,iv3));
	long ivmid = (iv1+iv2+iv3) - ivmin - ivmax;
	return ((ivmin*numVerts) + ivmid)*numVerts + ivmax;
}

long Transform::MAPEDGEID_I(int iv1, int iv2, std::map<Vertex*,int>& index)
{
	long numVerts = index.size();
	return std::min(iv1,iv2)*numVerts + std::max(iv1,iv2);
}
