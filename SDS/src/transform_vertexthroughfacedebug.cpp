#include "transform.h"



void Transform::MoveVertexThroughOppositeFace(int vi, Tetra* t, Organism* o)
{
	reset();
	called("MoveVertexThroughOppositeFace");

	LOG("V-F ");

	int vai, vbi, vci;
	t->oppositeFaceIndices(vi,vai,vbi,vci);
	Face* f0 = o->mesh()->getSurfaceFace(&t->v(vai),&t->v(vbi),&t->v(vci));
	if (f0==NULL) // CASE 1
	{
		LOG("V-F Internal\n");
		vfInternalCase(vi,vai,vbi,vci,t,o);
	}
	else // CASE 2
	{
		LOG("V-F External\n");
		vfBoundaryCase(vi,vai,vbi,vci,t,f0,o);
	}

	finaliseProperties();
}

/**********************************************************
 * MOVEMENT -- private helpers
 */

/// PRE: tdash, vdash both exist
void Transform::vfInternalCase(int vi, int ai, int bi, int ci, Tetra* t, Organism* o)
{
	LOG(vi << " " << ai << " " << bi << " " << ci << "\n");

	Vertex *v = &t->v(vi), *va = &t->v(ai), *vb = &t->v(bi), *vc = &t->v(ci);
	Tetra* tdash = NULL;
	Vertex* vdash = NULL;

	// find t' and v'
	tdash = t->neighbour(vi);
	if (!tdash)
	{
		error("tdash not there");
		return;
	}
	// assert(tdash);

	LOG("tdash: " << tdash << "\n");

	int aidash = tdash->vIndex(va), bidash = tdash->vIndex(vb), cidash = tdash->vIndex(vc);
	int vdashindex = 6 - aidash - bidash - cidash;
	vdash = &tdash->v(vdashindex);
	if (!vdash)
	{
		error("vdash not there");
		return;
	}
	//assert(vdash);

	// add 3 new tetrahedra
	// ta = v',vc,vb,v, tb = v',v,va,vc, tc = v',v,vb,va
	// and compute rest volume
	Cell *cvdash = o->getAssociatedCell(vdash),
		*cv = o->getAssociatedCell(v),
		*cva = o->getAssociatedCell(va),
		*cvb = o->getAssociatedCell(vb),
		*cvc = o->getAssociatedCell(vc);

	Tetra* ta = new Tetra(vdash,vc,vb,v,Transform::tetrahedraRestVolume(cvdash,cvc,cvb,cv));
	Tetra* tb = new Tetra(vdash,v,va,vc,Transform::tetrahedraRestVolume(cvdash,cv,cva,cvc));
	Tetra* tc = new Tetra(vdash,v,vb,va,Transform::tetrahedraRestVolume(cvdash,cv,cvb,cva));

	LOG("3 new tets: " << ta << " " << tb << " " << tc << "\n");

	o->mesh()->addTetra(ta);
	o->mesh()->addTetra(tb);
	o->mesh()->addTetra(tc);

	// set the correct neighbourhood links
	ta->setNeighbour(0,t->neighbour(ai));
	ta->setNeighbour(1,tc);
	ta->setNeighbour(2,tb);
	ta->setNeighbour(3,tdash->neighbour(aidash));

	tb->setNeighbour(0,t->neighbour(bi));
	tb->setNeighbour(1,tdash->neighbour(bidash));
	tb->setNeighbour(2,ta);
	tb->setNeighbour(3,tc);

	tc->setNeighbour(0,t->neighbour(ci));
	tc->setNeighbour(1,tdash->neighbour(cidash));
	tc->setNeighbour(2,tb);
	tc->setNeighbour(3,ta);

	// -------- fix tetra neighbourhood links

	// anything that linked to t should now link to one of these new tets
	Tetra 	*tetc = t->neighbour(ci),
			*tetb = t->neighbour(bi),
			*teta = t->neighbour(ai),
			*tetcdash = tdash->neighbour(cidash),
			*tetbdash = tdash->neighbour(bidash),
			*tetadash = tdash->neighbour(aidash);

	if (teta) teta->replaceNeighbour(t,ta);
	if (tetb) tetb->replaceNeighbour(t,tb);
	if (tetc) tetc->replaceNeighbour(t,tc);
	if (tetadash) tetadash->replaceNeighbour(tdash,ta);
	if (tetbdash) tetbdash->replaceNeighbour(tdash,tb);
	if (tetcdash) tetcdash->replaceNeighbour(tdash,tc);

	// connect v to v', add edge (v,v')
	v->addNeighbour(vdash);
	vdash->addNeighbour(v);
	Edge* e = new Edge(v,vdash,cv->r() + cvdash->r());
	o->mesh()->addEdge(e);

	// delete old tets
	o->mesh()->removeTetra(t);
	o->mesh()->removeTetra(tdash);

	delete tdash;
	delete t;
}

/// PRE: f0=(ai,bi,ci) is a surface face
void Transform::vfBoundaryCase(int vi, int ai, int bi, int ci, Tetra* t, Face* f0, Organism* o)
{
	Vertex *v = &t->v(vi), *va = &t->v(ai), *vb = &t->v(bi), *vc = &t->v(ci);

	if (not v->surface()) // CASE 2.1
	{
		// In this case, we just compress t into its three faces, removing f0 and t in the process
		// note than none of the following faces already exist..

		Face* fc = new Face(v,va,vb);
		Face* fb = new Face(v,vc,va);
		Face* fa = new Face(v,vb,vc);

		fc->setOuter(true);
		fb->setOuter(true);
		fa->setOuter(true);

		o->mesh()->addOuterFace(fa);
		o->mesh()->addOuterFace(fb);
		o->mesh()->addOuterFace(fc);

		Mesh::addFaceNeighbourLinks(fa);
		Mesh::addFaceNeighbourLinks(fb);
		Mesh::addFaceNeighbourLinks(fc);

		v->setSurface(true);
		va->setSurface(true);
		vb->setSurface(true);
		vc->setSurface(true);

		// fix up boundary topology
		// We know that all these neighbours exist as v is not on the surface
		t->neighbour(ai)->replaceNeighbour(t,NULL);
		t->neighbour(bi)->replaceNeighbour(t,NULL);
		t->neighbour(ci)->replaceNeighbour(t,NULL);

		// remove dead elements
		Mesh::removeFaceNeighbourLinks(f0);
		o->mesh()->removeFace(f0);
		o->mesh()->removeTetra(t);
		delete f0;
		delete t;
	}
	else // CASE 2.2
	{
		// v is on the surface, this breaks down into further subcases

		// CASE 2.2.1 (Single tetrahedra)
		if (t->neighbour(0)==NULL and
			t->neighbour(1)==NULL and
			t->neighbour(2)==NULL and
			t->neighbour(3)==NULL)
		{
			Tetra* tnew = new Tetra(&t->v(0),&t->v(1),&t->v(3),&t->v(2),t->rest());
			o->mesh()->addTetra(tnew);
			o->mesh()->removeTetra(t);
			delete t;

			Face* fa = o->mesh()->getSurfaceFace(v,vb,vc);
			fa->setVertices(&fa->v(0),&fa->v(2),&fa->v(1));

			Face* fb = o->mesh()->getSurfaceFace(v,va,vc);
			fb->setVertices(&fb->v(0),&fb->v(2),&fb->v(1));

			Face* fc = o->mesh()->getSurfaceFace(v,va,vb);
			fc->setVertices(&fc->v(0),&fc->v(2),&fc->v(1));

			f0->setVertices(&f0->v(1),&f0->v(0),&f0->v(2));
		}
		else // CASE 2.2.2, 2.2.3 and 2.2.4
		{
			// TODO: fix this?
			// XXX: This is an extreme simplification of all these cases
			// A more complete approach is given in the tech report
			// but I don't have time at the moment!

			// Add a new vertex and tetrahedra opposite v
			// then simply perform an internal v-f move


			Cell* c = o->getAssociatedCell(v);
			Vector3d dir = f0->n().normalise();

			Vertex* vdash = new Vertex(v->x() + dir*c->r(),c->m());
			vdash->setSurface(true);
			Cell* cdash = new Cell(vdash,c->r());

			o->mesh()->addVertex(vdash);
			o->addCell(cdash);

			// add the tetrahedron, faces, edges

			Tetra* tdash = new Tetra(vdash,va,vc,vb,t->rest());
			o->mesh()->addTetra(tdash);

			// ..faces
			Face* fda = new Face(vdash,vb,vc); o->mesh()->addOuterFace(fda);
			Face* fdb = new Face(vdash,vc,va); o->mesh()->addOuterFace(fdb);
			Face* fdc = new Face(vdash,va,vb); o->mesh()->addOuterFace(fdc);

			vdash->setSurface(true);

			Mesh::addFaceNeighbourLinks(fda);
			Mesh::addFaceNeighbourLinks(fdb);
			Mesh::addFaceNeighbourLinks(fdc);

			// ..edges
			Edge* evda = new Edge(vdash,va, o->getAssociatedCell(va)->r() + cdash->r());
			Edge* evdb = new Edge(vdash,vb, o->getAssociatedCell(vb)->r() + cdash->r());
			Edge* evdc = new Edge(vdash,vc, o->getAssociatedCell(vc)->r() + cdash->r());

			o->mesh()->addEdge(evda);
			o->mesh()->addEdge(evdb);
			o->mesh()->addEdge(evdc);

			evda->v(0)->addNeighbour(evda->v(1));
			evda->v(1)->addNeighbour(evda->v(0));

			evdb->v(0)->addNeighbour(evdb->v(1));
			evdb->v(1)->addNeighbour(evdb->v(0));

			evdc->v(0)->addNeighbour(evdc->v(1));
			evdc->v(1)->addNeighbour(evdc->v(0));

			// set f as a non-surface face
			Mesh::removeFaceNeighbourLinks(f0);
			o->mesh()->removeFace(f0);
			//o->mesh()->addFace(f0);
			delete f0;

			// then call the vfInternalCase
			vfInternalCase(vi,ai,bi,ci,t,o);
		}

	}
}

