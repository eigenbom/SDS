
#include <iostream>
#include <string>

#include "processmodel.h"

inline void test(std::string t, bool b)
{
	if (!b)
	{
		std::cerr << "Test \"" << t << "\" failed.";
		exit(-1);
	}
}

bool almostEquals(float a, float b)
{
	if (std::abs(a-b) < .00001) return true;
	else return false;
}

int main()
{
	std::string spec = "\n"
			"processModel :\n"
			"{\n"
			"type = \"LimbBudModelWithGrowth\";\n"
			"diffusionFgf8 = 0.1;\n"
			"diffusionFgf10 = 0.1;\n"
			"decayFgf8 = 0.05;\n"
			  "decayFgf10 = 0.05;\n"
			  "aerS = 0; // 0.0001;\n"
			  "aerR = 0; // 8e-005;\n"
			  "pzSE = 0; // 1.9e-005;\n"
			  "pzSM = 0; //8e-006;\n"
			  "pzR = 0; // 8e-005;\n"
			  "rE = 1.2;\n"
			  "rM = 1.3;\n"
			  "drdtE = .18;\n"
			  "drdtM = .18;\n"
			  "passiveCellGrowthRate = 0.05;\n"
			"};\n";

	ProcessModel* pm = ProcessModel::loadFromString(spec);
	test("processmodel exists", pm!=NULL);
	test("name", pm->name()=="LimbBudModelWithGrowth");
	test("parameter rE", almostEquals(pm->get("rE"),1.2));

	std::cout << "param decayFgf8 = " << pm->get("decayFgf8") << ", should be 0.05\n";

	std::cout << "all tests passed!";
}
