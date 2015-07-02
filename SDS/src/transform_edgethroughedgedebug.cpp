#include "transform.h"

void Transform::MoveEdgeThroughEdge(int ea, int eb, Tetra* t, Organism* o)
{
	called("MoveEdgeThroughEdge");

	LOG("Edge Through Edge " << ea << " " << eb << " " << reinterpret_cast<void*>(t) << "\n");

	static std::string substep = "start";

	properties.add("substep",substep);

	if (state == TRANSFORM_COMPLETED)
	{
		reset();
		substep = "start";
		state = TRANSFORM_TRANSFORMING;
	}
	else if (substep=="finish")
	{
		substep = "start";
		state = TRANSFORM_COMPLETED;
		finaliseProperties();
		return;
	}

	//-------- Given that edge (t->v(ea),t->v(eb)) intersects the
	//-------- opposite edge in t, perform an e-e move

	// vertexOrdering[i] = indices of vertices of t->f(i) ordered clockwise when viewed from t->v(i)
	static int vertexOrdering[4][3] = {{1,2,3},{0,3,2},{0,1,3},{1,0,2}};

	// label the vertices of t consistently
	static int v[4];

	if (substep=="start")
	{
		v[0] = ea;
		v[1] = eb;

		int ebi;
		for(ebi=0;ebi<3;ebi++)
			if (vertexOrdering[v[0]][ebi]==eb) break;
		if (ebi==3)
		{
			error("ebi==3");
			return;
		}
		v[2] = vertexOrdering[v[0]][(ebi+1)%3];
		v[3] = vertexOrdering[v[0]][(ebi+2)%3];
		if ((v[0]+v[1]+v[2]+v[3])!=6)
		{
			error("vertex index sum != 6");
			return;
		}
	}

	// edge (v0,v1) intersects edge (v2,v3)

	// Construct the lists U3, U2, D1, D0, U and D (lists of tetrahedra)
	// also U3V, U2V, D1V, D0V, UV, and DV (lists of vertex indices)

	// U3i0 = t
	// U3V0 = 3
	// loop
	//  U3i+1 = U3i->n(U3Vi)
	//  if (U3i+1 == NULL)
	//   U = U3, U2 = NULL, quit
	//  else if (U3+1 == t)
	//   Build U2
	//   U = U3 and U2
	//   break;
	//  else // continue
	//   U3Vi+1 = U3i+1->vIndex(v), where v in U3i->f(U3Vi) and v!=v0 and v!=v1 (there exists exactly one)


	// considering the above sets are all generated in a similar way, create a function to do it for us

	// input: t,i,e=(e.a,e.b)
	//   populate a list with the tetrahedra sharing edge e, proceeding in direction i
	//   also populate a list with vertex indices for later

	// X0 = t, V0 = i
	// loop
	//  Xi+1 = Xi->n(Vi)
	//  if (Xi+1==NULL)
	//    finish (ignore Xi+1)
	//  else if (Xi+1 == t)
	//    finish (ignore Xi+1)
	//  else
	//    Vi+1 = Xi+1->vIndex(v), where Xi->f(Vi).contains(v), v!=e.a, v!=e.b
	//    push Xi+1 onto X, push Vi+1 onto V

	static std::vector<Tetra*> UandD[4];
	static std::vector<int> UVandDV[4];

	// if at the start state, then we reinitialise UandD, UVandDV
	if (substep=="start")
	{
		static int i;
		static std::string subsubstep = "start";
		if (subsubstep=="start")
		{
			i = 0;
			subsubstep = "stepping";
		}

		if (i >= 4)
		{
			subsubstep = "start";
			substep = "next";
		}
		else
		{
			UandD[i].clear();
			UVandDV[i].clear();

			std::vector<Tetra*>& X = UandD[i];
			std::vector<int>& V = UVandDV[i];

			int edge[2];
			if (i==0 or i==1)
			{
				edge[0] = v[2];
				edge[1] = v[3];
			}
			else if (i==2 or i==3)
			{
				edge[0] = v[0];
				edge[1] = v[1];
			}

			// fill X and V with the tetrahedra surrounding the edge
			if (!findTetras(X,V,t,v[i],&t->v(edge[0]),&t->v(edge[1]),o))
			{
				//something went wrong
				state = TRANSFORM_ERROR;
				return;
			}

			// debug: expose the tetras to the user
			std::ostringstream puad;
			puad << "processing UandD[" << i << "]";
			properties.add("state",puad.str());

			std::ostringstream num;
			num << "t(" << i << ",";

			int counter = 0;
			BOOST_FOREACH(Tetra* t, UandD[i])
			{
				std::ostringstream numout;
				numout << num.str() << counter << ")";
				properties.add(numout.str(),t);
				counter++;
			}

			num.str("");
			num << "v(" << i << ",";
			for(unsigned int index=0;index<UVandDV[i].size();index++)
			{
				std::ostringstream numout;
				numout << num.str() << index << ")";
				properties.add(numout.str(),&(UandD[i][index]->v(UVandDV[i][index])));
			}

			// increase i
			i++;
		}

	}
	else if (substep=="next") // ELSE is necessary
	{

		std::vector<Tetra*>& D0 = UandD[0];
		std::vector<Tetra*>& D1 = UandD[1];
		std::vector<Tetra*>& U2 = UandD[2];
		std::vector<Tetra*>& U3 = UandD[3];

		// select the side with the least tetrahedra
		unsigned int UCount = U3.size();
		if (U3.size()==0 or U3[U3.size()-1]->neighbour(UVandDV[3][U3.size()-1])==NULL) // then a gap exists between U2 and U3
		{
			if (U2.size()!=0)
			{
				if (U2[U2.size()-1]->neighbour(UVandDV[2][U2.size()-1])!=NULL)
				{
					error("U2 issue");
					return;
				}
				UCount += U2.size();
			}
		}

		unsigned int DCount = D0.size();
		if (D0.size()==0 or D0[D0.size()-1]->neighbour(UVandDV[0][D0.size()-1])==NULL) // then a gap exits between D0 and D1
		{
			if (D1.size()!=0)
			{
				if (D1[D1.size()-1]->neighbour(UVandDV[1][D1.size()-1])!=NULL)
				{
					error("D1 issue");
					return;
				}
				DCount += D1.size();
			}
		}

		// let the chosen side be denoted X1, X2
		// with the vertex lists denoted V1 and V2
		// otherwise X1==X2 and we only need to process one side
		// let L be the ordered list of vertices
		// (starting at one of the verts of Q, and finishing at the opposite side)

		//  std::cerr << "U" << UCount << ", D" << DCount << "\n";

		int iv0, iv1, iv2, iv3;
		unsigned int XCount;

		std::vector<Tetra*> *pX1, *pX2, *pY1, *pY2;
		std::vector<int> *pV1, *pV2;

		// firstly choose the one with the gaps
		bool UhasGaps = not (U2.size()>0 and U3.size()>0 and UCount==U3.size());
		bool DhasGaps = not (D0.size()>0 and D1.size()>0 and DCount==D0.size());

		bool chooseU = UhasGaps;

		if ((!UhasGaps and !DhasGaps) or (UhasGaps and DhasGaps))
		{
			if (UCount <= DCount)
				chooseU = true;
			else
				chooseU = false;
		}

		LOG("chooseU? " << chooseU << "\n");

		// XXX: Does this assertion protect us from any bad behaviour?
		// assert(not (UhasGaps and DhasGaps));

		// if (UhasGaps or (!DhasGaps and UCount<=DCount))
		if (chooseU)
		{
			pX1 = &U2;
			pX2 = &U3;
			pY1 = &D0;
			pY2 = &D1;
			pV1 = &UVandDV[2];
			pV2 = &UVandDV[3];
			iv0 = v[0];
			iv1 = v[1];
			iv2 = v[2];
			iv3 = v[3];
			XCount = UCount;
		}
		else
		{
			pX1 = &D1;
			pX2 = &D0;
			pY1 = &U3;
			pY2 = &U2;
			pV1 = &UVandDV[1];
			pV2 = &UVandDV[0];
			iv0 = v[3];
			iv1 = v[2];
			iv2 = v[1];
			iv3 = v[0];
			XCount = DCount;
		}

		std::vector<Tetra*> &X1 = *pX1, &X2 = *pX2;
		std::vector<Tetra*> &Y1 = *pY1, &Y2 = *pY2;
		std::vector<int> &V1 = *pV1, &V2 = *pV2;

		LOG("XCount: " << XCount << "\n");
		LOG("iv: " << iv0 << " " << iv1 << " " << iv2 << " " << iv3 << "\n");

		if (not o->mesh()->getEdge(&t->v(iv2),&t->v(iv1)))
		{
			error("expecting edge, got nothing");
			return;
		}

		// test for a gap
		if (not (UhasGaps or DhasGaps))
		{
			LOG("no gap\n");

			// enumerate the points of the hull
			Vertex* u1 = &t->v(iv0);
			Vertex* u2 = &t->v(iv1);

			std::vector<Vector3d> points;
			std::vector<Vertex*> verts;
			double averageMass = 0;

			for(unsigned int i=0;i<X1.size();i++)
			{
				averageMass += X1[i]->v(V1[i]).m();
				verts.push_back(&X1[i]->v(V1[i]));
				points.push_back(X1[i]->v(V1[i]).x());
			}
			averageMass /= X1.size();

			verts.push_back(&t->v(iv2));
			points.push_back(t->v(iv2).x());

			int uIndex = points.size();
			points.push_back(u1->x());
			points.push_back(u2->x());
			verts.push_back(u1);
			verts.push_back(u2);

			// build the hull and delete its internals
			std::vector<boost::tuple<int,int,int> > hull;

			// keep a record of the neighbours of each hull face
			std::vector<Tetra*> hullN;
			std::vector<int> hullI;

			unsigned int polySize = points.size() - 2;
			for(unsigned int i=0;i<polySize;i++)
			{
				int i1 = i, i2 = (i+1)%polySize;

				hull.push_back(boost::make_tuple(uIndex,i1,i2));
				hull.push_back(boost::make_tuple(uIndex+1,i2,i1));

				properties.add(boost::make_tuple(verts[uIndex],verts[i1],verts[i2]));
				properties.add(boost::make_tuple(verts[uIndex+1],verts[i2],verts[i1]));

				if (i==(polySize-1))
				{
					Tetra *tu2 = t->neighbour(u2), *tu1 = t->neighbour(u1);
					hullN.push_back(tu2);
					hullN.push_back(tu1);

					if (tu2) properties.add(tu2);
					if (tu1) properties.add(tu1);

					if (tu2) hullI.push_back(tu2->getNeighbourIndex(t));
					else hullI.push_back(-1);

					if (tu2) properties.add(&tu2->v(tu2->getNeighbourIndex(t)));

					if (tu1) hullI.push_back(tu1->getNeighbourIndex(t));
					else hullI.push_back(-1);

					if (tu1) properties.add(&tu1->v(tu1->getNeighbourIndex(t)));
				}
				else
				{
					Tetra *tu2 = X1[i]->neighbour(u2), *tu1 = X1[i]->neighbour(u1);
					hullN.push_back(tu2);
					hullN.push_back(tu1);

					if (tu2) properties.add(tu2);
					if (tu1) properties.add(tu1);

					if (tu2) hullI.push_back(tu2->getNeighbourIndex(X1[i]));
					else hullI.push_back(-1);

					if (tu2) properties.add(&tu2->v(tu2->getNeighbourIndex(X1[i])));

					if (tu1) hullI.push_back(tu1->getNeighbourIndex(X1[i]));
					else hullI.push_back(-1);

					if (tu1) properties.add(&tu1->v(tu1->getNeighbourIndex(X1[i])));
				}
			}

			// tetrahedralise the hull
			std::vector<boost::tuple<int,int,int,int> > tetra;
			int pSizeBefore = points.size();

			try
			{
				Math::tetrahedralise(points,hull,tetra);
			}
			catch(int i)
			{
				// something has gone wrong here...
				state = TRANSFORM_ERROR;
				return;
			}

			// tetrahedralisation worked
			// remove internals
			BOOST_FOREACH(Tetra* x, X1)
			{
				o->mesh()->removeTetra(x);
				delete x;
			}
			Edge* e = o->mesh()->getEdge(u1,u2);
			o->mesh()->deleteEdge(e);
			o->mesh()->removeTetra(t);
			delete t;

			// add new points
			std::vector<Vertex*> newPoints;
			LOG("newpoints: ");
			for(unsigned int i=pSizeBefore;i<points.size();i++)
			{
				Vertex* v = new Vertex(points[i][0],points[i][1],points[i][2],averageMass);
				properties.add(v);

				Cell* c = new Cell(v,Math::radiusOfSphereGivenVolume(averageMass));
				o->mesh()->addVertex(v);
				o->addCell(c);
				newPoints.push_back(v);
				verts.push_back(v);
			}

			// add new vertices, edges, and tetras to mesh
			std::vector<Tetra*> newTets;

			LOG("newtets: ");
			typedef boost::tuple<int,int,int,int> TET;
			BOOST_FOREACH(TET& tet, tetra)
			{
				LOG(tet.get<0>() << tet.get<1>() << tet.get<2>() << tet.get<3>() << " ");

				// add this tetra to mesh
				Vertex *vs[] = { verts[tet.get<0>()],verts[tet.get<1>()],verts[tet.get<3>()],verts[tet.get<2>()]};
				Cell *cs[] = {o->getAssociatedCell(vs[0]),o->getAssociatedCell(vs[1]),o->getAssociatedCell(vs[2]),o->getAssociatedCell(vs[3])};

				Tetra* newtet = new Tetra(vs[0],vs[1],vs[2],vs[3],Transform::tetrahedraRestVolume(cs[0],cs[1],cs[2],cs[3]));
				o->mesh()->addTetra(newtet);
				newTets.push_back(newtet);

				properties.add(newtet);
			}

			// fix up neighbourhood information

			BOOST_FOREACH(Tetra* newtet, newTets)
			{
				// for each face of newtet add its appropriate neighbour

				for(int i=0;i<4;i++)
				{
					// if t->face[i] is on the hull then use the hull neighbour info
					// otherwise search the newtets for the complimenting neighbour
					int f1,f2,f3;
					newtet->oppositeFaceIndices(i,f1,f2,f3);
					Vertex *fv1,*fv2,*fv3;
					fv1 = &newtet->v(f1);
					fv2 = &newtet->v(f2);
					fv3 = &newtet->v(f3);

					int ishull = -1;
					for(unsigned int j=0;j<hull.size();j++)
					{
						boost::tuple<int,int,int>& faceI = hull[j];
						Vertex* fv[3] = {verts[faceI.get<0>()],verts[faceI.get<1>()],verts[faceI.get<2>()]};
						// if {fv1,fv2,fv3}=={fv[0],fv[1],fv[2]} then this is a hull neighbour

						int count = 0;
						for(int k=0;k<3;k++)
						{
							if (fv1==fv[k] or fv2==fv[k] or fv3==fv[k]) count++;
						}
						if (count==3) // then this face is on the hull
						{
							ishull = j;
							break;
						}
					}

					if (ishull!=-1) // set the neighbour of this face to the respective hull neighbour
					{
						Tetra* tn = hullN[ishull];
						newtet->setNeighbour(i,tn);
						if (tn)	tn->setNeighbour(hullI[ishull],newtet);
						else // newtet is on the surface
						{
							// outer face is already there
							/*
							// this face is an outer face
							Face* f = new Face(fv1,fv3,fv2);
							f->setOuter(true);
							Mesh::addFaceNeighbourLinks(f);
							o->mesh()->addOuterFace(f);
							 */
						}
					}
					else // find the corresponding neighbour inside the hull
					{
						Tetra* tn = NULL;
						BOOST_FOREACH(Tetra* _tn, newTets)
						{
							if (newtet==_tn) continue;
							if (_tn->contains(fv1,fv2,fv3))
							{tn = _tn; break;}
						}
						if (!tn)
						{
							error("expecting neighbour tetrahedron, got nothing");
							return;
						}

						newtet->setNeighbour(i,tn);
					}

					// add edges
					// XXX: we are very inefficiently just trying to add each edge
					addEdgeIfNone(fv1,fv2,o);
					addEdgeIfNone(fv2,fv3,o);
					addEdgeIfNone(fv1,fv3,o);

					/*
					// add edges
					// if the face contains a point from new points, then a new edge must be added
					// but we must avoid adding duplicate edges too

					BOOST_FOREACH(Vertex* v, newPoints)
					{
						if (fv1==v)
						{
							addEdgeIfNone(fv1,fv2);
							addEdgeIfNone(fv1,fv3);
						}
						else if (fv2==v)
						{
							addEdgeIfNone(fv2,fv1);
							addEdgeIfNone(fv2,fv3);
						}
						else if (fv3==v)
						{
							addEdgeIfNone(fv3,fv1);
							addEdgeIfNone(fv3,fv2);
						}
					}

					 */
				}
			}

			// DONE?




			/*

			// triangulate L

			// perspective projection of points of L to a plane at u2 with normal n=u2-u1
			Vertex *u1 = &t->v(iv0), *u2 = &t->v(iv1);
			Vertex *v1 = &t->v(iv3), *v2 = &t->v(iv2);

			std::vector<Vector3i> Lfaces;
			if (!findTriangulation(L,u1,u2,v1,v2,Lfaces))
			{
				sState = UNSTABLE;
				properties.add("bad triangulation of collapsing tetra",t);
				return false;
			}

			std::map<int,boost::tuple<Tetra*,Tetra*,int> > mTetMap;

			// for each face in Lfaces
			BOOST_FOREACH(Vector3i faceIndices, Lfaces)
			{
				boost::tuple<Vertex*,Vertex*,Vertex*> fv(L[faceIndices[0]],L[faceIndices[1]],L[faceIndices[2]]);

				// create two tetras that share this face
				Vertex *v0 = &t->v(iv0), *v1 = &t->v(iv1), *va = fv.get<0>(), *vb = fv.get<1>(), *vc = fv.get<2>();

				Tetra* t0 = new Tetra(v0,va,vb,vc,0);
				Tetra* t1 = new Tetra(v1,va,vc,vb,0);
				// add t0 and t1 to organism
				o->mesh()->addTetra(t0);
				o->mesh()->addTetra(t1);

				// assert that v0 and v1 are on either sides of the face, i.e., that t0 and t1 have positive volumes

				LOG("t0v " << t0->volume() << " t1v " << t1->volume() << "\n");

				if (t0->volume()<0 or t1->volume()<0)
				{
					sState = UNSTABLE;
					properties.add("t0",t0);
					properties.add("t1",t1);
					return false;
				}

				// set their rest volumes appropriately
				Cell* c0 = o->getAssociatedCell(v0);
				Cell* c1 = o->getAssociatedCell(v1);
				Cell* ca = o->getAssociatedCell(va);
				Cell* cb = o->getAssociatedCell(vb);
				Cell* cc = o->getAssociatedCell(vc);

				t0->setRest(Transform::tetrahedraRestVolume(c0,ca,cb,cc));
				t1->setRest(Transform::tetrahedraRestVolume(c1,ca,cb,cc));

				// debug: output
				std::ostringstream oss;
				oss << "tetra " << t0;
				properties.add(oss.str(),t0);
				oss.str("");
				oss << "tetra " << t1;
				properties.add(oss.str(),t1);

				// set face neighbours
				t0->setNeighbour(0,t1);
				t1->setNeighbour(0,t0);

				// update surface face neighbourhood

				// we have: va,vb,vc is clockwise from (v0 p.o.v)
				// for any pair of va,vb,vc:
				//   if the pair is adjacent (i.e., an edge joins them)
				//     then they are either on the hull surface, or are part of the edge iv2,iv3
				//     if they are on the hull surface then identify the corresponding face, and update the neighbourhood
				//     if they are part of iv2,iv3 then update the neighbourhood of D0[0], or D1[0]
				//   if they are not adjacent,
				//     then create a new internal face in internalFaces
				//     the neighbours of which are updated in the next loop

				for(unsigned int k1=0; k1<3; k1++)
				{
					unsigned int k2 = (k1+1)%3;
					unsigned int k3 = (k1+2)%3;

					// check vertices vk1,vk2
					unsigned int vi1 = faceIndices[k1], vi2 = faceIndices[k2], vi3 = faceIndices[k3];

					if (vi1==(Lsize-1) and vi2==0)
					{
						// then it is part of edge iv2,iv3
						assert(L[vi1]==&t->v(iv2));
						assert(L[vi2]==&t->v(iv3));

						// update the neighbourhood of D0[0] and D1[0]
						if (not Y1.empty())
						{
							Tetra* D0 = Y1[0];
							int d0vi1 = D0->vIndex(L[vi1]);
							int d0vi2 = D0->vIndex(L[vi2]);
							int d0iv1 = D0->vIndex(&t->v(iv1));
							assert(d0vi1!=-1);
							assert(d0vi2!=-1);
							assert(d0iv1!=-1);

							assert(d0vi1!=d0vi2);
							assert(d0vi1!=d0iv1);
							assert(d0vi2!=d0iv1);

							int viD0 = 6 - (d0vi1 + d0vi2 + d0iv1); // compute the index of the opposite vertex
							assert(viD0 >= 0 and viD0 <= 3);
							D0->setNeighbour(viD0,t1);

							int t1lvi1 = t1->vIndex(L[vi1]);
							int t1lvi2 = t1->vIndex(L[vi2]);
							int t1iv1 = t1->vIndex(&t->v(iv1));
							assert(t1lvi1!=-1);
							assert(t1lvi2!=-1);
							assert(t1iv1!=-1);

							assert(t1lvi1!=t1lvi2);
							assert(t1iv1!=t1lvi1);
							assert(t1iv1!=t1lvi2);

							t1->setNeighbour(6 - (t1lvi1 + t1lvi2 + t1iv1),D0);
						}

						if (not Y2.empty())
						{
							Tetra* D1 = Y2[0];

							int d1vi1 = D1->vIndex(L[vi1]);
							int d1vi2 = D1->vIndex(L[vi2]);
							int d1iv0 = D1->vIndex(&t->v(iv0));
							assert(d1vi1!=-1);
							assert(d1vi2!=-1);
							assert(d1iv0!=-1);

							assert(d1vi1!=d1vi2);
							assert(d1vi1!=d1iv0);
							assert(d1vi2!=d1iv0);

							D1->setNeighbour(6 - (d1vi1 + d1vi2 + d1iv0),t0);

							int t0lvi1 = t0->vIndex(L[vi1]);
							int t0lvi2 = t0->vIndex(L[vi2]);
							int t0iv0 = t0->vIndex(&t->v(iv0));

							assert(t0lvi1!=-1);
							assert(t0lvi2!=-1);
							assert(t0iv0!=-1);
							assert(t0lvi1!=t0lvi2);
							assert(t0iv0!=t0lvi1);
							assert(t0iv0!=t0lvi2);

							t0->setNeighbour(6 - (t0lvi1+t0lvi2+t0iv0),D1);
						}

					}
					else if ((vi1+1)%Lsize == vi2) // and not the special edge n->0
					{
						// then vi1--vi2 is on the boundary
						assert(o->edge(o->getAssociatedCell(L[vi1]),o->getAssociatedCell(L[vi2])));

						// for each surface face, we can get the neighbouring tetrahedra on the outer of the hull
						// we can calculate this easy enough

						// Tetra* told = X2[vi1];
						Tetra* told = X1[vi1];

						Tetra* t1n = told->neighbour(&t->v(iv0));
						Tetra* t0n = told->neighbour(&t->v(iv1));

						int t0lvi3 = t0->vIndex(L[vi3]);
						int t1lvi3 = t1->vIndex(L[vi3]);
						assert(t0lvi3!=-1);
						assert(t1lvi3!=-1);

						if (t0n)
						{
							t0->setNeighbour(t0lvi3,t0n);
							assert(t0n->getNeighbourIndex(told)!=-1);
							t0n->replaceNeighbour(told,t0);
						}
						else // auto sets to outer
							t0->setNeighbour(t0lvi3,NULL);

						if (t1n)
						{
							t1->setNeighbour(t1lvi3,t1n);
							assert(t1n->getNeighbourIndex(told)!=-1);
							t1n->replaceNeighbour(told,t1);
						}
						else // auto sets to outer
							t1->setNeighbour(t1lvi3,NULL);
					}
					else
					{
						// vi1--vi2 form an internal edge, and will make an internal face
						Cell* cvi1 = o->getAssociatedCell(L[vi1]);
						Cell* cvi2 = o->getAssociatedCell(L[vi2]);
						assert(cvi1);
						assert(cvi2);
						assert(o->edge(cvi1,cvi2)==NULL);

						// let za = min(vi1,vi2), and zb = max
						// if we haven't see (za,zb) before then create an entry (za,zb)->(t0,t1,vi3)
						// if we have seen (za,zb) then
						//   update the neighbourhoods of t0,t1 and those stored in the map(za,zb)
						//   add the new internal faces to the mesh and edges
						int za = std::min(vi1,vi2), zb = std::max(vi1,vi2);

						// XXX: don't need a hashmap for int->foo mapping...!
						if (mTetMap.find(za*Lsize + zb) == mTetMap.end())
						{
							mTetMap[za*Lsize+zb] = boost::make_tuple(t0,t1,vi3);
						}
						else
						{
							boost::tuple<Tetra*,Tetra*,int> res = mTetMap[za*Lsize+zb];
							Tetra *s0 = res.get<0>(), *s1 = res.get<1>();

							// s0 --s0n-neighbour--> t0,
							// where s0n = s0->vIndex(L[vi3])

							Vertex* vOther = L[res.get<2>()];

							int s0vo = s0->vIndex(vOther);
							int s1vo = s1->vIndex(vOther);
							int t0lvi3 = t0->vIndex(L[vi3]);
							int t1lvi3 = t1->vIndex(L[vi3]);

							s0->setNeighbour(s0vo,t0);
							s1->setNeighbour(s1vo,t1);
							t0->setNeighbour(t0lvi3,s0);
							t1->setNeighbour(t1lvi3,s1);

							// NO INTERNAL FACES needed
							// o->mesh()->addFace(new Face(L[vi1],L[vi2],&t->v(iv0)));
							// o->mesh()->addFace(new Face(L[vi1],L[vi2],&t->v(iv1)));
							o->mesh()->addEdge(new Edge(L[vi1],L[vi2],o->getAssociatedCell(L[vi1])->r() + o->getAssociatedCell(L[vi2])->r()));

							L[vi1]->addNeighbour(L[vi2]);
							L[vi2]->addNeighbour(L[vi1]);
						}
					}
				}//end edge enumeration



			}// end Lfaces iteration

			// finish this case by deleting U, internal faces, and vertex-neighbour links
			for(unsigned int i=0;i<X1.size();i++)
			{
				Tetra* ut = X1[i];
				o->mesh()->removeTetra(ut);
				delete ut;
			}

			// finally delete edge iv0-->iv1
			Edge* e = getEdge(&t->v(iv0),&t->v(iv1));
			assert(e!=NULL);
			o->mesh()->removeEdge(e);
			delete e;

			t->v(iv0).removeNeighbour(&t->v(iv1));
			t->v(iv1).removeNeighbour(&t->v(iv0));

			// finally delete t
			o->mesh()->removeTetra(t);
			delete t;

			 */

		} // end Ucount==X1.size()
		else // there is a gap in U!
		{
			LOG("gap\n");

			// XXX: The following "gap-fill" method is not that great, but it'll do for now

			// The algorithm proceeds the same as the non-gap case, but we add an extra tetrahedra to fill
			// in the gap, this tet will have surface faces, and will cover the surface faces of the gap

			// first remove the surface faces of X1 and X2

			Face* fX1 = NULL;
			if (X1.size()>0)
			{
				Tetra* sfX1 = X1[X1.size()-1];
				int iX1[3];
				sfX1->oppositeFaceIndices(V1[V1.size()-1],iX1[0],iX1[1],iX1[2]);
				Vertex* vX1[3] = {&sfX1->v(iX1[0]), &sfX1->v(iX1[1]), &sfX1->v(iX1[2])};
				fX1 = o->mesh()->getSurfaceFace(vX1[0],vX1[1],vX1[2]);
			}
			else // fX1 = the outer face of t
			{
				fX1 = o->mesh()->getSurfaceFace(&t->v(iv0),&t->v(iv1),&t->v(iv3));
			}
			if (fX1==NULL)
			{
				error("fX1==NULL");
				return;
			}

			for(int fi=0;fi<3;fi++)
				fX1->v(fi).removeFaceNeighbour(fX1);
			o->mesh()->removeFace(fX1);


			Face* fX2 = NULL;
			if (X2.size() > 0)
			{
				Tetra* sfX2 = X2[X2.size()-1];
				int iX2[3];
				sfX2->oppositeFaceIndices(V2[V2.size()-1],iX2[0],iX2[1],iX2[2]);
				Vertex* vX2[3] = {&sfX2->v(iX2[0]), &sfX2->v(iX2[1]), &sfX2->v(iX2[2])};
				fX2 = o->mesh()->getSurfaceFace(vX2[0],vX2[1],vX2[2]);
			}
			else
			{
				fX2 = o->mesh()->getSurfaceFace(&t->v(iv0),&t->v(iv1),&t->v(iv2));
			}

			if (fX2==NULL)
			{
				error("fX2 == NULL");
				return;
			}

			for(int fi=0;fi<3;fi++)
				fX2->v(fi).removeFaceNeighbour(fX2);
			o->mesh()->removeFace(fX2);

			// we build L , using both halves,
			// note that X2 needs to be iterated backwards
			// and we are missing one important vertex at the end of X2

			std::vector<Vertex*> L;
			if (X1.size()==0)
			{
				L.push_back(&t->v(iv3));
			}
			else
			{
				for(unsigned int i=0;i<X1.size();i++)
				{
					L.push_back(&X1[i]->v(V1[i]));
				}
			}

			// add the vertices adjacent to the gap

			if (X1.size() > 0)
			{
				Tetra* tTemp = X1[X1.size()-1];
				Vertex* gapVert = &tTemp->v(6 - (tTemp->vIndex(&X1[X1.size()-1]->v(V1[X1.size()-1])) + tTemp->vIndex(&t->v(iv0)) + tTemp->vIndex(&t->v(iv1))));
				if (gapVert==NULL)
				{
					error("gapVert == NULL");
					return;
				}

				L.push_back(gapVert);
			}

			if (X2.size() > 0)
			{
				Tetra* tTemp = X2[X2.size()-1];
				Vertex* gapVert = &tTemp->v(6 - (tTemp->vIndex(&X2[X2.size()-1]->v(V2[X2.size()-1])) + tTemp->vIndex(&t->v(iv0)) + tTemp->vIndex(&t->v(iv1))));
				if (gapVert==NULL)
				{
					error("gapVert == NULL");
					return;
				}
				L.push_back(gapVert);
			}

			if (X2.size()==0)
			{
				L.push_back(&t->v(iv2));
			}
			else
			{
				// for(unsigned int i=X2.size()-1;i>=0;i--) XXX: HAHA! this loop would never end because the int is unsigned..!
				for(int i=(int)(X2.size()-1);i>=0;i--)
				{
					L.push_back(&X2[i]->v(V2[i]));
				}
			}
			unsigned int Lsize = L.size();

			LOG("X1: ");
			BOOST_FOREACH(Tetra* t, X1){ LOG(t << " "); }
			LOG("\nX2: ");
			BOOST_FOREACH(Tetra* t, X2){ LOG(t << " "); }
			LOG("\n");


			// *****************
			// we need to handle the case where Lsize==2 in a special manner
			if (Lsize <= 1)
			{
				error("Lsize <= 1");
				return;
			}
			if (Lsize > 2)
			{
				// triangulate L, using same routine as non-gap

				/*
				Vertex *u1 = &t->v(iv0), *u2 = &t->v(iv1);
				Vertex *v1 = &t->v(iv3), *v2 = &t->v(iv2);
				Vector3d uaxis = (u1->x() - u2->x()).normalise();
				Vector3d vdaxis = v1->x() - v2->x();
				Vector3d waxis = cross(uaxis,vdaxis).normalise();
				Vector3d vaxis = cross(uaxis,waxis);

				Matrix3d uvw(uaxis,vaxis,waxis);
				Matrix3d uvwi = uvw.inv();
				std::vector<Vector3d> Lprojected;
				BOOST_FOREACH(Vertex* lv, L)
				{
					Vector3d lvproj = uvwi * lv->x();
					Lprojected.push_back(Vector3d(lvproj[1],lvproj[2],0));

					std::ostringstream oss;
					oss << "lv " << lv;
					properties.add(oss.str(), lv);
				}
				 */

				// perspective projection of points of L to a plane at u2 with normal n=u2-u1
				Vertex *u1 = &t->v(iv0), *u2 = &t->v(iv1);
				Vertex *v1 = &t->v(iv3), *v2 = &t->v(iv2);

				/*
				std::vector<Vector3d> Lprojected;
				projectPoints(L,u1,u2,v1,v2,Lprojected);

				std::vector<Vector3i> Lfaces = Math::triangulate(Lprojected);
				 */
				std::vector<Vector3i> Lfaces;
				findTriangulation(L,u1,u2,v1,v2,Lfaces);


				std::map<int,boost::tuple<Tetra*,Tetra*,int> > mTetMap;

				// for each face in Lfaces
				// ...
				BOOST_FOREACH(Vector3i faceIndices, Lfaces)
				{
					boost::tuple<Vertex*,Vertex*,Vertex*> fv(L[faceIndices[0]],L[faceIndices[1]],L[faceIndices[2]]);
					//Face* face = new Face(fv.get<0>(),fv.get<1>(),fv.get<2>());

					// add face internally to mesh
					// NO INTERNAL FACE NEEDED
					// o->mesh()->addFace(face);

					// create two tetras that share this face
					Vertex *v0 = &t->v(iv0), *v1 = &t->v(iv1), *va = fv.get<0>(), *vb = fv.get<1>(), *vc = fv.get<2>();

					LOG("v0v1vavbvc: " << v0 << v1 << va << vb << vc << "\n");

					Tetra* t0 = new Tetra(v0,va,vb,vc,0);
					Tetra* t1 = new Tetra(v1,va,vc,vb,0);

					// assert that v0 and v1 are on either sides of the face, i.e., that t0 and t1 have positive volumes
					if (t0->volume()<0 || t1->volume()<0)
					{
						error("expected tetrahedra have negative volume");
						return;
					}

					// set their rest volumes appropriately
					Cell* c0 = o->getAssociatedCell(v0);
					Cell* c1 = o->getAssociatedCell(v1);
					Cell* ca = o->getAssociatedCell(va);
					Cell* cb = o->getAssociatedCell(vb);
					Cell* cc = o->getAssociatedCell(vc);

					t0->setRest(Transform::tetrahedraRestVolume(c0,ca,cb,cc));
					t1->setRest(Transform::tetrahedraRestVolume(c1,ca,cb,cc));

					// debug: output
					std::ostringstream oss;
					oss << "tetra " << t0;
					properties.add(oss.str(),t0);
					oss.str("");
					oss << "tetra " << t1;
					properties.add(oss.str(),t1);

					// set face neighbours
					t0->setNeighbour(0,t1);
					t1->setNeighbour(0,t0);

					// update surface face neighbourhood

					// we have: va,vb,vc is clockwise from (v0 p.o.v)
					// for any pair of va,vb,vc:
					//   if the pair is adjacent (i.e., an edge joins them)
					//     then they are either on the hull surface, or are part of the edge iv2,iv3
					//     if they are on the hull surface then identify the corresponding face, and update the neighbourhood
					//     if they are part of iv2,iv3 then update the neighbourhood of D0[0], or D1[0]
					//   if they are not adjacent,
					//     then create a new internal face in internalFaces
					//     the neighbours of which are updated in the next loop

					for(unsigned int k1=0; k1<3; k1++)
					{
						unsigned int k2 = (k1+1)%3;
						unsigned int k3 = (k1+2)%3;

						// check vertices vk1,vk2
						unsigned int vi1 = faceIndices[k1], vi2 = faceIndices[k2], vi3 = faceIndices[k3];

						LOG("(vi1,vi2) = " << vi1 << "," << vi2 << ": ");
						if (vi1==(Lsize-1) and vi2==0)
						{
							LOG("base edge");

							// then it is part of edge iv2,iv3
							if (not(L[vi1]==&t->v(iv2) && L[vi2]==&t->v(iv3)))
							{
								error("discrepancy between L list and tetrahedra verts");
								return;
							}

							// update the neighbourhood of D0[0] and D1[0]
							if (not Y1.empty())
							{
								Tetra* D0 = Y1[0];
								int viD0 = 6 - (D0->vIndex(L[vi1]) + D0->vIndex(L[vi2]) + D0->vIndex(&t->v(iv1))); // compute the index of the opposite vertex
								D0->setNeighbour(viD0,t1);
								t1->setNeighbour(6 - (t1->vIndex(L[vi1]) + t1->vIndex(L[vi2]) + t1->vIndex(&t->v(iv1))),D0);
							}

							if (not Y2.empty())
							{
								Tetra* D1 = Y2[0];
								int viD1 = 6 - (D1->vIndex(L[vi1]) + D1->vIndex(L[vi2]) + D1->vIndex(&t->v(iv0))); // compute the index of the opposite vertex
								D1->setNeighbour(viD1,t0);
								t0->setNeighbour(6 - (t0->vIndex(L[vi1]) + t0->vIndex(L[vi2]) + t0->vIndex(&t->v(iv0))),D1);
							}
						}
						else if ((vi1+1)%Lsize == vi2) // and not the special edge n->0
						{
							LOG("boundary edge\n");

							// then vi1--vi2 is on the boundary
							// BUT because there is a gap there, this may be a new edge
							Edge* e = o->edge(o->getAssociatedCell(L[vi1]),o->getAssociatedCell(L[vi2]));
							if (e==NULL) // so create a new edge
							{
								LOG("new edge\n");

								e = new Edge(L[vi1],L[vi2],o->getAssociatedCell(L[vi1])->r() + o->getAssociatedCell(L[vi2])->r());
								o->mesh()->addEdge(e);
								L[vi1]->addNeighbour(L[vi2]);
								L[vi2]->addNeighbour(L[vi1]);

								// t0n, and t1n have surface faces now
								t0->setNeighbour(t0->vIndex(L[vi3]),NULL);
								t1->setNeighbour(t1->vIndex(L[vi3]),NULL);

								// add surface faces
								Face *f0, *f1;
								o->mesh()->addOuterFace(f0 = new Face(L[vi2],L[vi1],&t->v(iv0)));
								o->mesh()->addOuterFace(f1 = new Face(L[vi1],L[vi2],&t->v(iv1)));
								for(int fi=0;fi<3;fi++)
								{
									f0->v(fi).addFaceNeighbour(f0);
									f1->v(fi).addFaceNeighbour(f1);
								}

							}
							else // edge exists already
							{
								LOG("existing edge\n");

								// for each surface face, we can get the neighbouring tetrahedra on the outer of the hull
								// we can calculate this easy enough

								Tetra *t1n = NULL, *t0n = NULL;
								Tetra *told = NULL;
								if (vi1 < X1.size())
								{
									told = X1[vi1];

									t1n = X1[vi1]->neighbour(X1[vi1]->vIndex(&t->v(iv0)));
									t0n = X1[vi1]->neighbour(X1[vi1]->vIndex(&t->v(iv1)));
								}
								else
								{
									// then it is part of edge iv2,iv3
									if (not (
											(X2.size() > (XCount-vi1)) &&
											(X2[XCount-vi1]->vIndex(&t->v(iv0))!=-1) &&
											(X2[XCount-vi1]->vIndex(&t->v(iv1))!=-1)
										))
									{
										error("problem with X2");
										return;
									}

									told = X2[XCount-vi1];

									t1n = X2[XCount-vi1]->neighbour(X2[XCount-vi1]->vIndex(&t->v(iv0)));
									t0n = X2[XCount-vi1]->neighbour(X2[XCount-vi1]->vIndex(&t->v(iv1)));
								}

								LOG("(t0n,t1n) = " << t0n << "," << t1n << "\n");

								if (not ((t0->vIndex(L[vi3])!=-1) && t1->vIndex(L[vi3])!=-1))
								{
									error("Problem with t0->vIndex or t1->vIndex");
									return;
								}

								if (t0n)
								{
									t0->setNeighbour(t0->vIndex(L[vi3]),t0n);
									// assert(t0n->getNeighbourIndex(t)!=-1);
									// t0n->replaceNeighbour(t,t0);
									if (not (t0n->getNeighbourIndex(told)!=-1))
									{
										error("t0n tOld neighbour invalid");
										return;
									}
									t0n->replaceNeighbour(told,t0);
								}
								else // auto sets to outer
									t0->setNeighbour(t0->vIndex(L[vi3]),NULL);

								if (t1n)
								{
									t1->setNeighbour(t1->vIndex(L[vi3]),t1n);
									//assert(t1n->getNeighbourIndex(t)!=-1);
									//t1n->replaceNeighbour(t,t1);
									if (not (t1n->getNeighbourIndex(told)!=-1))
									{
										error("t1n tOld neighbour invalid");
										return;
									}
									t1n->replaceNeighbour(told,t1);
								}
								else // auto sets to outer
									t1->setNeighbour(t1->vIndex(L[vi3]),NULL);
							}
						}
						else
						{
							LOG("internal edge\n");

							// vi1--vi2 form an internal edge, and will make an internal face

							Cell* cvi1 = o->getAssociatedCell(L[vi1]);
							Cell* cvi2 = o->getAssociatedCell(L[vi2]);

							if (cvi1==NULL || cvi2==NULL || o->edge(cvi1,cvi2)!=NULL)
							{
								error("error with internal edge");
								return;
							}

							// let za = min(vi1,vi2), and zb = max
							// if we haven't see (za,zb) before then create an entry (za,zb)->(t0,t1,vi3)
							// if we have seen (za,zb) then
							//   update the neighbourhoods of t0,t1 and those stored in the map(za,zb)
							//   add the new internal faces to the mesh and edges
							int za = std::min(vi1,vi2), zb = std::max(vi1,vi2);

							// XXX: don't need a hashmap for int->foo mapping...!
							if (mTetMap.find(za*Lsize + zb) == mTetMap.end())
							{
								mTetMap[za*Lsize+zb] = boost::make_tuple(t0,t1,vi3);
							}
							else
							{
								boost::tuple<Tetra*,Tetra*,int> res = mTetMap[za*Lsize+zb];
								Tetra *s0 = res.get<0>(), *s1 = res.get<1>();

								// s0 --s0n-neighbour--> t0,
								// where s0n = s0->vIndex(L[vi3])

								Vertex* vOther = L[res.get<2>()];
								s0->setNeighbour(s0->vIndex(vOther),t0);
								s1->setNeighbour(s1->vIndex(vOther),t1);
								t0->setNeighbour(t0->vIndex(L[vi3]),s0);
								t1->setNeighbour(t1->vIndex(L[vi3]),s1);

								// NO INTERNAL FACES needed
								// o->mesh()->addFace(new Face(L[vi1],L[vi2],&t->v(iv0)));
								// o->mesh()->addFace(new Face(L[vi1],L[vi2],&t->v(iv1)));
								o->mesh()->addEdge(new Edge(L[vi1],L[vi2],o->getAssociatedCell(L[vi1])->r() + o->getAssociatedCell(L[vi2])->r()));

								L[vi1]->addNeighbour(L[vi2]);
								L[vi2]->addNeighbour(L[vi1]);
							}
						}
					}//end edge enumeration

					// add t0 and t1 to organism
					o->mesh()->addTetra(t0);
					o->mesh()->addTetra(t1);

				}// end Lfaces iteration

				// finish this case by deleting U, internal faces, and vertex-neighbour links
				BOOST_FOREACH(Tetra* ut, X1)
				{
					o->mesh()->removeTetra(ut);
					delete ut;
				}
				BOOST_FOREACH(Tetra* ut, X2)
				{
					o->mesh()->removeTetra(ut);
					delete ut;
				}

				/*
				 * also delete faces of t
				 * BUT NO INTERNAL FACES
				Face* f = getFace(&t->v(iv0),&t->v(iv1),L[X1.size()]);
				assert(f!=NULL);
				o->mesh()->removeFace(f);
				delete f;
				 */

				// finally delete edge iv0-->iv1
				Edge* e = o->mesh()->getEdge(&t->v(iv0),&t->v(iv1));
				if (e==NULL)
				{
					error("Want an edge, got NULL");
					return;
				}
				o->mesh()->removeEdge(e);
				delete e;

				t->v(iv0).removeNeighbour(&t->v(iv1));
				t->v(iv1).removeNeighbour(&t->v(iv0));

				// finally delete t
				o->mesh()->removeTetra(t);
				delete t;
			}
			else // Lsize == 2 , so we have a very special case...
			{
				// first assert that D0 and D1 both exist
				// XXX: we need to handle the case for which this assertion fails
				//      probably just ignore the move altogether...
				if (not ((not Y1.empty()) and (not Y2.empty())))
				{
					error("Y1 or Y2 empty");
					return;
				}

				// delete the surface faces of t
				// NOTE: they were already deleted above

				// add the new surface faces
				// v0,v2,v3 v1,v2,v3
				// make sure the order is correct!!!
				int nf[2][3] = {{iv1,iv3,iv2},{iv0,iv2,iv3}};
				for(int fi=0;fi<2;fi++)
				{
					Vertex* fiv[3];
					for(int ii=0;ii<3;ii++)
						fiv[ii] = &t->v(nf[fi][ii]);

					Face* f = new Face(fiv[0],fiv[1],fiv[2]);
					o->mesh()->addOuterFace(f);
					fiv[0]->addFaceNeighbour(f);
					fiv[1]->addFaceNeighbour(f);
					fiv[2]->addFaceNeighbour(f);
				}

				// finally delete edge iv0-->iv1
				Edge* e = o->mesh()->getEdge(&t->v(iv0),&t->v(iv1));
				if (e==NULL)
				{
					error("Expected edge, got NULL");
					return;
				}
				o->mesh()->removeEdge(e);
				delete e;

				// delete neighbour links to t
				t->neighbour(iv0)->replaceNeighbour(t,NULL);
				t->neighbour(iv1)->replaceNeighbour(t,NULL);

				t->v(iv0).removeNeighbour(&t->v(iv1));
				t->v(iv1).removeNeighbour(&t->v(iv0));

				// finally delete t
				o->mesh()->removeTetra(t);
				delete t;
			}
		}

		substep = "finish";
	}

	finaliseProperties();
	return;
}
