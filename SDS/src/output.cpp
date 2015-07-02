#include "output.h"
#include <sstream>

bool Output::setParameter(std::string name, std::string value)
{
	if (name=="output_steps")
	{
		std::istringstream iss(value);
		iss >> mN;
		if (mN > 0)
			return true;
		else
			return false;
	}

	return false;
}
