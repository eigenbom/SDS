'''Creates a rectangular lattice of tetrahedra, useful for tests, etc.'''

from createmesh import *

def invert(tet):
    "inverts the tet and returns it, used when flipping any of the axes"
    tet.v[2], tet.v[3] = tet.v[3], tet.v[2]
    return tet

def unitcube():
    '''create a unit cube mesh'''
    m = Mesh()
    
    coords = [[0,0,0],
              [1,0,0],
              [1,1,0],
              [0,1,0],
              [0,0,1],
              [1,0,1],
              [1,1,1],
              [0,1,1]]
    nodes = a,b,c,d,e,f,g,h = [Node(i,j,k) for (i,j,k) in coords]
    for n in nodes:
        m.addNode(n)
    
    tets = [[d,a,b,e],
            [d,c,g,b],
            [e,d,g,b],
            [h,d,g,e],
            [e,g,f,b]]
    for t in tets:
        m.addTetra(Tetra(t[0],t[1],t[2],t[3]))
    
    return m

def addunitcubefromlist(vlist,mesh,inverted=False):
    '''vlist: index list of verts'''
    a,b,c,d,e,f,g,h = vlist
    tets = [[d,a,b,e],
            [d,c,g,b],
            [e,d,g,b],
            [h,d,g,e],
            [e,g,f,b]]    
    for t in tets:
        tet = Tetra(t[0],t[1],t[2],t[3])
        mesh.addTetra(tet if not inverted else invert(tet))

def ladd(l1,l2):
    return [x+y for x,y in zip(l1,l2)]

def unitcubecoordsrelative(pt):
    '''pt = [i,j,k]'''
    return [ladd(pt,[0,0,0]),
            ladd(pt,[1,0,0]),
            ladd(pt,[1,1,0]),
            ladd(pt,[0,1,0]),            
            ladd(pt,[0,0,1]),
            ladd(pt,[1,0,1]),
            ladd(pt,[1,1,1]),
            ladd(pt,[0,1,1])]
    
def unitcubecoordsrelativeflipped(pt,flipx,flipy,flipz):
    '''pt = [i,j,k]'''
    x0,x1 = (1,0) if flipx else (0,1)
    y0,y1 = (1,0) if flipy else (0,1)
    z0,z1 = (1,0) if flipz else (0,1)     
    return [ladd(pt,[x0,y0,z0]),
            ladd(pt,[x1,y0,z0]),
            ladd(pt,[x1,y1,z0]),
            ladd(pt,[x0,y1,z0]),            
            ladd(pt,[x0,y0,z1]),
            ladd(pt,[x1,y0,z1]),
            ladd(pt,[x1,y1,z1]),
            ladd(pt,[x0,y1,z1])]

def getNode(nodes,(i,j,k)):
    return nodes[k][j][i]

if __name__=="__main__":
    
    print "Testing: ", __file__
        
    m = Mesh()
        
    dim = (2,2,2)
    
    nodes = [[[None for i in range(dim[0]+1)] for j in range(dim[1]+1)] for k in range(dim[2]+1)]
    for i,j,k in ((i,j,k) for i in range(dim[0]+1) for j in range(dim[1]+1) for k in range(dim[2]+1)):
        n = nodes[k][j][i] = Node(i,j,k)
        m.addNode(n)        
    
    # add unit cubes
    for i,j,k in ((i,j,k) for i in range(dim[0]) for j in range(dim[1]) for k in range(dim[2])):
        coords = unitcubecoordsrelativeflipped([i,j,k],i%2==1,j%2==1,k%2==1)        
        nodelist = [getNode(nodes,c) for c in coords]        
        addunitcubefromlist(nodelist,m,(i+j+k)%2==1)

    filename = "lattice%d%d%d"%dim
    print "Writing out to prefix: ", filename
    OutputMeshDot1(m,filename,".")    
        
    