'''meshinfo: reads a *.1.node mesh file and spits out some info''' 

import sys

if len(sys.argv)<=1:
    print ("Need a list of files\n")
    sys.exit()

vertices = []

for fileprefix in sys.argv[1:]:
    print fileprefix
    
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
    print "Bounds: ", vmin, ",", vmax
    print "Dimensions: ", dim
    print "Absolute Center: ", vmin[0] + dim[0]/2, vmin[1] + dim[1]/2, vmin[2] + dim[2]/2
    print "Bottom Center: ", vmin[0] + dim[0]/2, vmin[1], vmin[2] + dim[2]/2
    
    
      
    
    
