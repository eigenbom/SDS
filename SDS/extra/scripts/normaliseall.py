'''normalises _all_ *.1.node mesh files in this directory''' 

import os, sys

allnodefiles = [x[:-5] for x in os.listdir(os.getcwd()) if x[-4:]=="node"]
print allnodefiles

def vadd(v1,v2):
    return [a+b for (a,b) in zip(v1,v2)]

def vsub(v1,v2):
    return [a-b for (a,b) in zip(v1,v2)]

def vmul(v1,s):
    return [a*s for a in v1] 

for fileprefix in allnodefiles:
    vertices = []
    vfile = open(fileprefix + ".node",'r')
    vinfo = vfile.readline()
    # parse vinfo to get # of nodes
    numNodes = int(vinfo.split()[0])
    assert(numNodes > 0)        
    counter = 0
    for vline in vfile:
        # parse vline to get node coordinate
        vline = vline.strip()
        if len(vline)==0 or vline[0]=='#': continue
        coord = [float(x) for x in vline.split()[1:]]
        vertices.append(coord)        
        counter += 1
    assert(counter==numNodes)
    vfile.close()    

    # analyse vertices
    
    # get min, max in all dimensions
    vmin = None
    vmax = None
    
    for (x,y,z) in vertices:
        if vmin==None:
            vmin = [x,y,z]
        else:
            if vmin[0]>x: vmin[0] = x
            if vmin[1]>y: vmin[1] = y
            if vmin[2]>z: vmin[2] = z
        if vmax==None:
            vmax = [x,y,z]
        else:
            if vmax[0]<x: vmax[0] = x
            if vmax[1]<y: vmax[1] = y
            if vmax[2]<z: vmax[2] = z
            
    dim = vmax[0]-vmin[0], vmax[1]-vmin[1], vmax[2]-vmin[2]
    bottomCenter = vmin[0] + dim[0]/2, vmin[1], vmin[2] + dim[2]/2
    scaleFactor = min(1./dim[0],1./dim[1],1./dim[2])
    
    # print out modified vertices
    ofile = open(fileprefix + ".node",'w')  
    print>>ofile, vinfo
    counter = 0
    for v in vertices:
        vf = vmul(vsub(v,bottomCenter),scaleFactor)
        print>>ofile, counter, vf[0], vf[1], vf[2]   
        counter += 1
    
