/* gettimeofday is not available in MingW, so we supply an equivalent...
 * from instructions here: http://openbabel.org/wiki/Install_(MinGW)
 * */

void timeofday(struct timeval* p, void* tz /* IGNORED */);


