void Vertex::zeroF(){mF = Vector3d::ZERO;}
void Vertex::zeroV(){mOldX = mX; mV = Vector3d::ZERO;}
void Vertex::addF(const Vector3d& v){mF += v;}
void Vertex::addX(const Vector3d& x){mX += x;}
void Vertex::resetX(const Vector3d& x){mX = x; mOldX = x; mV = Vector3d::ZERO;}

void Vertex::addM(double dm){mMass += dm; computeR();}
void Vertex::setM(double d){mMass = d; computeR();}
void Vertex::setSurface(bool s){mSurface = s;}

const Vector3d& Vertex::v() const {return mV;}
const Vector3d& Vertex::x() const {return mX;}
const Vector3d& Vertex::f() const {return mF;}
const double Vertex::m() const {return mMass;}
bool Vertex::surface() const {return mSurface;}

const Vector3d& Vertex::ox() const {return mOldX;}

bool Vertex::isFrozen() const {return mIsFrozen;}
void Vertex::setFrozen(bool f) {mIsFrozen = f;}

bool Vertex::tag() const {return mTag;}
void Vertex::tag(bool t){mTag = t;}

const std::list<Face*>&	Vertex::surfaceFaces() const {return mFaceNeighbours;}
const std::list<Vertex*>& Vertex::neighbours() const {return mNeighbours;}

double Vertex::r() const {return mR;}
