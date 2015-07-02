#include "transform.h"

#include "tetra.h"
#include "organism.h"
#include "meshops.h"

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <list>
#include <set>
#include <iostream>

typedef boost::tuple<int,int,int,int> INT4;
typedef boost::tuple<int,int,int> INT3;
typedef boost::tuple<int,int> INT2;
typedef boost::tuple<Vertex*,Vertex*,Vertex*> VERT3;

// bool Transform::Complexify(std::list<Tetra*> T, Organism* o){return true;}

struct _PLC
{
	std::list<VERT3> faces;
	std::list<Cell*> cells;

	std::set<Edge*> allEdgesAttachedToAnyCellOnPLC;
	std::list<Tetra*> allTetraAttachedToAnyCellOnPLC;

	bool invalid;

	_PLC():invalid(false){}
};

struct HullFaceInfo
{
	Tetra* tn;
	int tni;
	VERT3 verts;
};

struct TetResults
{

	// new_vertices: all the new vertices
	// std::list<Cell*> cells; // the cells from PLC
	// std::vector<Tetra*> tetrahedra; // all the new Tetra*, with neighbours set
	// surface_tetrahedra: all the Tetra* with at least one face on the hull
	// surface_tetrahedra_which_are_attached_to_new_or_isolated_cells
	// edges: all the edges in the tetrahedralisation
	std::list<Vertex*> new_vertices;
	std::list<Cell*> cells;
	std::vector<Tetra*> tetrahedra;
	std::list<Tetra*> surface_tetrahedra;
	std::list<Tetra*> surface_tetrahedra_which_are_attached_to_new_or_isolated_cells;
	std::list<INT2> edges;
	std::list<INT2> surface_edges;

	bool invalid;
	TetResults():invalid(false){}
};

_PLC PLCConstructionAndElementDeletion(std::list<MeshOps::FaceRef>& H, std::set<Vertex*>& I, std::set<Vertex*>& B, std::list<Cell*>& C, std::list<Tetra*>& T, Organism* o, std::list<HullFaceInfo>& hullFaceInfo);
TetResults Tetrahedralise(_PLC, Organism* o);
bool TetResultsAdditionAndIntegration(_PLC plc, TetResults& tr, Organism* o, std::list<HullFaceInfo>& hullFaceInfo);

bool Transform::Complexify(std::list<Cell*> C, std::list<Tetra*> T, Organism* o)
{
	// PLC = empty
	// build hull H
	// for each boundary face f in H
	//   delete f and add cells to center of faces
	//   remove f from H, add new cells and c in f to PLC
	// add cells to center of each T, and to PLC
	// add all cells in each T to PLC
	// for each FaceRef fr in H:
	//   record neighbour tn, neighbour index tni, and vertices of the face
	// remove T from O, and delete, don't forget internal edges
	// add H to PLC
	// tetrahedralise(H)
	// put new verts into o
	// put new tets into o
	// set up internal neighbourhood relations
	// set up external neighbourhood relations
	// add surface faces and set up surface vertex neighbours

	std::list<HullFaceInfo> hullFaceInfo;

	std::list<MeshOps::FaceRef> H = MeshOps::getHullFromTetrahedraSet(o->mesh(), T);

	std::cerr << "H, size=" << H.size() << std::endl;

	// extract vertices that are internal to the hull H
	std::set<Vertex*> I, B;
	BOOST_FOREACH(Tetra* t, T)
	{
		BOOST_FOREACH(Vertex* v, t->vertices())
		{
			bool isInternal = true;

			// if v is not in H then it is in I
			BOOST_FOREACH(MeshOps::FaceRef f, H)
			{
				Vertex *v0,*v1,*v2;
				boost::tie(v0,v1,v2) = f.tetra->oppositeFaceVertices(f.index);
				if (v==v0 or v==v1 or v==v2)
				{
					//then v is on the surface
					isInternal = false;
					break;
				}
				else
				{
					// v may be internal
					// but have to check different faces
				}
			}

			if (isInternal)
				I.insert(v);
			else
				B.insert(v);
		}
	}

	// POST: H is modified
	_PLC PLC = PLCConstructionAndElementDeletion(H,I,B,C,T,o,hullFaceInfo);
	if (PLC.invalid)
	{
		return false;
	}

	TetResults results = Tetrahedralise(PLC,o);
	if (results.invalid)
	{
		return false;
	}

	return TetResultsAdditionAndIntegration(PLC,results, o, hullFaceInfo);
}

_PLC PLCConstructionAndElementDeletion(std::list<MeshOps::FaceRef>& H, std::set<Vertex*>& I, std::set<Vertex*>& B, std::list<Cell*>& C, std::list<Tetra*>& T, Organism* o, std::list<HullFaceInfo>& hullFaceInfo)
{
	std::cerr << "PLCConstructionAndElementDeletion" << std::endl;

	_PLC PLC;

	// for each boundary face f in H
	//   delete f and add cells to center of faces
	//   remove f from H, add new cells and c in f to PLC
	//   remove all edges attached to boundary cells in C

	std::list<MeshOps::FaceRef> boundaryFaces;
	BOOST_FOREACH(MeshOps::FaceRef f, H)
	{
		if (f.tetra->neighbour(f.index)==NULL)
		{
			boundaryFaces.push_back(f);
		}
	}

	BOOST_FOREACH(MeshOps::FaceRef& f, boundaryFaces)
	{
		// add cell to center of f
		// mass and radius are not set
		Vector3d centroid = f.tetra->getFaceCentroid(f.index);
		Vertex* cv = new Vertex(centroid, 0);
		Cell* c = new Cell(cv, 0);
		o->addCell(c);
		o->mesh()->addVertex(cv);
		cv->setSurface(true);

		PLC.cells.push_back(c);

		// delete f
		H.remove(f);
		Face* face = o->mesh()->getFace(f.tetra,f.index);
		if (face==NULL)
		{
			PLC.invalid = true;
			return PLC;
		}
		o->mesh()->deleteFace(face);
	}

	// Remove all edges connected to the proliferating cells
	BOOST_FOREACH(Cell* c, C)
	{
		if (c->v()->surface())
		{
			// remove all connected edges, if they haven't been removed alredy
			std::list<Edge*> edgesToRemove;
			BOOST_FOREACH(Vertex* vn, c->v()->neighbours())
			{
				Edge* e = o->mesh()->getEdge(c->v(),vn);
				if (e!=NULL)
					edgesToRemove.push_back(e);
			}

			//delete them in a separate loop, so c->v()->neighbours() isn't disrupted
			BOOST_FOREACH(Edge* e, edgesToRemove)
				o->mesh()->deleteEdge(e);
		}
	}

	// record all edges that are connected to vertices on H but
	// that are not internal to H
	BOOST_FOREACH(Vertex* v, B)
	{
		BOOST_FOREACH(Vertex* vn, v->neighbours())
		{
			if (I.count(vn)==0)
			{
				Edge* e = o->mesh()->getEdge(v,vn);
				if (PLC.allEdgesAttachedToAnyCellOnPLC.count(e)==0)
					PLC.allEdgesAttachedToAnyCellOnPLC.insert(e);
			}
		}
	}

	// record all tets that are connected to vertices on F but
	// are not internal to H
	BOOST_FOREACH(Tetra* t, o->mesh()->tetras())
	{
		BOOST_FOREACH(Vertex* v, B)
		{
			if (t->contains(v))
			{
				// check that t isn't in T
				if (std::find(T.begin(),T.end(),t)==T.end())
				{
					// then add it
					PLC.allTetraAttachedToAnyCellOnPLC.push_back(t);
				}
			}
		}
	}

	// add cells to center of each T, and PLC
	// add all cells in each T to PLC
	BOOST_FOREACH(const Tetra* t, T)
	{
		Vector3d p = t->center();
		Vertex* v = new Vertex(p,0);
		Cell* c = new Cell(v, 0);
		o->addCell(c);
		o->mesh()->addVertex(v);

		PLC.cells.push_back(c);
		BOOST_FOREACH(Vertex* vt, t->vertices())
		{
			Cell* ci = o->getAssociatedCell(vt);
			if (std::find(PLC.cells.begin(),PLC.cells.end(),ci)==PLC.cells.end())
			{
				PLC.cells.push_back(ci);
			}
		}
	}

	// record tet neighbour info
	// and add H to PLC
	BOOST_FOREACH(MeshOps::FaceRef fr, H)
	{
		HullFaceInfo hfi; // {tn,tni,(vj,vk,vl)}
		hfi.tn = fr.tetra->neighbour(fr.index);
		hfi.tni = -1;
		if (hfi.tn!=NULL)
			hfi.tni = hfi.tn->getNeighbourIndex(fr.tetra);
		int j,k,l;
		fr.tetra->oppositeFaceIndices(fr.index,j,k,l);
		hfi.verts = boost::make_tuple(&fr.tetra->v(j),&fr.tetra->v(k),&fr.tetra->v(l));
		hullFaceInfo.push_back(hfi);

		PLC.faces.push_back(hfi.verts);
	}

	// remove T from O, and delete, don't forget internal edges
	std::list<MeshOps::FaceRef> HplusBoundary = std::list<MeshOps::FaceRef>(H);
	HplusBoundary.insert(HplusBoundary.end(), boundaryFaces.begin(), boundaryFaces.end());

	BOOST_FOREACH(Tetra* t, T)
	{
		o->mesh()->removeTetra(t);

		// for each pair of vertices in T:
		//   if the pair is not part of a face on the hull H+boundary:
		//     remove and delete the edge between them
		BOOST_FOREACH(Vertex* vi, t->vertices())
		{
			BOOST_FOREACH(Vertex* vj, t->vertices())
			{
				if (vj==vi) continue;

				bool onHull = false;
				BOOST_FOREACH(MeshOps::FaceRef fr, HplusBoundary)
				{
					boost::tuple<Vertex*,Vertex*,Vertex*> vt = fr.tetra->oppositeFaceVertices(fr.index);

					if (vi==vt.get<0>())
					{
						if (vj==vt.get<1>() or vj==vt.get<2>()) onHull = true;
					}
					else if (vi==vt.get<1>())
					{
						if (vj==vt.get<0>() or vj==vt.get<2>()) onHull = true;
					}
					else if (vi==vt.get<2>())
					{
						if (vj==vt.get<0>() or vj==vt.get<1>()) onHull = true;
					}

					if (onHull)
						break;
				}

				if (not onHull)
				{
					// delete the edge
					Edge* e = o->mesh()->getEdge(vi,vj);
					if (e==NULL) // has already been removed
					{
						// PLC.invalid = true;
						// return PLC;

					}
					else
						o->mesh()->deleteEdge(e);
				}
			}
		}

		delete t;
	}
	return PLC;
}

TetResults Tetrahedralise(_PLC PLC, Organism* o)
{
	std::cerr << "Tetrahedralise" << std::endl;

	TetResults tr;

	// Convert PLC to a compatible format
	std::vector<Vector3d> points;
	std::vector<INT3> trianglesOnHull;
	std::map<Vertex*,int> vmap;
	std::vector<Vertex*> vertices;
	std::vector<bool> isolatedCells(PLC.cells.size(), true); // cell[i] is isolated iff isolatedCells[i]==true

	int index = 0;
	BOOST_FOREACH(Cell* c, PLC.cells)
	{
		points.push_back(c->x());
		vmap[c->v()] = index++;
		vertices.push_back(c->v());
	}

	BOOST_FOREACH(VERT3 v3, PLC.faces)
	{
		INT3 vtx3 = INT3(vmap[v3.get<0>()],vmap[v3.get<1>()],vmap[v3.get<2>()]);
		trianglesOnHull.push_back(vtx3);

		isolatedCells[vtx3.get<0>()] = false;
		isolatedCells[vtx3.get<1>()] = false;
		isolatedCells[vtx3.get<2>()] = false;

		tr.surface_edges.push_back(INT2(vtx3.get<0>(),vtx3.get<1>()));
		tr.surface_edges.push_back(INT2(vtx3.get<0>(),vtx3.get<2>()));
		tr.surface_edges.push_back(INT2(vtx3.get<1>(),vtx3.get<2>()));

	}

	// find a valid tetrahedralisation of the new hull and two points
	std::vector<INT4> newTetrahedra;
	std::vector<INT4> newTetrahedraNeighbours;
	std::vector<INT2> newEdges;

	std::vector<Vector3d>::size_type numPointsBefore = points.size();
	Math::tetrahedralise(points,
						 trianglesOnHull,
						 newTetrahedra,
						 newTetrahedraNeighbours,
						 newEdges);
	std::vector<Vector3d>::size_type numPointsAfter = points.size();

	// group results into TetResults
	int numNewTetrahedra = newTetrahedra.size();
	int numNewEdges = newEdges.size();

	// new_vertices
	for(int i=numPointsBefore; i<numPointsAfter; i++)
	{
		Vector3d x = points[i];
		Vertex* v = new Vertex(x,0);
		tr.new_vertices.push_back(v);
		vertices.push_back(v);
	}

	// cells
	tr.cells = PLC.cells;

	// tetrahedra
	for(unsigned int i=0;i<newTetrahedra.size();i++)
	{
		INT4 t = newTetrahedra[i];
		// XXX: We switch vertex indices 2 and 3 to get the correct orientation
		int vi[4] = {t.get<0>(),t.get<1>(),t.get<3>() /*NB: not 2*/,t.get<2>() /*NB: not 3*/};
		Vertex* v[4] = {vertices[vi[0]],vertices[vi[1]],vertices[vi[2]],vertices[vi[3]]};
		Tetra* new_tet = new Tetra(v[0],v[1],v[2],v[3],0);

		tr.tetrahedra.push_back(new_tet);
	}

	// tetrahedra neighbours, and build surface_tetrahedra list
	//   mark as (new|isolated)+surface
	// TODO: need to switch 2/3
	for(unsigned int i=0; i<newTetrahedraNeighbours.size(); i++)
	{
		INT4 tn = newTetrahedraNeighbours[i];
		Tetra* t = tr.tetrahedra[i];
		INT4 ti = newTetrahedra[i];
		// XXX: We switch vertex indices 2 and 3 to get the correct orientation
		int vi[4] = {ti.get<0>(),ti.get<1>(),ti.get<3>(),ti.get<2>()};
		int tn_indices[] = {tn.get<0>(),tn.get<1>(),tn.get<3>(),tn.get<2>()};

		bool added_to_surface = false;
		for(int j=0;j<4;j++)
		{
			int ti = tn_indices[j];
			if (ti==-1)
			{
				t->setNeighbour(j, NULL);
				if (!added_to_surface)
				{
					added_to_surface = true;
					tr.surface_tetrahedra.push_back(t);

					bool attached_to_new_or_isolated_cell = false;
					for(int k=0;k<4;k++)
					{
						if (vi[k]>=numPointsBefore or isolatedCells[vi[k]]==true)
						{
							attached_to_new_or_isolated_cell = true;
							break;
						}
					}

					if (attached_to_new_or_isolated_cell)
						tr.surface_tetrahedra_which_are_attached_to_new_or_isolated_cells.push_back(t);
				}
			}
			else
				t->setNeighbour(j, tr.tetrahedra[ti]);
		}
	}

	// edges
	tr.edges.insert(tr.edges.end(), newEdges.begin(),newEdges.end());

	return tr;
}

bool TetResultsAdditionAndIntegration(_PLC PLC, TetResults& results, Organism* o, std::list<HullFaceInfo>& hullFaceInfo)
{
	std::cerr << "TetResultsAdditionAndIntegration" << std::endl;

	std::vector<Cell*> all_affected_cells = std::vector<Cell*>(results.cells.begin(),results.cells.end());
	std::list<Edge*> all_affected_edges = std::list<Edge*>(PLC.allEdgesAttachedToAnyCellOnPLC.begin(),PLC.allEdgesAttachedToAnyCellOnPLC.end());
	std::list<Tetra*> all_affected_tetras = std::list<Tetra*>(PLC.allTetraAttachedToAnyCellOnPLC.begin(),PLC.allTetraAttachedToAnyCellOnPLC.end());

	// put new verts into o
	BOOST_FOREACH(Vertex* v, results.new_vertices)
	{
		o->mesh()->addVertex(v);
		Cell* c = new Cell(v, 0);
		o->addCell(c);
		all_affected_cells.push_back(c);
	}

	// put new tets into o
	BOOST_FOREACH(Tetra* t, results.tetrahedra)
	{
		o->mesh()->addTetra(t);
		all_affected_tetras.push_back(t);
	}

	// set up internal neighbourhood relations
	//  first, edges
	BOOST_FOREACH(INT2 edge, results.edges)
	{
		// if edge is new then add it
		INT2 edge_backward = INT2(edge.get<1>(),edge.get<0>());
		if ((std::find(results.surface_edges.begin(),results.surface_edges.end(),edge)==results.surface_edges.end()))
			if ((std::find(results.surface_edges.begin(),results.surface_edges.end(),edge_backward)==results.surface_edges.end()))
			{
				Vertex* v0 = all_affected_cells[edge.get<0>()]->v();
				Vertex* v1 = all_affected_cells[edge.get<1>()]->v();

				Edge* e = new Edge(v0, v1, 0);
				o->mesh()->addEdge(e);
				v0->addNeighbour(v1);
				v1->addNeighbour(v0);

				all_affected_edges.push_back(e);

				// e->setRestMultiplier() ?
			}
	}

	// all cells now have all their neighbourhoods set
	// compute the size of all the cells by averaging the size of all edges
	// then compute the cell mass
	BOOST_FOREACH(Cell* c, all_affected_cells)
	{
		Vector3d x = c->x();
		double totalLength = 0;
		BOOST_FOREACH(Vertex* n, c->v()->neighbours())
		{
			totalLength += ((n->x()-x).length()/2);
		}
		totalLength /= c->v()->neighbours().size();

		c->setR(totalLength);
		c->setM(Math::volumeOfSphereGivenRadius(totalLength));
	}

	// compute rest lengths for all involved edges and tetrahedra
	BOOST_FOREACH(Edge* e, all_affected_edges)
	{
		double rest = o->getAssociatedCell(e->v(0))->r() + o->getAssociatedCell(e->v(1))->r();
		e->setRest(rest);
		e->setRestMultiplier(e->length()/rest);
	}

	BOOST_FOREACH(Tetra* t, all_affected_tetras)
	{
		double rest = Transform::tetrahedraRestVolume(&t->v(0),
				&t->v(1),
				&t->v(2),
				&t->v(3),o);
		t->setRest(rest);
		t->setRestMultiplier(t->volume()/rest);
	}

	// set up external neighbourhood relations
	//  tetras, for each outside tetra on the hull boundary, join it up with  the matching internal tetra
	//          we do this by finding the tet that has the matching set of three verts
	BOOST_FOREACH(HullFaceInfo hfi, hullFaceInfo)
	{
		Tetra* t = NULL;
		BOOST_FOREACH(Tetra* rt, results.tetrahedra)
		{
			if (rt->contains(hfi.verts.get<0>(),hfi.verts.get<1>(),hfi.verts.get<2>()))
			{
				t = rt;
				break;
			}
		}

		// compute ti = index of face hfi.verts
		int ti = 6 - (t->vIndex(hfi.verts.get<0>()) + t->vIndex(hfi.verts.get<1>()) + t->vIndex(hfi.verts.get<2>()));

		t->setNeighbour(ti, hfi.tn);
		hfi.tn->setNeighbour(hfi.tni, t);
	}

	// add surface faces and set up surface vertex neighbours
	//   the last thing to do is add the surface faces and reconnect all neighbours
	BOOST_FOREACH(Tetra* t, results.surface_tetrahedra_which_are_attached_to_new_or_isolated_cells)
	{
		for(int i=0;i<4;i++)
		{
			if (t->neighbour(i)==NULL)
			{
				// add face to o->mesh()
				Face* f = t->createFace(i);
				o->mesh()->addOuterFace(f);
				o->mesh()->addFaceNeighbourLinks(f);
			}
		}
	}

	return true;
}

