# This script creates a special e-e case
# BP 011208

import math, numpy

filename = "..\\mesh\\eespecial.1"

def vec(x,y,z):
    return numpy.array([x,y,z])

def vec2str(v):
    return "%f %f %f "%(v[0],v[1],v[2])

vc = [vec(-1,-.1,0),vec(0,0,1),vec(1,0,0),vec(0,0,-1),vec(0,1,0),vec(0,-1,0),vec(2,.1,0)]

ele = [
        (0,1,2,3),
        (4,1,3,2),
        (5,1,2,0),
        (5,2,3,0),
        (6,1,4,2),
        (6,2,4,3)
        ]

neigh = [
         (1,3,-1,2),
         (0,5,4,-1),
         (0,3,-1,-1),
         (0,-1,2,-1),
         (1,5,-1,-1),
         (1,-1,-1,4)
         ]

nodefile = open(filename+".node",'w')
nodefile.write("%d 3 0 0\n"%len(vc))
for vi in range(len(vc)):    
    nodefile.write("%d %s\n"%(vi,vec2str(vc[vi])))
nodefile.close()


elefile = open(filename+".ele",'w')
elefile.write("%d 4 0\n"%(len(ele))) # header
for i in range(len(ele)):
    e = ele[i]    
    elefile.write("%d %d %d %d %d\n"%(i,e[0],e[1],e[2],e[3]))
elefile.close()


neighfile = open(filename+".neigh",'w')
neighfile.write("%d 4\n"%(len(neigh)))
for i in range(len(neigh)):
    n = neigh[i]
    neighfile.write("%d %d %d %d %d\n"%(i,n[0],n[1],n[2],n[3]))

neighfile.close()

# finished!
print "finished!"