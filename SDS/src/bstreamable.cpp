#include "bstreamable.h"

template <>
void read<std::string>(std::istream& fin, std::string& s)
{
	int numchars;
	read(fin,numchars);
	char* buf = new char[numchars+1];
	for(int i=0;i<numchars;i++)
		read(fin,buf[i]);
	buf[numchars] = '\0';
	s = buf;
	delete[]buf;
}

template <>
void write<std::string>(std::ostream& fout, const std::string& t)
{
	write(fout,(int)t.size());
	for(unsigned int i=0;i<t.size();i++)
		write(fout,t[i]);
}
