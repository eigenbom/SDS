#ifndef WORLD_H
#define WORLD_H

#include "aabb.h"
#include "mesh.h"

#include <list>

/*** World: Contains all meshes that interact with each other.
 * Also contains a bounding cube that defines the limits of the simulation.
 * Note: Destroys meshes upon deletion.
 *
 */
class World
{
public:
	World(bool owns=true):mOwns(owns){}
	~World();

	/// add a new mesh to this world
	void addMesh(Mesh* m);
	void removeMesh(Mesh* m);

	/// calculate a suitable bounding box that contains all meshes
	// calculated bounds are the bounds of all meshes doubled in dimension
	void calculateBounds();

	/// set up a default bounding box of side length 2, centered such that the bottom-center point is at (0,0,0)
	void defaultBounds();

	/// manually set the bounds of the world
	inline void setBounds(AABB b);

	/// accessors
	inline const AABB& bounds();
	inline std::list<Mesh*> meshes();

	// stats
	double energy() const;


protected:

	bool mOwns;
	AABB mBounds;
	std::list<Mesh*> mMeshes;
};

#include "world.inl"

#endif
