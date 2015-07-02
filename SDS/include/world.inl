
void World::setBounds(AABB b){mBounds = b;}
const AABB& World::bounds(){return mBounds;}
std::list<Mesh*> World::meshes(){return mMeshes;}


