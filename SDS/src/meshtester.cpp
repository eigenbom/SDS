#include "meshtester.h"

#include "vertex.h"
#include "edge.h"
#include "face.h"
#include "aabb.h"

#include "vector3.h"
#include "vmath.h"
#include "hmath.h"
#include "random.h"

#include <boost/foreach.hpp>
#include <vector>
#include <list>
#include <fstream>

Mesh* MeshTester::CreateTestMesh(std::string name)
{
	Mesh* m = NULL;

	if (name=="edge")
			m = CreateEdgeTestMesh();
		else if (name=="face")
			m = CreateTriTestMesh();
		else if (name=="damp")
			m = CreateDampingTestMesh();
		else if (name=="dampb")
			m = CreateDampingTestMeshB();
		else if (name=="1tet")
			m = CreateUnitEdgeTetra();
		else if (name=="cube")
			m = cube(3,3,3,1);
		else if (name=="cubecol" or name=="cubecol1")
			m = CreateCubeColTest();
		else if (name=="cubecol2")
			m = CreateCubeColTest2();
		else if (name=="cubecol3")
			m = CreateCubeColTest3();
		else if (name=="cubecol4")
			m = CreateCubeColTest4();
		else if (name=="cubecol5")
			m = CreateCubeColTest5();
		else if (name=="cubecol6")
			m = CreateCubeColTest6();

	if (m==NULL) return NULL;

	// set all outer faces
	// XXX: hack (should be doing this when we build the mesh)
	BOOST_FOREACH(Face* f, m->mOuterFaces)
	{
		f->setOuter(true);
	}
	return m;
}

Mesh* MeshTester::TestStreamer(std::string name, std::string binfile)
{
	Mesh* m = CreateTestMesh(name);

	std::ofstream ostr(binfile.c_str(), std::ios_base::binary);
	m->bWrite(ostr);
	ostr.close();

	delete m;

	m = new Mesh;
	std::ifstream istr(binfile.c_str(), std::ios_base::binary);
	m->bRead(istr);
	istr.close();

	return m;
}

Mesh* MeshTester::CreateEdgeTestMesh()
{
	Mesh* m = new Mesh;

	Vertex *a = new Vertex(1,1,1);
	Vertex* b = new Vertex(0,0,0);

	m->mVertices.push_back(a);
	m->mVertices.push_back(b);

	m->mEdges.push_back(new Edge(a,b,.5));

	m->mAABB = AABB(0,0,0,1,1,1);

	return m;
}

// XXX: Kaput -- add aabb
Mesh* MeshTester::CreateTriTestMesh()
{
	Mesh* m = new Mesh;
	Vertex* a = new Vertex(0,1,0);
	Vertex* b = new Vertex(Math::rotateZ(a->x(),4*M_PI/3));
	Vertex* c = new Vertex(Math::rotateZ(a->x(),2*M_PI/3));

	m->mVertices.push_back(a);
	m->mVertices.push_back(b);
	m->mVertices.push_back(c);

	m->mEdges.push_back(new Edge(a,b));
	m->mEdges.push_back(new Edge(b,c));
	m->mEdges.push_back(new Edge(c,a));

	Face* f;
	m->mFaces.push_back(f = new Face(a,b,c));
	// f->rest(1);

	return m;
}

// XXX: Kaput -- add aabb
Mesh* MeshTester::CreateDampingTestMesh()
{
	Mesh* m = new Mesh;
	Vertex *a = new Vertex(0,1,0),
				 *b = new Vertex(1,0,0),
				 *c = new Vertex(0,0,0);

	m->mVertices.push_back(a);
	m->mVertices.push_back(b);
	m->mVertices.push_back(c);

	Edge* e;
	(e=new Edge(a,b))->setRest(1);
	m->mEdges.push_back(e);
	(e=new Edge(b,c))->setRest(1);
	m->mEdges.push_back(e);
	(e=new Edge(c,a))->setRest(1);
	m->mEdges.push_back(e);

	return m;
}

// XXX: Kaput -- add aabb
Mesh* MeshTester::CreateDampingTestMeshB()
{
	Mesh* m = new Mesh;
	Vertex *a = new Vertex(0,1,0),
				 *b = new Vertex(1,0,0),
				 *c = new Vertex(0,0,.1);

	m->mVertices.push_back(a);
	m->mVertices.push_back(b);
	m->mVertices.push_back(c);

	Edge* e;
	(e=new Edge(a,b))->setRest(.8);
	m->mEdges.push_back(e);
	(e=new Edge(b,c))->setRest(.8);
	m->mEdges.push_back(e);
	(e=new Edge(c,a))->setRest(.8);
	m->mEdges.push_back(e);

	return m;
}

void MeshTester::addSingleTetra(Vector3d a, Vector3d b, Vector3d c, Vector3d d, Mesh* m)
{
	Vertex* v[4] =
	{
		new Vertex(a),
		new Vertex(b),
		new Vertex(c),
		new Vertex(d)
	};

	for(int i=0;i<4;++i)
		v[i]->setSurface(true);

	Tetra* t = new Tetra(v[0],v[1],v[2],v[3]);

	Edge* e[6] = {
		new Edge(v[0],v[1]),
		new Edge(v[0],v[2]),
		new Edge(v[0],v[3]),
		new Edge(v[1],v[2]),
		new Edge(v[1],v[3]),
		new Edge(v[2],v[3]),};

	Face* f[4] = {
		new Face(v[0],v[2],v[1]),
		new Face(v[0],v[1],v[3]),
		new Face(v[1],v[2],v[3]),
		new Face(v[0],v[3],v[2])};

	// face neighbours
	int fn[4][3] = {{0,1,3},{0,1,2},{0,2,3},{1,2,3}};

	// setup neighbours
	for(int i=0;i<4;++i)
	{
		for(int j=1;j<4;++j)
		{
			v[i]->mNeighbours.push_back(v[(i+j)%4]);
		}

		for(int j=0;j<3;++j)
			v[i]->mFaceNeighbours.push_back(f[fn[i][j]]);
	}

	m->addVertex(v[0]);
	m->addVertex(v[1]);
	m->addVertex(v[2]);
	m->addVertex(v[3]);

	/*
	m->mVertices.push_back(v[0]);
	m->mVertices.push_back(v[1]);
	m->mVertices.push_back(v[2]);
	m->mVertices.push_back(v[3]);
	*/

	for(int i=0;i<6;++i)
	{
		m->addEdge(e[i]);
		// m->mEdges.push_back(e[i]);
	}

	for(int i=0;i<4;++i)
	{
		m->addOuterFace(f[i]);

		//m->mFaces.push_back(f[i]);
		//m->mOuterFaces.push_back(f[i]);
	}

	m->addTetra(t);
	//m->mTetras.push_back(t);
}

void MeshTester::addYReflectedTetra(Vector3d a, Vector3d b, Vector3d c, Vector3d d, Mesh* m)
{
	Vertex* v[5] =
	{
		new Vertex(a),
		new Vertex(b),
		new Vertex(c),
		new Vertex(d),
		new Vertex(d.x(),-d.y(),d.z())
	};

	for(int i=0;i<5;++i)
		v[i]->setSurface(true);

	Tetra* t = new Tetra(v[0],v[1],v[2],v[3]);
	Tetra* t2 = new Tetra(v[0],v[2],v[1],v[4]);

	Edge* e[9] = {
		new Edge(v[0],v[1]),
		new Edge(v[0],v[2]),
		new Edge(v[0],v[3]),
		new Edge(v[0],v[4]),
		new Edge(v[1],v[2]),
		new Edge(v[1],v[3]),
		new Edge(v[1],v[4]),
		new Edge(v[2],v[3]),
		new Edge(v[2],v[4])
	};

	Face* f[7] = {
		new Face(v[0],v[1],v[2]),
		new Face(v[0],v[3],v[1]),
		new Face(v[2],v[1],v[3]),
		new Face(v[3],v[0],v[2]),
		new Face(v[0],v[1],v[4]),
		new Face(v[2],v[4],v[1]),
		new Face(v[2],v[0],v[4])};


	// outer face neighbours
	int fn[5][10] =  // fn[i][0] = n neighbours, fn[i][j], j=1..n
	{
		{4,1,3,4,6},
		{4,1,2,4,5},
		{4,0,2,5,6},
		{3,1,2,3},
		{3,4,5,6}
	};

	// setup neighbours
	for(int i=0;i<4;++i)
	{
		if (i<3)
		{
			for(int j=1;j<5;++j)
				v[i]->mNeighbours.push_back(v[(i+j)%5]);
		}
		else
		{
			for(int j=0;j<3;++j)
				v[i]->mNeighbours.push_back(v[j]);
		}

		for(int j=1;j<=fn[i][0];++j)
			v[i]->mFaceNeighbours.push_back(f[fn[i][j+1]]);
	}

	m->mVertices.push_back(v[0]);
	m->mVertices.push_back(v[1]);
	m->mVertices.push_back(v[2]);
	m->mVertices.push_back(v[3]);
	m->mVertices.push_back(v[4]);

	for(int i=0;i<9;++i)
		m->mEdges.push_back(e[i]);

	for(int i=0;i<7;++i)
	{
		m->mFaces.push_back(f[i]);
		if (i!=0)
			m->mOuterFaces.push_back(f[i]);
	}

	m->mTetras.push_back(t);
	m->mTetras.push_back(t2);
}

/*
Mesh* MeshTester::singleCube()
{
	Mesh* m = new Mesh;

	Vector3d p[] = {
		Vector3d(0,0,0),
		Vector3d(0,0,1),
		Vector3d(0,1,0),
		Vector3d(0,1,1),
		Vector3d(1,0,0),
		Vector3d(1,0,1),
		Vector3d(1,1,0),
		Vector3d(1,1,1)};

	Vertex* v[8];
	for(int i=0;i<8;++i)
		v[i] = new Vertex(p[i]);

	// split this cube into five tetrahedra
	Tetra* t[5];

	t[0] = new Tetra(v[0],v[2],v[4],v[1]);
	t[1] = new Tetra(v[4],v[7],v[5],v[1]);
	t[2] = new Tetra(v[4],v[2],v[6],v[7]);
	t[3] = new Tetra(v[2],v[3],v[7],v[1]);
	t[4] = new Tetra(v[2],v[4],v[1],v[7]);

	// and the edges
	Edge* e[18];

	// first the 12 original cube edges
	int k = 0;
	for(int dim=0;dim<3;++dim)
	{
		int ii = (dim+1) % 3;
		int jj = (dim+2) % 3;

		int twopdim = 1 << dim;
		int twopii = 1 << ii;
		int twopjj = 1 << jj;

		for(int i=0;i<2;++i)
			for(int j=0;j<2;++j)
			{
				int startfrom = i*twopii + j*twopjj;
				int endat = startfrom + twopdim;
				e[k] = new Edge(v[startfrom],v[endat]);

				++k;
			}
	}

	// the remaining six can be enumerated...
	e[k++] = new Edge(v[0],v[6]);
	e[k++] = new Edge(v[0],v[5]);
	e[k++] = new Edge(v[0],v[3]);
	e[k++] = new Edge(v[7],v[4]);
	e[k++] = new Edge(v[7],v[1]);
	e[k++] = new Edge(v[7],v[2]);

	// Now to add all the faces (careful not to duplicate them)

	Face...?!



	return m;
}
*/

inline int threetoone(int i, int j, int k, int xw, int yw, int zw)
{
	return (i*(1+yw) + j)*(1+zw) + k;
}

#define V(i,j,k) vtx[threetoone(i,j,k,xw,yw,zw)]

Mesh* MeshTester::cube(int xw, int yw, int zw, double w)
{
	Mesh* m = new Mesh;

	// setup vertices
	// std::vector<Vertex*> vtx((1+xw)*(1+yw)*(1+zw));
	typedef Vertex* PVERT;
	PVERT* vtx = new PVERT[(1+xw)*(1+yw)*(1+zw)];

	Vector3d min = Vector3d::ZERO, max = Vector3d(xw*w,yw*w,zw*w);
	m->mAABB = AABB(min,max);

	for(int i=0;i<=xw;i++)
		for(int j=0;j<=yw;j++)
			for(int k=0;k<=zw;k++)
			{
				Vertex* newvtx = new Vertex(Vector3d(i*w,j*w,k*w),0); // zero mass
				vtx[threetoone(i,j,k,xw,yw,zw)] = newvtx;
				// V(i,j,k) = newvtx;
				m->mVertices.push_back(newvtx);

				if (i==0 or i==xw or j==0 or j==yw or k==0 or k==zw)
					newvtx->setSurface(true);
			}


	std::list<Tetra*> t;

	// for each cube
	// add its tetrahedra
	// edges
	// and faces
	for(int i=0;i<xw;++i)
		for(int j=0;j<yw;++j)
			for(int k=0;k<zw;++k)
			{
				// build tetras of cube at )i,j,k(
				// flip every second cubes orientation

				bool clockwise = true;

				int ri = i, ni = i+1,
						rj = j, nj = j+1,
						rk = k, nk = k+1;

				if ((i%2)==1)
				{
					ri = i+1;
					ni = i;
					clockwise = !clockwise;
				}
				if ((j%2)==1)
				{
					rj = j+1;
					nj = j;
					clockwise = !clockwise;
				}
				if ((k%2)==1)
				{
					rk = k+1;
					nk = k;
					clockwise = !clockwise;
				}

				// outer
				if (!clockwise)
				{
					t.push_back(new Tetra(V(ri,rj,rk),V(ri,nj,rk),V(ni,rj,rk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ni,rj,rk),V(ni,nj,nk),V(ni,rj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ri,nj,nk),V(ni,nj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,nk),V(ni,nj,rk),V(ni,rj,rk)));
					// inner
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,nk),V(ni,rj,rk),V(ri,rj,nk)));
				}
				else
				{
					t.push_back(new Tetra(V(ri,rj,rk),V(ni,rj,rk),V(ri,nj,rk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ni,rj,rk),V(ni,rj,nk),V(ni,nj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,nk),V(ri,nj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,rk),V(ni,nj,nk),V(ni,rj,rk)));
					// inner
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,rj,rk),V(ni,nj,nk),V(ri,rj,nk)));
				}

				// add mass to each of those vertices
				double dm = 1/(4.0*6.0);
				V(ri,rj,rk)->addM(dm);V(ri,nj,rk)->addM(dm);V(ni,nj,rk)->addM(dm);V(ri,rj,nk)->addM(dm);
				V(ni,rj,rk)->addM(dm);V(ni,nj,nk)->addM(dm);V(ni,rj,nk)->addM(dm);V(ri,rj,nk)->addM(dm);
				V(ri,nj,rk)->addM(dm);V(ri,nj,nk)->addM(dm);V(ni,nj,nk)->addM(dm);V(ri,rj,nk)->addM(dm);
				V(ri,nj,rk)->addM(dm);V(ni,nj,nk)->addM(dm);V(ni,nj,rk)->addM(dm);V(ni,rj,rk)->addM(dm);

				// inner
				// XXX: (should this mass be equidistributed?)
				V(ri,nj,rk)->addM(2.0*dm);V(ni,nj,nk)->addM(2.0*dm);V(ni,rj,rk)->addM(2.0*dm);V(ri,rj,nk)->addM(2.0*dm);

				// add edges

				// to avoid duplicates we only add the edges on the lower cube faces
				// unless the cube is an upper cube, and then we add the appropriate edges

				// real = 0, next = 1, {a,b,c,d,e,f} implies edge from (a,b,c)->(d,e,f)

				int edges[18][6] = {
				{0,0,0,1,0,0},
				{0,0,0,0,1,0},
				{0,0,0,0,0,1},
				{0,1,0,1,0,0},
				{0,1,0,0,0,1},
				{1,0,0,0,0,1},
				{1,0,0,1,0,1},
				{1,0,1,0,0,1},
				{0,1,0,0,1,1},
				{0,1,1,0,0,1},
				{0,1,0,1,1,0},
				{1,1,0,1,0,0},
				{1,0,0,1,1,1},
				{1,1,1,0,0,1},
				{0,1,0,1,1,1},
				{1,1,1,1,0,1},
				{0,1,1,1,1,1},
				{1,1,1,1,1,0},
				};

				for(int ii=0;ii<18;++ii)
				{
					int fi = edges[ii][0]?ni:ri;
					int fj = edges[ii][1]?nj:rj;
					int fk = edges[ii][2]?nk:rk;
					int ti = edges[ii][3]?ni:ri;
					int tj = edges[ii][4]?nj:rj;
					int tk = edges[ii][5]?nk:rk;

					// check if this is a lower edge
					// if ((fi==i and ti==i) or (fj==j and tj==j) or (fk==k and tk==k))
					if ((fi==i or ti==i) and (fj==j or tj==j) and (fk==k or tk==k))
					{
						m->mEdges.push_back(new Edge(V(fi,fj,fk),V(ti,tj,tk)));
						V(fi,fj,fk)->mNeighbours.push_back(V(ti,tj,tk));
						V(ti,tj,tk)->mNeighbours.push_back(V(fi,fj,fk));
					}

					// else check all the border cases...

					if (
							// three faces
							(fi==ti and ti==xw and (fj==j or tj==j) and (fk==k or tk==k)) or
							(fj==tj and tj==yw and (fi==i or ti==i) and (fk==k or tk==k)) or
							(fk==tk and tk==zw and (fi==i or ti==i) and (fj==j or tj==j)) or
							// three remaining edges
							(fi==ti and ti==xw and fj==tj and tj==yw) or
							(fj==tj and tj==yw and fk==tk and tk==zw) or
							(fi==ti and ti==xw and fk==tk and tk==zw))
					{
						m->mEdges.push_back(new Edge(V(fi,fj,fk),V(ti,tj,tk)));
						V(fi,fj,fk)->mNeighbours.push_back(V(ti,tj,tk));
						V(ti,tj,tk)->mNeighbours.push_back(V(fi,fj,fk));
					}


					/* XXX: OOPS my logic was baad!
					if ((j==(yw-1)) and fj==(j+1) and (tj==(j+1)))
					{
						m->mEdges.push_back(new Edge(V(fi,fj,fk),V(ti,tj,tk)));
						V(fi,fj,fk)->mNeighbours.push_back(V(ti,tj,tk));
						V(ti,tj,tk)->mNeighbours.push_back(V(fi,fj,fk));
					}

					if ((k==(zw-1)) and fk==(k+1) and (tk==(k+1)))	{
						m->mEdges.push_back(new Edge(V(fi,fj,fk),V(ti,tj,tk)));
						V(fi,fj,fk)->mNeighbours.push_back(V(ti,tj,tk));
						V(ti,tj,tk)->mNeighbours.push_back(V(fi,fj,fk));
					}
					*/
				}

				/*
				m->mEdges.push_back(new Edge(V(ri,rj,rk),V(ni,rj,rk)));
				m->mEdges.push_back(new Edge(V(ri,rj,rk),V(ri,nj,rk)));
				m->mEdges.push_back(new Edge(V(ri,rj,rk),V(ri,rj,nk)));
				m->mEdges.push_back(new Edge(V(ri,nj,rk),V(ni,rj,rk)));
				m->mEdges.push_back(new Edge(V(ri,nj,rk),V(ri,rj,nk)));
				m->mEdges.push_back(new Edge(V(ni,rj,rk),V(ri,rj,nk)));
				m->mEdges.push_back(new Edge(V(ni,rj,rk),V(ni,rj,nk)));
				m->mEdges.push_back(new Edge(V(ni,rj,nk),V(ri,rj,nk)));
				m->mEdges.push_back(new Edge(V(ri,nj,rk),V(ri,nj,nk)));
				m->mEdges.push_back(new Edge(V(ri,nj,nk),V(ri,rj,nk)));
				m->mEdges.push_back(new Edge(V(ri,nj,rk),V(ni,nj,rk)));
				m->mEdges.push_back(new Edge(V(ni,nj,rk),V(ni,rj,rk)));
				m->mEdges.push_back(new Edge(V(ni,rj,rk),V(ni,nj,nk)));
				m->mEdges.push_back(new Edge(V(ni,nj,nk),V(ri,rj,nk)));
				m->mEdges.push_back(new Edge(V(ri,nj,rk),V(ni,nj,nk)));
				m->mEdges.push_back(new Edge(V(ni,nj,nk),V(ni,rj,nk)));
				m->mEdges.push_back(new Edge(V(ri,nj,nk),V(ni,nj,nk)));
				m->mEdges.push_back(new Edge(V(ni,nj,nk),V(ni,nj,rk)));

				//V(n,n,n),V(r,r,n)
				//V(n,r,r),V(r,r,r)
				//V(r,n,r),V(r,r,n)
				//V(r,n,r),V(n,n,n)
				//V(r,n,r),V(n,r,r)
				//V(n,n,n),V(n,r,r)
				*/

				// Add Cube Inner Faces

				// inner faces ...
				int inf[12][3] = {
				{ri,nj,nk},{ni,nj,nk},{ni,rj,rk},
				{ri,nj,nk},{ni,nj,nk},{ri,rj,nk},
				{ni,rj,rk},{ni,nj,nk},{ri,rj,nk},
				{ri,nj,nk},{ni,rj,rk},{ri,rj,nk},
				};

				for(int ii=0;ii<4;++ii)
				{
					Face* f;
					if (!clockwise) f = new Face(
							V(inf[ii*3][0],inf[ii*3][1],inf[ii*3][2]),
							V(inf[ii*3+1][0],inf[ii*3+1][1],inf[ii*3+1][2]),
							V(inf[ii*3+2][0],inf[ii*3+2][1],inf[ii*3+2][2]));
					else f = new Face(
							V(inf[ii*3][0],inf[ii*3][1],inf[ii*3][2]),
							V(inf[ii*3+2][0],inf[ii*3+2][1],inf[ii*3+2][2]),
							V(inf[ii*3+1][0],inf[ii*3+1][1],inf[ii*3+1][2]));

					m->mFaces.push_back(f);
				}

				// Add Cube Outer Faces
				// Add Surface Faces
				// XXX: set surface neighbours for vertices
				// outer faces

				// vtx's listed in
				// anti-clockwise
				// order
				// with ... (0 - real, 1 - next)

				int faces[3*12][3] =
				{
				// k,
				{0,0,0},{1,0,0},{0,1,0},
				{0,1,0},{1,0,0},{1,1,0},
				// j,
				{1,0,0},{0,0,0},{0,0,1},
				{1,0,0},{0,0,1},{1,0,1},
				// i,
				{0,0,1},{0,0,0},{0,1,0},
				{0,0,1},{0,1,0},{0,1,1},
				// k+1,
				{0,0,1},{1,1,1},{1,0,1},
				{1,1,1},{0,0,1},{0,1,1},
				// j+1,
				{0,1,0},{1,1,0},{1,1,1},
				{0,1,0},{1,1,1},{0,1,1},
				// i+1,
				{1,0,0},{1,0,1},{1,1,1},
				{1,0,0},{1,1,1},{1,1,0},
				};

				for(int ii=0;ii<12;++ii)
				{
					int fv[3][3]; // fv[a][b] , vertex a's b'th coord
					for(int jj=0;jj<3;++jj)
					{
						fv[jj][0] = faces[3*ii+jj][0]?ni:ri;
						fv[jj][1] = faces[3*ii+jj][1]?nj:rj;
						fv[jj][2] = faces[3*ii+jj][2]?nk:rk;
					}

					// construct lower faces
					if ((fv[0][0]==i and fv[1][0]==i and fv[2][0]==i) or
							(fv[0][1]==j and fv[1][1]==j and fv[2][1]==j) or
							(fv[0][2]==k and fv[1][2]==k and fv[2][2]==k))
					{
						Face* f;
						if (!clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);

						// if f is outer face
						// XXX: oops this logic only works if the mesh is thicker than 1 triangle!
						// some better logic follows
						// if (f->surface())
						if (
								(i==0 and fv[0][0]==i and fv[1][0]==i and fv[2][0]==i) or
								(j==0 and fv[0][1]==j and fv[1][1]==j and fv[2][1]==j) or
								(k==0 and fv[0][2]==k and fv[1][2]==k and fv[2][2]==k))
						{
							m->mOuterFaces.push_back(f);
							V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
							V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
							V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
						}
					}

					// construct the end caps
					if (
							((i==(xw-1)) and fv[0][0]==(i+1) and fv[1][0]==(i+1) and fv[2][0]==(i+1))
					or	((j==(yw-1)) and fv[0][1]==(j+1) and fv[1][1]==(j+1) and fv[2][1]==(j+1))
					or 	((k==(zw-1)) and fv[0][2]==(k+1) and fv[1][2]==(k+1) and fv[2][2]==(k+1))
					)
					{
						Face* f;
						if (!clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);
						m->mOuterFaces.push_back(f);

						V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
						V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
						V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
					}

					/*
					if ((j==(yw-1)) and fv[0][1]==(j+1) and fv[1][1]==(j+1) and fv[2][1]==(j+1))
					{
						Face* f;
						if (clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);
						m->mOuterFaces.push_back(f);

						V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
						V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
						V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
					}

					if ((k==(zw-1)) and fv[0][2]==(k+1) and fv[1][2]==(k+1) and fv[2][2]==(k+1))
					{
						Face* f;
						if (clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);
						m->mOuterFaces.push_back(f);

						V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
						V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
						V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
					}
					*/
				}
			}

	// add all tetras to mesh
	BOOST_FOREACH(Tetra* tet, t)
		m->mTetras.push_back(tet);

	delete[]vtx;
	return m;
}

Mesh* MeshTester::CreateCubeColTest()
{
	Mesh* m = new Mesh;
	m->mAABB = AABB::ZERO;

	addcube(Vector3i(3,3,3),Vector3d(0,0,0),1,m);
	addcube(Vector3i(3,3,3),Vector3d(1.1,4,2.1),1,m);

	return m;
}

Mesh* MeshTester::CreateCubeColTest2()
{
	Mesh* m = new Mesh;
	m->mAABB = AABB::ZERO;

	Vector3d diff = Vector3d(0.3,2.3,0.2);
	addcube(Vector3i(2,2,2),Vector3d::ZERO,1,m);
	addcube(Vector3i(2,2,2),diff,1,m);
	addcube(Vector3i(2,2,2),diff*2,1,m);
	addcube(Vector3i(2,2,2),diff*3,1,m);

	return m;
}

Mesh* MeshTester::CreateCubeColTest3()
{
	Mesh* m = new Mesh;
	m->mAABB = AABB::ZERO;

	int widthArray = 3;
	int numSegs = 2;
	double width = 1;
	double sep = 0.9;

	Vector3i seg = Vector3i(numSegs,numSegs,numSegs);
	double widthcube = width*numSegs; // widthcube
	double wc = widthcube+sep;

	for(int i=0;i<widthArray;++i)
	for(int j=0;j<widthArray;++j)
	for(int k=0;k<widthArray;++k)
		addcube(seg,Vector3d(i*wc+j*sep,j*wc,k*wc+j*sep),1,m);

	return m;
}

Mesh* MeshTester::CreateCubeColTest4()
{
	Mesh* m = new Mesh;
	m->mAABB = AABB::ZERO;

	Vector3d diff = Vector3d::ZERO;

	static Random random;

	for(int i=0;i<10;++i)
	{
		double dx = random.getDouble()*2 - 1;
		double dz = random.getDouble()*2 - 1;
		double dy = 2.1 + random.getDouble()*1;
		diff += Vector3d(dx,dy,dz);
		addcube(Vector3i(2,2,2),diff,1,m);
	}

	return m;
}

Mesh* MeshTester::CreateCubeColTest5()
{
	Mesh* m = new Mesh;
	m->mAABB = AABB::ZERO;

	Vector3d diff = Vector3d::ZERO;

	static Random random;

	for(int i=0;i<10;++i)
	{
		double dx = 2*(random.getDouble()*2 - 1);
		double dz = 2*(random.getDouble()*2 - 1);
		double dy = 3.1 + random.getDouble()*1;
		diff += Vector3d(dx,dy,dz);
		addcube(Vector3i(3,3,3),diff,1,m);
	}

	return m;
}

Mesh* MeshTester::CreateCubeColTest6()
{
	Mesh* m = new Mesh;
	m->mAABB = AABB::ZERO;

	Vector3d diff = Vector3d::ZERO;

	static Random random;

	for(int i=0;i<20;++i)
	{
		double dx = 5*(random.getDouble()*2 - 1);
		double dz = 5*(random.getDouble()*2 - 1);
		double dy = 6.1 + random.getDouble()*1;
		diff += Vector3d(0,dy,0);
		addcube(Vector3i(3,3,3),Vector3d(dx,0,dz)+diff,2,m);
	}

	return m;
}


/// Create a tetrahedra with unit length edges
Mesh* MeshTester::CreateUnitEdgeTetra()
{
	Mesh* m = new Mesh;

	double root3 = sqrt(3.0);

	Vector3d a = Vector3d(1/root3,0,0);
	Vector3d b = Vector3d(-.5/root3,0,-.5);
	Vector3d c = Vector3d(-.5/root3,0,.5);
	Vector3d d = Vector3d(0,sqrt(2.0)/root3,0);

	// NOTE: the correct ordering
	addSingleTetra(a,b,c,d,m);

	m->updateAABB();
	return m;
}


void MeshTester::addcube(Vector3i dim, Vector3d origin, double w, Mesh* m)
{
	int xw = dim.x(), yw = dim.y(), zw = dim.z();

	// setup vertices
	// std::vector<Vertex*> vtx((1+xw)*(1+yw)*(1+zw));
	typedef Vertex* PVERT;
	PVERT* vtx = new PVERT[(1+xw)*(1+yw)*(1+zw)];

	Vector3d min = origin, max = origin + Vector3d(xw*w,yw*w,zw*w);
	m->mAABB += AABB(min,max);

	for(int i=0;i<=xw;i++)
		for(int j=0;j<=yw;j++)
			for(int k=0;k<=zw;k++)
			{
				Vertex* newvtx = new Vertex(origin + Vector3d(i*w,j*w,k*w),0); // zero mass
				vtx[threetoone(i,j,k,xw,yw,zw)] = newvtx;
				// V(i,j,k) = newvtx;
				m->mVertices.push_back(newvtx);

				if (i==0 or i==xw or j==0 or j==yw or k==0 or k==zw)
					newvtx->setSurface(true);
			}

	std::list<Tetra*> t;

	// for each cube
	// add its tetrahedra
	// edges
	// and faces
	for(int i=0;i<xw;++i)
		for(int j=0;j<yw;++j)
			for(int k=0;k<zw;++k)
			{
				// build tetras of cube at )i,j,k(
				// flip every second cubes orientation

				bool clockwise = true;

				int ri = i, ni = i+1,
						rj = j, nj = j+1,
						rk = k, nk = k+1;

				if ((i%2)==1)
				{
					ri = i+1;
					ni = i;
					clockwise = !clockwise;
				}
				if ((j%2)==1)
				{
					rj = j+1;
					nj = j;
					clockwise = !clockwise;
				}
				if ((k%2)==1)
				{
					rk = k+1;
					nk = k;
					clockwise = !clockwise;
				}

				// outer
				if (!clockwise)
				{
					t.push_back(new Tetra(V(ri,rj,rk),V(ri,nj,rk),V(ni,rj,rk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ni,rj,rk),V(ni,nj,nk),V(ni,rj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ri,nj,nk),V(ni,nj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,nk),V(ni,nj,rk),V(ni,rj,rk)));
					// inner
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,nk),V(ni,rj,rk),V(ri,rj,nk)));
				}
				else
				{
					t.push_back(new Tetra(V(ri,rj,rk),V(ni,rj,rk),V(ri,nj,rk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ni,rj,rk),V(ni,rj,nk),V(ni,nj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,nk),V(ri,nj,nk),V(ri,rj,nk)));
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,nj,rk),V(ni,nj,nk),V(ni,rj,rk)));
					// inner
					t.push_back(new Tetra(V(ri,nj,rk),V(ni,rj,rk),V(ni,nj,nk),V(ri,rj,nk)));
				}

				// add mass to each of those vertices
				double dm = 1/(4.0*6.0);
				V(ri,rj,rk)->addM(dm);V(ri,nj,rk)->addM(dm);V(ni,nj,rk)->addM(dm);V(ri,rj,nk)->addM(dm);
				V(ni,rj,rk)->addM(dm);V(ni,nj,nk)->addM(dm);V(ni,rj,nk)->addM(dm);V(ri,rj,nk)->addM(dm);
				V(ri,nj,rk)->addM(dm);V(ri,nj,nk)->addM(dm);V(ni,nj,nk)->addM(dm);V(ri,rj,nk)->addM(dm);
				V(ri,nj,rk)->addM(dm);V(ni,nj,nk)->addM(dm);V(ni,nj,rk)->addM(dm);V(ni,rj,rk)->addM(dm);

				// inner
				// XXX: (should this mass be equidistributed?)
				V(ri,nj,rk)->addM(2.0*dm);V(ni,nj,nk)->addM(2.0*dm);V(ni,rj,rk)->addM(2.0*dm);V(ri,rj,nk)->addM(2.0*dm);

				// add edges

				// to avoid duplicates we only add the edges on the lower cube faces
				// unless the cube is an upper cube, and then we add the appropriate edges

				// real = 0, next = 1, {a,b,c,d,e,f} implies edge from (a,b,c)->(d,e,f)

				int edges[18][6] = {
				{0,0,0,1,0,0},
				{0,0,0,0,1,0},
				{0,0,0,0,0,1},
				{0,1,0,1,0,0},
				{0,1,0,0,0,1},
				{1,0,0,0,0,1},
				{1,0,0,1,0,1},
				{1,0,1,0,0,1},
				{0,1,0,0,1,1},
				{0,1,1,0,0,1},
				{0,1,0,1,1,0},
				{1,1,0,1,0,0},
				{1,0,0,1,1,1},
				{1,1,1,0,0,1},
				{0,1,0,1,1,1},
				{1,1,1,1,0,1},
				{0,1,1,1,1,1},
				{1,1,1,1,1,0},
				};

				for(int ii=0;ii<18;++ii)
				{
					int fi = edges[ii][0]?ni:ri;
					int fj = edges[ii][1]?nj:rj;
					int fk = edges[ii][2]?nk:rk;
					int ti = edges[ii][3]?ni:ri;
					int tj = edges[ii][4]?nj:rj;
					int tk = edges[ii][5]?nk:rk;

					// check if this is a lower edge
					// if ((fi==i and ti==i) or (fj==j and tj==j) or (fk==k and tk==k))
					if ((fi==i or ti==i) and (fj==j or tj==j) and (fk==k or tk==k))
					{
						m->mEdges.push_back(new Edge(V(fi,fj,fk),V(ti,tj,tk)));
						V(fi,fj,fk)->mNeighbours.push_back(V(ti,tj,tk));
						V(ti,tj,tk)->mNeighbours.push_back(V(fi,fj,fk));
					}

					// else check all the border cases...

					if (
							// three faces
							(fi==ti and ti==xw and (fj==j or tj==j) and (fk==k or tk==k)) or
							(fj==tj and tj==yw and (fi==i or ti==i) and (fk==k or tk==k)) or
							(fk==tk and tk==zw and (fi==i or ti==i) and (fj==j or tj==j)) or
							// three remaining edges
							(fi==ti and ti==xw and fj==tj and tj==yw) or
							(fj==tj and tj==yw and fk==tk and tk==zw) or
							(fi==ti and ti==xw and fk==tk and tk==zw))
					{
						m->mEdges.push_back(new Edge(V(fi,fj,fk),V(ti,tj,tk)));
						V(fi,fj,fk)->mNeighbours.push_back(V(ti,tj,tk));
						V(ti,tj,tk)->mNeighbours.push_back(V(fi,fj,fk));
					}
				}
				// Add Cube Inner Faces

				// inner faces ...
				int inf[12][3] = {
				{ri,nj,nk},{ni,nj,nk},{ni,rj,rk},
				{ri,nj,nk},{ni,nj,nk},{ri,rj,nk},
				{ni,rj,rk},{ni,nj,nk},{ri,rj,nk},
				{ri,nj,nk},{ni,rj,rk},{ri,rj,nk},
				};

				for(int ii=0;ii<4;++ii)
				{
					Face* f;
					if (!clockwise) f = new Face(
							V(inf[ii*3][0],inf[ii*3][1],inf[ii*3][2]),
							V(inf[ii*3+1][0],inf[ii*3+1][1],inf[ii*3+1][2]),
							V(inf[ii*3+2][0],inf[ii*3+2][1],inf[ii*3+2][2]));
					else f = new Face(
							V(inf[ii*3][0],inf[ii*3][1],inf[ii*3][2]),
							V(inf[ii*3+2][0],inf[ii*3+2][1],inf[ii*3+2][2]),
							V(inf[ii*3+1][0],inf[ii*3+1][1],inf[ii*3+1][2]));

					m->mFaces.push_back(f);
				}

				// Add Cube Outer Faces
				// Add Surface Faces
				// XXX: set surface neighbours for vertices
				// outer faces

				// vtx's listed in
				// anti-clockwise
				// order
				// with ... (0 - real, 1 - next)

				int faces[3*12][3] =
				{
				// k,
				{0,0,0},{1,0,0},{0,1,0},
				{0,1,0},{1,0,0},{1,1,0},
				// j,
				{1,0,0},{0,0,0},{0,0,1},
				{1,0,1},{1,0,0},{0,0,1},
				// i,
				{0,0,1},{0,0,0},{0,1,0},
				{0,0,1},{0,1,0},{0,1,1},
				// k+1,
				{0,0,1},{1,1,1},{1,0,1},
				{1,1,1},{0,0,1},{0,1,1},
				// j+1,
				{0,1,0},{1,1,0},{1,1,1},
				{0,1,0},{1,1,1},{0,1,1},
				// i+1,
				{1,0,0},{1,0,1},{1,1,1},
				{1,0,0},{1,1,1},{1,1,0},
				};

				for(int ii=0;ii<12;++ii)
				{
					int fv[3][3]; // fv[a][b] , vertex a's b'th coord
					for(int jj=0;jj<3;++jj)
					{
						fv[jj][0] = faces[3*ii+jj][0]?ni:ri;
						fv[jj][1] = faces[3*ii+jj][1]?nj:rj;
						fv[jj][2] = faces[3*ii+jj][2]?nk:rk;
					}

					// construct lower faces
					if ((fv[0][0]==i and fv[1][0]==i and fv[2][0]==i) or
							(fv[0][1]==j and fv[1][1]==j and fv[2][1]==j) or
							(fv[0][2]==k and fv[1][2]==k and fv[2][2]==k))
					{
						Face* f;
						if (!clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);

						// if f is outer face
						// XXX: oops this logic only works if the mesh is thicker than 1 triangle!
						// some better logic follows
						// if (f->surface())
						if (
								(i==0 and fv[0][0]==i and fv[1][0]==i and fv[2][0]==i) or
								(j==0 and fv[0][1]==j and fv[1][1]==j and fv[2][1]==j) or
								(k==0 and fv[0][2]==k and fv[1][2]==k and fv[2][2]==k))
						{
							m->mOuterFaces.push_back(f);
							V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
							V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
							V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
						}
					}

					// construct the end caps
					if (
							((i==(xw-1)) and fv[0][0]==(i+1) and fv[1][0]==(i+1) and fv[2][0]==(i+1))
					or	((j==(yw-1)) and fv[0][1]==(j+1) and fv[1][1]==(j+1) and fv[2][1]==(j+1))
					or 	((k==(zw-1)) and fv[0][2]==(k+1) and fv[1][2]==(k+1) and fv[2][2]==(k+1))
					)
					{
						Face* f;
						if (!clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);
						m->mOuterFaces.push_back(f);

						V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
						V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
						V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
					}

					/*
					if ((j==(yw-1)) and fv[0][1]==(j+1) and fv[1][1]==(j+1) and fv[2][1]==(j+1))
					{
						Face* f;
						if (clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);
						m->mOuterFaces.push_back(f);

						V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
						V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
						V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
					}

					if ((k==(zw-1)) and fv[0][2]==(k+1) and fv[1][2]==(k+1) and fv[2][2]==(k+1))
					{
						Face* f;
						if (clockwise) f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[1][0],fv[1][1],fv[1][2]),
								V(fv[2][0],fv[2][1],fv[2][2]));
						else f = new Face(
								V(fv[0][0],fv[0][1],fv[0][2]),
								V(fv[2][0],fv[2][1],fv[2][2]),
								V(fv[1][0],fv[1][1],fv[1][2]));
						m->mFaces.push_back(f);
						m->mOuterFaces.push_back(f);

						V(fv[0][0],fv[0][1],fv[0][2])->mFaceNeighbours.push_back(f);
						V(fv[1][0],fv[1][1],fv[1][2])->mFaceNeighbours.push_back(f);
						V(fv[2][0],fv[2][1],fv[2][2])->mFaceNeighbours.push_back(f);
					}
					*/
				}
			}

	// add all tetras to mesh
	BOOST_FOREACH(Tetra* tet, t)
		m->mTetras.push_back(tet);

	delete[]vtx;
}
