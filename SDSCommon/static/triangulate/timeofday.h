/* gettimeofday is not available in MingW, so we supply an equivalent...
 * from instructions here: http://openbabel.org/wiki/Install_(MinGW)
 * */

struct timeval;
void timeofday(struct timeval* p, void* tz /* IGNORED */);


