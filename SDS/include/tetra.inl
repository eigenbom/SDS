

// NULL -- no neighbours ... is an outer tetra ...
void Tetra::setNeighbours(Tetra* a, Tetra* b, Tetra* c, Tetra* d)
{
	if (not (a and b and c and d))
		mOuter = true;
	else
		mOuter = false;

	mN[0] = a;
	mN[1] = b;
	mN[2] = c;
	mN[3] = d;
}

void Tetra::setNeighbour(int i, Tetra* t)
{
	if (t==NULL)
		mOuter = true;
	mN[i] = t;
}

void Tetra::setNeighbour(Vertex* v, Tetra* t)
{
	setNeighbour(this->vIndex(v),t);
}

void Tetra::replaceNeighbour(Tetra* tobereplaced, Tetra* toreplace)
{
	setNeighbour(getNeighbourIndex(tobereplaced),toreplace);
}

double Tetra::volume() const
{
	return (1.0/6) * dot(mV[1]->x()-mV[0]->x(),cross(mV[2]->x()-mV[0]->x(),mV[3]->x()-mV[0]->x()));
}


bool Tetra::isOuter() const {return mOuter;}
Vector3d Tetra::center() const {return (mV[0]->mX + mV[1]->mX + mV[2]->mX + mV[3]->mX) / 4.0;}



double Tetra::springCoefficient() const
{
	return mSpringCoefficient;
}

void Tetra::setSpringCoefficient(double ka)
{
	mSpringCoefficient = ka;
}

void Tetra::setIntersected(bool b){mIntersected = b;}
bool Tetra::intersected() const {return mIntersected;}

const Vertex& Tetra::vertex(int i) const {return *mV[i];}
const Vertex& Tetra::cv(int i) const {return *mV[i];}
Vertex& Tetra::v(int i){return *mV[i];}
Vertex* Tetra::pv(int i){return mV[i];}
Tetra& Tetra::n(int i){return *mN[i];}
Tetra* Tetra::neighbour(int i){return mN[i];}
Tetra* Tetra::neighbour(Vertex* v){int vi = vIndex(v); return (vi==-1)?NULL:mN[vi];}

int Tetra::vIndex(const Vertex* v) const
{
	if (v==mV[0]) return 0;
	else if (v==mV[1]) return 1;
	else if (v==mV[2]) return 2;
	else if (v==mV[3]) return 3;
	else return -1;
}


const AABB& Tetra::bounds() const {return mAABB;}
const AABB& Tetra::aabb() const {return mAABB;}
