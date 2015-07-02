/* Collision: Performs a check for collision between objects, self-collision, and collision with a world bounding box.
 *
 * Define CDBGDRAW if you need access to more collision algorithm info
 *
 * bp
 */

#ifndef COLLISION_H
#define COLLISION_H

#include "mesh.h"
#include "aabb.h"

#include "vector3.h"

#include <list>
#include <set>
#include <map>
#include <vector>
#include <string>
#include <exception>

class Vertex;
class Face;
class Collision
{
	// XXX: right size?
	const static int HASH_TABLE_SIZE = 2999;

	/// Physical Parameters
	const static double CO_RESTITUTION = 0; // 0.001;
	const static double CO_KINETIC_FRICTION = 0; // 0.00001; //0.0001;
	const static double CO_STATIC_FRICTION = 0; // 0.00001; //0.0001; //0.7;

    public:

	class Exception: public std::exception
	{
		std::string mDescription;
	public:
		Exception(std::string description) throw();
		virtual ~Exception() throw() {}
		virtual const char* what() const throw();
	};

	public:

	struct VInfo
	{
		Vertex* v;
		bool border;
	};

	struct EInfo
	{
		Vertex *a, *b; // edge from a(inside mesh) to b(outside mesh)
		bool set; // has this struct had it's p and n values calculated yet?
		Face* f; // face it intersects

		Vector3d p,n; // intersection position and interpolated normal
	};

	// penetrated point information
	struct PInfo
	{
		Vertex* v;
		Face* f;
		double depth; // depth of penetration guess
		Vector3d direction; // penetration direction guess
		double totalDepth;

		PInfo(Vertex* _v = NULL, Face* _f = NULL, double d = 0, Vector3d dir = Vector3d::ZERO)
			:v(_v),f(_f),depth(d),direction(dir),totalDepth(0){}
	};

	public:

	Collision();
	~Collision();

	/// INITIALISATION PROCEDURES
	/// PRE: called only at initialisation
		/// PRE: Mesh* doesn't changed structure
		void addMesh(Mesh*);
		void addStaticMesh(Mesh*); // a static mesh does not move!
		void setWorldBounds(AABB);
		AABB bounds(){return mWorldBounds;}

		void reset();

	/// COLLISION PROCEDURES
		/// a fast collision detection procedure (tags tetras as collided or not)
		void detectCollisions();

		/// A fast and accurate collision detection procedure
		// that estimates penetration depth and direction
		// for every vertex penetrating the surface of the mesh
		// Note: This procedure only detects collision with outer faces
		// i.e., it is not useful for internal collision of tetrahedra
		void estimateCollisionDepthAndDirection();

	/// RESPONSE procedures
	/// PRE: just called estimateCollisionDepthAndDirection
		/// moves vertices to appropriate positions
		void simpleResponse();
		void simpleResponseB();

		void aabbResponse(); // respond to world aabb collision (called by simpleResponse/B)

	/// INFORMATION RETRIEVAL
		/// Information about all penetrating vertices and proposed response
		inline std::vector<PInfo>& penetrationInfo();
		/// List of edges that intersect boundary faces and information about point of intersection, etc.
		inline std::vector<EInfo>& intersectingEdgeInfo();

	/// CDBGDRAW
		/// A list of voxels bounding the intersected boundary faces
		inline std::list<AABB>& DBG_intersectingFaceVoxels();


	/// Get a description of the current state of the Collision system
		std::string str();

	// IMPLEMENTATION DETAILS

	protected:

	inline unsigned int hash(int i, int j, int k);
	inline unsigned int hash(int* i);
	inline void toGridCoords(const Vector3d& worldCoords, int* pGridCoords);
	inline Vector3i toGridCoords(const Vector3d& wc);
	inline void toGridCoords(const double* worldCoords, int* pGridCoords);

	inline Vector3d fromGridCoords(const Vector3i& gc);
	inline AABB fromVoxelSpaceToAABB(const Vector3i& vox);

	double mCellSize;
	std::list<Mesh*> mMeshList;
	std::list<Mesh*> mStaticMeshList;
	std::list<Mesh*> mAllMeshes;
	AABB mWorldBounds;

	std::list<Vertex*>* mpHashTable;
	std::list<EInfo*>* mpEdgeHashTable;

	// collision information
	std::set<Vertex*> mProcessedVertices;
	std::set<Vertex*> mUnprocessedVertices;

	std::map<Vertex*,PInfo> mPenetrationData;

	// collision info
	std::vector<PInfo> mPenetrationInfo;
	std::vector<EInfo> mIntersectingEdges;

	// derived params
	int mTotalNumberOfEdges;

	// CDBGDRAW stuff
	std::list<AABB> mIntersectingFaceVoxels;
};

#include "collision.inl"

#endif
