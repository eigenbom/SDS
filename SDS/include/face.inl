Vector3d Face::center() const {return (mV[0]->mX + mV[1]->mX + mV[2]->mX) / 3.0;}
double Face::rest() const {return mRestArea;}
double Face::rest(double r){return mRestArea = r;}

const Vertex& Face::cv(int i) const {return *mV[i];}
Vertex& Face::v(int i) const {return *mV[i];}
void Face::setVertices(Vertex* a, Vertex* b, Vertex* c){mV[0] = a; mV[1] = b; mV[2] = c;}

// vertex and edge composition tests
bool Face::contains(Vertex* v) const {return mV[0]==v or mV[1]==v or mV[2]==v;}

int Face::index(Vertex* v) const {for(int i=0;i<3;i++){if (mV[i]==v) return i;} /*otherwise*/ return -1;}
// note: this returns some false positives
// for those triangles that are touching the surface
// but not part of the surface
bool Face::surface() const {
	std::cerr << "Face::surface is deprecated, but has just been called!\n";
	return mV[0]->surface() and mV[1]->surface() and mV[2]->surface();
	}

// outer(): is this triangle on the surface?
// nb: better than calling surface()
bool Face::outer() const {return mOuter;}
void Face::setOuter(bool o) {mOuter = o;}

const Vector3d& Face::q() const {return mV[0]->x();}
Vector3d Face::n() const {return cross(mV[1]->x()-mV[0]->x(),mV[2]->x()-mV[0]->x()).normaliseInPlace();}
