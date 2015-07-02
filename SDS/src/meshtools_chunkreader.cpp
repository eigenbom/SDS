#include "meshtools.h"
#include "bstreamable.h"

char* MeshTools::ReadMeshChunk(std::istream& istr, bool& topochanged, unsigned int& numBytes)
{
	read(istr,topochanged);
	read(istr,numBytes);

	if (numBytes > 0)
	{
		char* buff = new char[numBytes];
		if (buff)
		{
			istr.read(buff,numBytes*sizeof(char));
		}
		return buff;
	}
	else return NULL;
}
