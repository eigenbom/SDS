std::vector<Collision::PInfo>& Collision::penetrationInfo(){return mPenetrationInfo;}
std::vector<Collision::EInfo>& Collision::intersectingEdgeInfo(){return mIntersectingEdges;}

std::list<AABB>& Collision::DBG_intersectingFaceVoxels(){return mIntersectingFaceVoxels;}

unsigned int Collision::hash(int i, int j, int k)
{
	static const int p1 = 73856093, p2 = 19349663, p3 = 83492791;
	int result = (i*p1 + j*p2 + k*p3) % HASH_TABLE_SIZE;
	if (result < 0) result += HASH_TABLE_SIZE;
	return result;
}

unsigned int Collision::hash(int* i)
{
	return hash(i[0],i[1],i[2]);
}

void Collision::toGridCoords(const Vector3d& v, int* p)
{
	p[0] = (int)std::floor(v.x() / mCellSize);
	p[1] = (int)std::floor(v.y() / mCellSize);
	p[2] = (int)std::floor(v.z() / mCellSize);
}

void Collision::toGridCoords(const double* v, int* p)
{
	p[0] = (int)std::floor(v[0] / mCellSize);
	p[1] = (int)std::floor(v[1] / mCellSize);
	p[2] = (int)std::floor(v[2] / mCellSize);
}

Vector3i Collision::toGridCoords(const Vector3d& v)
{
	return Vector3i(
			(int)std::floor(v[0] / mCellSize),
			(int)std::floor(v[1] / mCellSize),
			(int)std::floor(v[2] / mCellSize));
}

Vector3d Collision::fromGridCoords(const Vector3i& gc)
{
	return Vector3d(
		gc[0]*mCellSize,
		gc[1]*mCellSize,
		gc[2]*mCellSize);
}

AABB Collision::fromVoxelSpaceToAABB(const Vector3i& gc)
{
	return AABB(
		gc[0]*mCellSize,
		gc[1]*mCellSize,
		gc[2]*mCellSize,
		(gc[0]+1)*mCellSize,
		(gc[1]+1)*mCellSize,
		(gc[2]+1)*mCellSize);
}
