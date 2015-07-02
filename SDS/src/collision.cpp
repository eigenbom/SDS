#include "collision.h"
#include "edge.h"

#include "vector3.h"
#include "hmath.h"
#include "gmath.h"
#include "vmath.h"

#include <cmath>
#include <set>
#include <cfloat>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <boost/foreach.hpp>

#include "log.h"

inline void throwif(bool condition, std::string msg)
{
	if (condition) throw Collision::Exception(msg);
}

Collision::Exception::Exception(std::string description) throw()
{
	mDescription = std::string("Collision::Exception \"") + description + "\"";
}

const char* Collision::Exception::what() const throw(){
	return mDescription.c_str();
}

Collision::Collision()
:mCellSize(0),
 mTotalNumberOfEdges(0)
{
	mpHashTable = new std::list<Vertex*>[HASH_TABLE_SIZE]; // XXX: adjust size? dynamically?
	mpEdgeHashTable = new std::list<EInfo*>[HASH_TABLE_SIZE]; // XXX: what should the size of this hashtable be?
}

Collision::~Collision()
{
	delete[]mpHashTable;
	delete[]mpEdgeHashTable;
}

void Collision::detectCollisions()
{
	LOG(__FILE__ << __LINE__);

	// XXX: The paper mentions an efficiency gain by NOT DOING THIS
	// clean up pass
	// reset all vertex lists in the hash table
	for(int i=0;i<HASH_TABLE_SIZE;i++)
		mpHashTable[i].clear();

	// first pass
	//   hash all vertices into hashtable
	BOOST_FOREACH(Mesh* m, mMeshList)
		BOOST_FOREACH(Vertex* v, m->mVertices)
		{
			// ignore frozen verts
			if (v->isFrozen()) continue;

			Vector3d p = v->x();
			int i = (int)std::floor(p.x() / mCellSize), j = (int)std::floor(p.y() / mCellSize), k = (int)std::floor(p.z() / mCellSize);
			unsigned int hash_index = hash(i,j,k);

			mpHashTable[hash_index].push_back(v);
		}

	// second pass
	// hash tetras to hashtable, and detect intersections...

	BOOST_FOREACH(Mesh* m, mMeshList)
		BOOST_FOREACH(Tetra* t, m->mTetras)
		{
			t->setIntersected(false);

			// update AABB
			const AABB& aabb = t->updateAABB();
			const double* cpts = aabb.cpts();

			// XXX: assume valid
			throwif(not aabb.valid(),"AABB invalid");

			// get min,max Z coords...
			int min[3] = {(int)std::floor(cpts[0] / mCellSize),(int)std::floor(cpts[1]/mCellSize),(int)std::floor(cpts[2]/mCellSize)},
					max[3] = {(int)std::floor(cpts[3] / mCellSize),(int)std::floor(cpts[4]/mCellSize),(int)std::floor(cpts[5]/mCellSize)};

			// for every integer coord between min and max
			// hash it and check it for collision with the vertices at that hashed location
			for(int ix=min[0];ix<=max[0];++ix)
			for(int iy=min[1];iy<=max[1];++iy)
			for(int iz=min[2];iz<=max[2];++iz)
			{
				unsigned int hash_index = hash(ix,iy,iz);
				const std::list<Vertex*>& vlist = mpHashTable[hash_index];

				BOOST_FOREACH(const Vertex* v, vlist)
				{
					// ignore those the vertices of t
					if (t->isVertex(v)) continue;

					// check for v intersection with t

					// first check the bounding boxes...
					if (aabb.contains(v->x()))
					{
						// MAY intersect with t
						Vector3d b = t->bary(v->x());
						if (b.x() >= 0 and b.y() >= 0 and b.z() >= 0 and (b.x()+b.y()+b.z()) <= 1)
						{
							// we have an intersection

							// XXX:
							// For now, lets just highlight the tetra ...
							// Here we would like to store all the information of the collision
							// and then perform some appropriate response to it
							t->setIntersected(true);
						}

						// else no intersection
					}

					// else
					// no intersection
				}
			}
		}
}

void Collision::addMesh(Mesh* m)
{
	mAllMeshes.push_back(m);
	mMeshList.push_back(m);

	// adjust the cell size to an appropriate size
	double edgeLength = 0;
	BOOST_FOREACH(Edge* e, m->mEdges)
		edgeLength += e->rest();

	double otel = mTotalNumberOfEdges * mCellSize;
	mTotalNumberOfEdges += m->mEdges.size();

	mCellSize = (otel + edgeLength) / mTotalNumberOfEdges;
}

void Collision::addStaticMesh(Mesh* m)
{
	mAllMeshes.push_back(m);
	mStaticMeshList.push_back(m);

	// adjust the cell size to an appropriate size
	double edgeLength = 0;
	BOOST_FOREACH(Edge* e, m->mEdges)
		edgeLength += e->rest();

	double otel = mTotalNumberOfEdges * mCellSize;
	mTotalNumberOfEdges += m->mEdges.size();

	mCellSize = (otel + edgeLength) / mTotalNumberOfEdges;
}

void Collision::setWorldBounds(AABB a)
{
	mWorldBounds = a;
}

void Collision::reset()
{
	mMeshList.clear();
	mStaticMeshList.clear();
	mAllMeshes.clear();
	mTotalNumberOfEdges = 0;
	mCellSize = 0;
}

/* estimateCollisionDepthAndDirection:
 * Implements the algorithm in
 * "Consistent penetration depth estimation for deformable collision response" by
 * Heidelberger et al. 2004.
 */

#ifdef OFFLINE
	#define CDBGDRAW 0
#else
	#define CDBGDRAW 0
#endif

void Collision::estimateCollisionDepthAndDirection()
{
	LOG("Collision::estimateCollisionDepthAndDirection()\n");

	mIntersectingFaceVoxels.clear();

	// 1 point-tetra collision detection
	// classify all points as colliding or non-colliding

	// XXX: The paper mentions an efficiency gain by NOT DOING THIS
	// clean up pass
	// reset all vertex lists in the hash table
	for(int i=0;i<HASH_TABLE_SIZE;++i)
	{
		mpHashTable[i].clear();
		mpEdgeHashTable[i].clear();
	}

	// first pass
	// hash all vertices into hashtable
	// untag all verts
	BOOST_FOREACH(Mesh* m, mMeshList)
	BOOST_FOREACH(Vertex* v, m->mVertices)
	{
		// ignore frozen verts
		if (v->isFrozen()) continue;

		Vector3d p = v->x();
		int gc[3];
		toGridCoords(p,gc);
		unsigned int hash_index = hash(gc);
		mpHashTable[hash_index].push_back(v);

		v->tag(false);
	}

	// second pass
	// hash tetras to hashtable, and detect intersections...

	// XXX: Manage this memory better...!
	// XXX: Profile this
	std::list<VInfo> collidedVertInfo;
	std::set<const Vertex*> collidedVerts;

	// XXX: We have AABB for mesh, so use it to quickly cull inviable collisions
	BOOST_FOREACH(Mesh* m, mAllMeshes)
		BOOST_FOREACH(Tetra* t, m->mTetras)
		{
			t->setIntersected(false);

			// update AABB
			const AABB& aabb = t->updateAABB();
			const double* cpts = aabb.cpts();

			// XXX: assume valid
			throwif(!aabb.valid(),"AABB not valid");

			// get min,max Z coords...
			int min[3], max[3];
			toGridCoords(cpts,min);
			toGridCoords(cpts+3,max);

			// for every integer coord between min and max
			// hash it and check it for collision with the vertices at that hashed location
			for(int ix=min[0];ix<=max[0];++ix)
			for(int iy=min[1];iy<=max[1];++iy)
			for(int iz=min[2];iz<=max[2];++iz)
			{
				unsigned int hash_index = hash(ix,iy,iz);
				const std::list<Vertex*>& vlist = mpHashTable[hash_index];

				BOOST_FOREACH(Vertex* v, vlist)
				{
					// only check non collided verts
					if (collidedVerts.count(v)==1) continue;

					// ignore those the vertices of t
					if (t->isVertex(v)) continue;

					// check for v intersection with t

					// first check the bounding boxes...
					if (aabb.contains(v->x()))
					{
						// MAY intersect with t
						Vector3d b = t->bary(v->x());
						if (b.x() >= 0 and b.y() >= 0 and b.z() >= 0 and (b.x()+b.y()+b.z()) <= 1)
						{
							// we have an intersection

							// tag tetra as collided
							// create a new vertex collision struct
							t->setIntersected(true);
							VInfo vi = {v, false};
							collidedVertInfo.push_back(vi);
							collidedVerts.insert(v);
						}

						// else no intersection
					}

					// else
					// no intersection
				}
			}
		}

	// 2 intersection point calculation
	// identify "border points" and "intersecting edges"

	// XXX: Manage mem better
	mIntersectingEdges.clear();
	LOG("collidedVertInfo: " << collidedVertInfo.size() << "\n");
	BOOST_FOREACH(VInfo& vi, collidedVertInfo)
	{
		// if vi.v has a non-collided neighbour then it is a border point
		BOOST_FOREACH(Vertex* n, vi.v->neighbours())
		{
			// if n isn't a collided vert
			if (collidedVerts.count(n)==0)
			{
				vi.border = true;

				// add vi.v->n to intersecting edges
				EInfo ei = {vi.v,n,false};
				mIntersectingEdges.push_back(ei);
			}
		}
	}

	// XXX:
	// std::cerr << mIntersectingEdges.size() << "\n";

	// foreach intersecting edge e=(v1,v2):
	//  traverse from v1 to v2 using Ama87 voxel traversal
	//    classify e into corresponding voxel/hashtable

	// LOG("c" << __LINE__ << "\n");
	LOG("mIntersectingEdges: " << mIntersectingEdges.size() << "\n");
	BOOST_FOREACH(EInfo& ei, mIntersectingEdges)
	{
		Vector3d a = ei.a->x(), b = ei.b->x();
		Vector3d bma = b - a;

		// normalise into voxel space
		Vector3i ap = toGridCoords(a), bp = toGridCoords(b);
		a /= mCellSize;
		b /= mCellSize;
		bma /= mCellSize;

		std::set<unsigned int> mHashes;

		// special case
		if (ap==bp)
		{
			mpEdgeHashTable[hash(ap)].push_back(&ei);
			continue;
		}

		// perform voxel traversal from ap->bp
		// 	foreach (i,j,k) grid location
		// 		hi = hash(i,j,k)
		// 		mpEdgeHashTable[hi].push_back(ei)

		// using the quick voxel traversal algo of
		// Amanatides&Woo 1987

		// direction to step in each of the axes
		Vector3i step(
			(b.x()>a.x())?1:-1,
			(b.y()>a.y())?1:-1,
			(b.z()>a.z())?1:-1);

		// distance from pt a to nearest voxel edge
		Vector3d origin_to_voxel_edges = a - ap;

		for(int i=0;i<3;++i)
			if (step[i]==1)
				origin_to_voxel_edges[i] = 1 - origin_to_voxel_edges[i];

		//
		Vector3d bma_abs = Math::abs(bma);
		Vector3d tMax(
			origin_to_voxel_edges[0]/bma_abs.x(),
			origin_to_voxel_edges[1]/bma_abs.y(),
			origin_to_voxel_edges[2]/bma_abs.z());

		if (bma_abs.x()==0)
			tMax[0] = DBL_MAX;
		if (bma_abs.y()==0)
			tMax[1] = DBL_MAX;
		if (bma_abs.z()==0)
			tMax[2] = DBL_MAX;

		Vector3d tDelta(
			1.0/bma_abs.x(),
			1.0/bma_abs.y(),
			1.0/bma_abs.z());

		// current voxel
		int x = ap.x(), y = ap.y(), z = ap.z();
		double &tx = tMax[0], &ty = tMax[1], &tz = tMax[2];

		bool continu = true;
		do
		{
			// this edge inhabits the < x,y,z > voxel
			// add it to the hashmap
			unsigned int h = hash(x,y,z);
			if (mHashes.count(h)==0)
			{
				mHashes.insert(h);
				mpEdgeHashTable[h].push_back(&ei);
			}

			// step to the next voxel

			if (tx < ty)
			{
				if (tx < tz)
				{
					x += step[0];
					tx += tDelta[0];
				}
				else // tz <= tx < ty
				{
					z += step[2];
					tz += tDelta[2];
				}
			}
			else if (ty < tz) // ty <= tx && ty < tz
			{
				y += step[1];
				ty += tDelta[1];
			}
			else // tz <= ty <= tx
			{
				z += step[2];
				tz += tDelta[2];
			}

			// exit condition
			continu = false;
			continu |= (step.x()==1)?(x<bp.x()):(x>bp.x());
			continu |= (step.y()==1)?(y<bp.y()):(y>bp.y());
			continu |= (step.z()==1)?(z<bp.z()):(z>bp.z());
		}
		while (continu);

		// have ap!=bp, so need to add bp
		unsigned int h = hash(bp);
		if (mHashes.count(h)==0)
		{
			mHashes.insert(h);
			mpEdgeHashTable[h].push_back(&ei);
		}

		//std::list<EInfo*>& elist = mpEdgeHashTable[h];
		// TODO: Wait a sec, this isn't always true!
		//assert(std::find(elist.begin(),elist.end(),&ei)==elist.end());
		//elist.push_back(&ei);
	}
	// LOG(__LINE__ << "\n");

	// foreach outer face f:
	// 	use a box plane intersection test (Greene) to classify it into the edge hashtable
	// 	foreach voxel it intersects:
	// 		foreach edge e at the hashed pos:
	// 			test e against f for intersection by
	// 			computing Barycentric coords of intersection point
	// 			use Bcoords to interpolate a smooth surface normal at the intersect point

	BOOST_FOREACH(Mesh* m, mAllMeshes)
	BOOST_FOREACH(Face* f, m->outerFaces())
	{
		// calculate AABB for this face
		const AABB& aabb = f->aabb();

		int min[3], max[3];
		toGridCoords(aabb.min(),min);
		toGridCoords(aabb.max(),max);

		// for each voxel within the bounding box of f
		for(int i=min[0]; i<=max[0]; ++i)
		for(int j=min[1]; j<=max[1]; ++j)
		for(int k=min[2]; k<=max[2]; ++k)
		{
			// do a simple plane test to remove most non-intersecting voxels
			const Vector3d& norm = f->n();
			const Vector3d& q = f->q();

			Vector3i ni(norm.x()>0?i:i+1,norm.y()>0?j:j+1,norm.z()>0?k:k+1);
			Vector3i pi(norm.x()<=0?i:i+1,norm.y()<=0?j:j+1,norm.z()<=0?k:k+1);
			Vector3d n = fromGridCoords(ni);
			Vector3d p = fromGridCoords(pi);

			// if dot(n-q,norm) <= 0 and dot(p-q,norm) >= 0
			// then voxel (i,j,k) is likely to intersect f, so
			// do the better intersection test for each of its hashed intersecting edges...

			if (dot(n-q,norm) <= 0 and dot(p-q,norm) >= 0)
			{
				// face "may" intersect this voxel, so get the list of intersecting edges
				// and test for collision with this face against all of them

				#if CDBGDRAW
				mIntersectingFaceVoxels.push_back(fromVoxelSpaceToAABB(Vector3i(i,j,k))));
				#endif

				BOOST_FOREACH(EInfo* ei,mpEdgeHashTable[hash(i,j,k)])
				{
					// Note: The intersecting edge may intersect MORE THAN ONE face
					// in this case we will classify it's intersection point as the
					// point nearest to its non-colliding vertex
					// see NEAREST

					// ignore edges that belong to this face
					// and ignore if ei has been set and tested against this face
					if (f->contains(ei->a) or f->contains(ei->b)) continue;
					if (ei->set and ei->f==f) continue;

					// if intersect then
					// set intersection position ei->p, and interpolate face normal ei->n

					// find point in plane of edge intersection
					// double pdotn = dot(p,n), adotn = dot(ei->a->x(),n), bdotn = dot(ei->b->x(),n);
					double qdotn = dot(q,norm), adotn = dot(ei->a->x(),norm), bdotn = dot(ei->b->x(),norm);
					double t = (qdotn - adotn)/(bdotn - adotn);
					if (0 <= t and t <= 1) // then we have a plane intersection
					{
						Vector3d intersectionPt = ei->a->x()*(1-t) + ei->b->x()*t;

						// see if this point falls within the face
						// by computing its barycentric coords

						double b1,b2;
						Vector3d a = f->v(0).x(), b = f->v(1).x(), c = f->v(2).x();
						Math::baryTri(a,b,c,intersectionPt,b1,b2);

						// std::cerr << "bary: " << ei->a->x() << "\t" << ei->b->x() << "\t[" << b1 << ":" << b2 << "]\n";

						if (b1>=0 and b2>=0 and (b1+b2)<=1) // then edge intersects this face
						{
							// NEAREST
							if (ei->set==false or
									sqdist(intersectionPt,ei->b->x()) < sqdist(ei->p,ei->b->x())
									)
							{
								ei->p = intersectionPt;
								ei->set = true;
								ei->f = f;
							}
							else
								continue;

							// Interpolate face norm
							// f = outerface ==> f.v(i).surface()==true (and hence we can compute the vertex (interpolated) normals)
							Vector3d an = f->v(0).n(), bn = f->v(1).n(), cn = f->v(2).n();

							// interpolate using the distance from ei->p to a,b,c
							double p2a = dist(a,ei->p), p2b = dist(b,ei->p), p2c = dist(c,ei->p);
							double totalDist = p2a+p2b+p2c;
							double at = totalDist - p2a/totalDist, bt = totalDist - p2b/totalDist, ct = totalDist - p2c/totalDist;

							ei->n = (an*at + bn*bt + cn*ct).normalise();

						}
					}
				}
			}
		}
	}

	// let IP(p) = {v: v is an intersection point and v is adjacent to p}
	// let w(a,b) = 1/sqlength(a-b)
	// let nv = surface normal at v

	// 3 border point depth estimation
	// foreach border point p
	// 	compute d(p) = sum(v in IP(p)) (w(p,v).(v-p).nv) / sum (v in IP(p)) w(v,p)
	// 	compute r'(p) = sum(v in IP(p)) w(v,p).nv / sum(v in IP(p)) w(v,p)
	// 	compute r(p) = r'(p).norm(), where r'(p) is as above
	//  tag p as processed

	// our implementation
	// set d(p) = 0 for all p
	// set t(p) = 0 for all p
	// set r'(p) = <0,0,0>

	// TODO: SpeedUp by replacing all these maps with a single vertex*-><double,double,Vector3d> map

	// clear info structures
	mProcessedVertices.clear();
	mUnprocessedVertices.clear();
	mPenetrationInfo.clear();
	mPenetrationData.clear();

	BOOST_FOREACH(VInfo& vi, collidedVertInfo)
	{
		if (vi.border)
		{
			mPenetrationData.insert(std::pair<Vertex*,PInfo>(vi.v,PInfo(vi.v)));
			mProcessedVertices.insert(vi.v);
		}
		else
			mUnprocessedVertices.insert(vi.v);
	}

	// loop over all intersecting edges e
	// {
	// 	 d(e.a) += w(e.a,e.p) * dot(e.a-e.p,e.n)
	// 	 t(e.a) += w(e.a,e.p)
	// 	 r'(e.a) += w(e.a,p) * e.n
	// }

	BOOST_FOREACH(EInfo& ei, mIntersectingEdges)
	{
		// XXX: TODO: deal with this!
		// assert(ei.set);

		// XXX: hack , ignore bad edge data
		if (ei.set)
		{
			double sqd = sqdist(ei.a->x(),ei.p);
			if (sqd < 0.000000001)
			{
				throwif(mPenetrationData.count(ei.a)!=1,"PenetrationData.count()!=1");
				PInfo& pi = mPenetrationData[ei.a];
				pi.depth += 0;
				pi.totalDepth += 1;
				pi.direction += Vector3d(0,1,0);
				pi.f = ei.f;
			}
			else
			{
				double wep = 1/sqd;
				double dotp = dot(ei.p-ei.a->x(),ei.n);
				//assert (dotp>=0);

				throwif(mPenetrationData.count(ei.a)!=1,"PenetrationData.count()!=1");
				PInfo& pi = mPenetrationData[ei.a];
				pi.depth += wep*dotp;
				pi.totalDepth += wep;
				pi.direction += wep*ei.n;
				pi.f = ei.f;
			}
		}
	}

	// foreach (border pt p)
	// {
	//   d(p) /= t(p)
	//   r'(p) /= t(p)
	//   r(p) = r'(p).normalised()
	// }

	BOOST_FOREACH(Vertex* v, mProcessedVertices)
	{
		throwif(mPenetrationData.count(v)!=1,"PenetrationData.count != 1");
		PInfo& pi = mPenetrationData[v];
		//LOG("penetration data: " << pi.depth << "," << pi.totalDepth << "," << pi.direction << "\n");

		pi.depth /= pi.totalDepth;
		pi.direction.normaliseInPlace();
		mPenetrationInfo.push_back(pi);
	}

	// let IV(p) = {v: v is an adjacent to p and mProcessedVertices}

	// 4 info propagation
	// let P = set of (unprocessed,colliding) points
	// do the following repeatedly until |P|==0
	//  select an element e of P s.t., e's neighbours are all mProcessedVertices or non-colliding
	//  compute d(e) = sum(v in IV(e)) (w(e,v).((v-e).r(v) + d(v))) / sum(v in IV(e)) w(v,e)
	//  computer r'(e) = sum(v in IV(e)) w(e,v).r(v) / sum(v in IV(e)) w(e,v)
	//  compute r(e) = r'(e).norm()
	//  remove e from P

	while (1)
	{
		// find an unprocessed point whose neighbours are all mProcessedVertices or non-colliding
		Vertex* e = NULL;
		BOOST_FOREACH(Vertex* v, mUnprocessedVertices)
		{
			bool valid = true;

			// check if valid
			BOOST_FOREACH(Vertex* n,v->mNeighbours)
			{
				if (mUnprocessedVertices.count(n)>0)
				{
					valid = false;
					break;
				}
			}

			if (valid)
			{
				e = v;
				break;
			}
		}

		if (e==NULL) // we are finished
			break;

		// else
		// e is valid, so process it
		throwif(not mProcessedVertices.count(e)==0,"ProcessedVertices.count(e)!=0");
		throwif(not mPenetrationData.count(e)==0, "not mPenetrationData.count(e)==0");

		std::pair<std::map<Vertex*,PInfo>::iterator,bool> pieRet;
		pieRet = mPenetrationData.insert(std::pair<Vertex*,PInfo>(e,PInfo(e)));
		PInfo& pie = pieRet.first->second;

		BOOST_FOREACH(Vertex* ne, e->mNeighbours)
		{
			if (mProcessedVertices.count(ne)==1)
			{
				throwif(not mPenetrationData.count(ne)==1, "not mPenetrationData.count(ne)==1");
				throwif(not mPenetrationData.count(e)==1, "not mPenetrationData.count(e)==1");
				PInfo& pine = mPenetrationData[ne];

				Vector3d ex = e->x();
				Vector3d vx = ne->x();
				double wev = 1/sqdist(ex,vx);

				pie.depth += wev*(dot(vx-ex,pine.direction) + pine.depth);
				pie.totalDepth += wev;
				pie.direction += wev*pine.direction;
			}
		}

		pie.depth /= pie.totalDepth;
		pie.direction.normaliseInPlace();
		mPenetrationInfo.push_back(PInfo(e,NULL,pie.depth,pie.direction));

		mProcessedVertices.insert(e);
		mUnprocessedVertices.erase(e);
	}
}

// a trivial response procedure
// simply push back the penetrating vertices out of the mesh
// and set the outgoing velocity to be proportional to the penetration dist

void Collision::simpleResponse()
{
	BOOST_FOREACH(PInfo& pi, mPenetrationInfo)
	{
		throwif(pi.v==NULL, "pi.v==NULL");
		Vector3d guess = pi.depth * pi.direction;
		pi.v->mOldX = pi.v->mX;
		pi.v->mX += guess;
	}
}

/* A more appropriate response procedure. applies impulse forces and friction.
 * XXX: Should consider masses and moving faces...
*/
void Collision::simpleResponseB()
{
	BOOST_FOREACH(PInfo& p, mPenetrationInfo)
	{
		//LOG("Applying response on: " << p.v << " at depth " << p.depth << " in direction " << p.direction << "\n");

		if (p.depth < 0.00000001) continue;

		// p.v->mOldX = p.v->mX;
		Vector3d vel = p.v->mX - p.v->mOldX;

		Vector3d n;
		if (p.f != NULL)
			n = p.f->n();
		else
			n = p.direction;

		double vdotn = dot(vel,n);
		Vector3d vn = vdotn * n;
		Vector3d vt = vel - vn;

		p.v->mX += p.depth * p.direction;
		// apply impulse
		// based on newton's law of restitution for instantaneous collisions with no friction

		if (vdotn < 0)
			p.v->mOldX = p.v->mX + CO_RESTITUTION*vn;
		else
			p.v->mOldX = p.v->mX - vel;

		// apply frictional force
		// forces have already been calculated thus far

		double fdotn = dot(p.v->mF,n);
		if (fdotn < 0)
		{
			// force
			Vector3d fn = fdotn * n;
			Vector3d ft = p.v->mF - fn;

			// frictional force
			// if tangential force exceeds static frictional force,
			// then apply kinetic friction

			if (ft.length() > fabs(CO_STATIC_FRICTION * fdotn))
			{
				Vector3d fr = fdotn * CO_KINETIC_FRICTION * ft.normalise();
				p.v->mF = ft + fr;
			}
			else
				p.v->mF = Vector3d::ZERO;
		}
	}

	aabbResponse();
}

void Collision::aabbResponse()
{
	BOOST_FOREACH(Mesh* m, mMeshList)
	BOOST_FOREACH(Vertex* v, m->vertices())
	{
		Vector3d& x = v->mX;
		Vector3d& ox = v->mOldX;

		const Vector3d min = mWorldBounds.min();
		const Vector3d max = mWorldBounds.max();

		Vector3d ll = x - min;
		Vector3d ur = x - max;

		if (ll.x() >= 0) ll.x(0);
		if (ll.y() >= 0) ll.y(0);
		if (ll.z() >= 0) ll.z(0);

		if (ur.x() < 0) ur.x(0);
		if (ur.y() < 0) ur.y(0);
		if (ur.z() < 0) ur.z(0);

		if (ll.x() < 0 or ll.y() < 0 or ll.z() < 0 or ur.x() > 0 or ur.y() > 0 or ur.z() > 0)
		{
			// calculate normal
			Vector3d diff = -(ll + ur);
			Vector3d n = diff.normalise();
			Vector3d vel = x - ox;

			// ox = x; // XXX: not the best way...
			x += diff;

			double vdotn = dot(vel,n);
			Vector3d vn = vdotn * n;
			Vector3d vt = vel - vn;

			// apply impulse
			// based on newton's law of restitution for instantaneous collisions with no friction

			if (vdotn < 0)
				// ox = x + CO_RESTITUTION*vn;
				// pass, keep ox at the same position
				;
			else
			{
				ox = x - vel;
				//std::cout << ".";
			}

			/*

			// apply frictional force
			// forces have already been calculated thus far
			double fdotn = dot(v->mF,n);
			if (fdotn < 0)
			{
				// force
				Vector3d fn = fdotn * n;
				Vector3d ft = v->mF - fn;

				// frictional force
				// if tangential force exceeds static frictional force,
				// then apply kinetic friction

				if (ft.length() > fabs(CO_STATIC_FRICTION * fdotn))
				{
					Vector3d fr = fdotn * CO_KINETIC_FRICTION * ft.normalise();
					v->mF = ft + fr;
				}
				else
					v->mF = Vector3d::ZERO;
			}
			*/
		}
	}
}

std::string Collision::str()
{
	std::ostringstream info;
	info <<
		"HASH_TABLE_SIZE: " << HASH_TABLE_SIZE << std::endl <<
		"CO_RESTITUTION: " <<  CO_RESTITUTION << std::endl <<
		"CO_KINETIC_FRICTION: " << CO_KINETIC_FRICTION << std::endl <<
		"CO_STATIC_FRICTION: " << CO_STATIC_FRICTION << std::endl;

	info << "mCellSize: " << mCellSize << std::endl
		<< "mTotalNumberOfEdges:  " << mTotalNumberOfEdges << std::endl;

	// compute some stats
	double totalDepthOfPenetrations = 0, averageDepthOfPenetrations = 0;
	BOOST_FOREACH(PInfo& p, mPenetrationInfo)
		totalDepthOfPenetrations += p.depth;
	averageDepthOfPenetrations = totalDepthOfPenetrations / mPenetrationInfo.size();

	info << "Last collision: " << std::endl
	<< "Number of penetrations: " << mPenetrationInfo.size() << std::endl
	<< "Total depth of all penetrations: " << totalDepthOfPenetrations << std::endl
	<< "Average depth of all penetrations: " << averageDepthOfPenetrations << std::endl;

	return info.str();
}
