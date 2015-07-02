/* BStreamable: Interface and helpers for streaming in binary mode.
 * BP250108
 */

#ifndef BSTREAMABLE_H
#define BSTREAMABLE_H

#include <fstream>
#include <string>

class BStreamable
{
	public:

	// PRE: streams have their binary bit set
	virtual void bWrite(std::ostream&){};
	virtual void bRead(std::istream&){};
};

// helper functions for binary streamers
// XXX: this doesn't track endianess,
// so beware transferring serialised
// objects between platforms

template <typename T>
void read(std::istream& fin, T& t)
{
	fin.read(reinterpret_cast<char*>(&t),sizeof(T));
}

template <> void read<std::string>(std::istream& fin, std::string& s);

template <typename T>
void write(std::ostream& fout, const T& t)
{
	fout.write(reinterpret_cast<const char*>(&t),sizeof(T));
}

template <> void write<std::string>(std::ostream& fout, const std::string& t);

#endif
