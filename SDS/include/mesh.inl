
const std::list<Vertex*>& Mesh::vertices() const {return mVertices;}
const std::list<Edge*>& Mesh::edges() const {return mEdges;}
const std::list<Face*>& Mesh::faces() const {return mFaces;}
const std::list<Tetra*>& Mesh::tetras() const {return mTetras;}
const std::list<Face*>& Mesh::outerFaces() const {return mOuterFaces;}

const AABB& Mesh::aabb(){return mAABB;}

bool Mesh::hasTopoChanged(){return mTopoChanged;}

// call this whenever modifying the topology of the mesh
void Mesh::topoChange(){mTopoChanged = true;}
void Mesh::setTopoChanged(bool t){mTopoChanged = t;}

std::map<Vertex*,unsigned int>& Mesh::vertexMapWrite(){return mVertexMapWrite;}
std::map<unsigned int,Vertex*>& Mesh::vertexMapRead(){return mVertexMapRead;}
