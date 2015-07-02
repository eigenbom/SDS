'''Provides a higher-level mesh creation interface. 
Meshes can be created, modified, etc. using the ihgh-level functions.
The mesh can then be output into a form suitable for TPS.
BP270209 
''' 

import math
from euclid import Vector3

class Indexable(object):
    def __init__(self):
        self.index = None
    def __cmp__(self,other):
        return -1 if self.index<other.index else (1 if self.index>other.index else 0)            

class Node(Indexable):
    def __init__(self,x,y,z):
        self.pos = Vector3(x,y,z)
        Indexable.__init__(self)
    
class Tetra(Indexable):
    def __init__(self,v1,v2,v3,v4):
        self.v = [v1,v2,v3,v4]        
        Indexable.__init__(self)
        self.neighbours = [None,None,None,None]
        
    def contains(self,v1,v2=None,v3=None,v4=None):
        return v1 in self.v and \
            ((v2 in self.v) if v2 is not None else True) and \
            ((v3 in self.v) if v3 is not None else True) and \
            ((v4 in self.v) if v4 is not None else True)          
    
    def oddOneOut(self,v1,v2,v3):
        '''given three verts, return the vert in this tet thats missing'''
        for v in self.v:
            if v not in [v1,v2,v3]: return v
    
class Mesh:
    def __init__(self):
        self.nodes = []
        self.tetras = []  
        self.updatedNeighbours = False      
        
    def addNode(self,node):
        nextIndex = self.generateNewNodeIndex()
        node.index = nextIndex
        self.nodes.append(node)
        
    def addTetra(self,tetra):
        nextIndex = self.generateNewTetraIndex()
        tetra.index = nextIndex
        self.tetras.append(tetra)
        
    def generateNewNodeIndex(self):
        return len(self.nodes)
    
    def generateNewTetraIndex(self):
        return len(self.tetras)
        
    def updateNeighbours(self):
        '''calculates the neighbour info from the tetra list'''
        self.updatedNeighbours = True
        for t in self.tetras:
            # for each triplet of vertices, try to find the corresponding triplet in another tetra
            for (v1,v2,v3) in [[v1,v2,v3] for v1 in t.v for v2 in t.v for v3 in t.v if v1>v2 and v2>v3]:                
                v = t.oddOneOut(v1,v2,v3)                
                vindex = t.v.index(v)
                if t.neighbours[vindex] is None:                
                    for tn in self.tetras:
                        if t==tn: continue
                        if tn.contains(v1,v2,v3):
                            t.neighbours[vindex] = tn
                            tn.neighbours[tn.v.index(tn.oddOneOut(v1,v2,v3))] = t             
        
def OutputMeshDot1(mesh,prefix,dir="."):
    '''Writes mesh out to .1 format file, in the directory specified
    Creates the files prefix.1.node, prefix.1.ele and prefix.1.neigh'''
    prefix = dir + "\\" + prefix + ".1"
    
    if not mesh.updatedNeighbours:
        mesh.updateNeighbours()
    
    def VecToStr(v):
        return "%f %f %f"%(v.x,v.y,v.z)
    
    # write out nodes (.node file)
    nodefile = open(prefix + ".node","w")
    nodefile.write("%d 3 0 0\n"%len(mesh.nodes)) # header
    
    for n in sorted(mesh.nodes):
        nodefile.write("%d %s\n"%(n.index,VecToStr(n.pos)))
    nodefile.close()
    
    # write out tetras (.ele file)
    elefile = open(prefix + ".ele","w")
    elefile.write("%d 4 0\n"%(len(mesh.tetras))) # header

    for t in sorted(mesh.tetras):
        vi = [v.index for v in t.v]
        elefile.write("%d %d %d %d %d\n"%(t.index,vi[0],vi[1],vi[2],vi[3]))
    
    elefile.close()

    # write out neighbours (.neigh file)
    neighfile = open(prefix + ".neigh","w")
    neighfile.write("%d 4\n"%(len(mesh.tetras)))

    for t in sorted(mesh.tetras):
        neighfile.write("%d "%(t.index))
        for n in t.neighbours:
            if n:
                neighfile.write("%d "%(n.index))
            else:
                neighfile.write("-1 ")
        neighfile.write("\n")
    neighfile.close()
    
# tests

if __name__=="__main__":
    print "Testing ", __file__
    
    m = Mesh()
    v1,v2,v3,v4 = Node(0,0,0), Node(1,0,0), Node(1,1,0), Node(1,1,1)
    m.addNode(v1)
    m.addNode(v2) 
    m.addNode(v3) 
    m.addNode(v4)    
    m.addTetra(Tetra(v1,v2,v3,v4))    
    
    OutputMeshDot1(m,"test")    
    
    
    
    