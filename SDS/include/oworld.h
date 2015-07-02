#ifndef O_WORLD_H
#define O_WORLD_H

#include "world.h"
#include <list>

class Organism;

/**
 * An OWorld (organism world), handles the management, creation and deletion of organisms.
 * It encapsulates a TPS World and handles the system interfacing between organism and mesh.
 */
class OWorld
{
public:
	/**
	 * owns=true indicates that OWorld destroys organisms upon deletion
	 */
	OWorld(bool owns=true);
	virtual ~OWorld();

	void addOrganism(Organism*);
	/// shorthand, returns the first organism
	// PRE: there is at least one organism in the world
	Organism* organism();

	/**
	 * Remove the organisms without deleting them.
	 */
	void removeOrganisms();

	void addStaticMesh(Mesh* m);
	const std::list<Mesh*>& getStaticMeshes() const;

	/// pass-through interface to world
	/// calculate a suitable bounding box that contains all meshes
	// calculated bounds are the bounds of all meshes doubled in dimension
	void calculateBounds();

	/// set up a default bounding box of side length 2, centered such that the bottom-center point is at (0,0,0)
	void defaultBounds(){mWorld->defaultBounds();}

	/// manually set the bounds of the world
	void setBounds(AABB b){mWorld->setBounds(b);}

	/// accessors
	const AABB& bounds(){return mWorld->bounds();}
	std::list<Mesh*> meshes(){return mWorld->meshes();}

	// stats
	double energy() const {return mWorld->energy();}

protected:
	bool mOwnsOrganisms;
	World* mWorld;
	std::list<Organism*> mOrganisms;
	std::list<Mesh*> mStaticMeshes;

};

#endif
