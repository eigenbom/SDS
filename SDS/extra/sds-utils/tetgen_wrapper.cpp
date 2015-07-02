/**
 * Demonstrates the wrapper around the tetgen command-line style interface.
 */

#include "sdsutil.h"
int main(int argc, char** argv)
{
	bool success = SDSUtil::runTetgenCommandLine(argc,argv);
	if (success)
		return 0;
	else
		return 1;
}
