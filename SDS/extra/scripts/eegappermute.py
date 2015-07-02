# Allows Permutations of center tetras vertices 
# BP 241108

import math, numpy


# variables
    # nu+1 - number of tetrahedra in upper component
    # nd+1 - number of tetrahedra in lower component
     
nu2 = 0
nu3 = 0
nu = nu2+nu3
nd = 2

allValidPermutations = [
                        [0,1,2,3],
                        [0,2,3,1],
                        [0,3,1,2],
                        [1,0,3,2],
                        [1,2,0,3],
                        [1,3,2,0],
                        [2,3,0,1],
                        [2,0,1,3],
                        [2,1,3,0],
                        [3,1,0,2],
                        [3,0,2,1],
                        [3,2,1,0]                        
                        ]

for permutation in allValidPermutations:    
    def P(i):
        return permutation[i]
    
    def vIndex((a,b,c,d),x):
        if (x==a): return 0
        elif (x==b): return 1
        elif (x==c): return 2
        elif (x==d): return 3
        else: assert(False)    
    
    filename = "..\\mesh\\eegap%d%d%d%d_%d_%d_%d.1"%(permutation[0],permutation[1],permutation[2],permutation[3],nu2,nu3,nd)
    
    def vec(x,y,z):
        return numpy.array([x,y,z])
    
    def vec2str(v):
        return "%f %f %f "%(v[0],v[1],v[2])
    
    vertexCoordinates = [vec(0,1,1), vec(0,1,-1), vec(1,0,0), vec(-1,0,0)]
    v0 = 0
    v1 = 1
    v2 = 2
    v3 = 3
    X = (P(v0),P(v1),P(v2),P(v3))
    Xindex = 0
    
    # generate u_i nodes
    ui = []
    
    def u(i):
        return ui[i-1]
    
    for i in range(1,nu+1):        
        theta_i = math.pi*(i-1)/(nu-1)
        ui_coord = vec(0,1,0) + vec(1,0,0)*math.cos(theta_i) + vec(0,1,0)*math.sin(theta_i)
        ui.append(len(vertexCoordinates))
        vertexCoordinates.append(ui_coord)
    
    #generate d_i nodes
    di = []
    
    def d(i):
        return di[i-1]
    
    for i in range(1,nd+1):
        theta_i = math.pi*i/nd
        di_coord = vec(0,0,0) + vec(0,0,1)*math.cos(theta_i) + vec(0,-1,0)*math.sin(theta_i)
        di.append(len(vertexCoordinates))
        vertexCoordinates.append(di_coord)
        
    # print out the .node file
    allnodeindices = [v0,v1,v2,v3]
    allnodeindices.extend(ui)
    allnodeindices.extend(di)
    
    nodefile = open(filename+".node",'w')
    nodefile.write("%d 3 0 0\n"%(nu+nd+4)) # header
    
    for vi in allnodeindices:    
        nodefile.write("%d %s\n"%(vi,vec2str(vertexCoordinates[vi])))
    
    nodefile.close()
    
    # generate U tetrahedra and their topological infos
    # the tetra index of Ui is (i+1)
    
    # split this part into U2 and U3 generation
    
    #===============================================================================
    # U = []
    # UN = []
    # 
    # def Uindex(i):
    #    return i
    # 
    # U.append((v2,u(1),v1,v0))
    # for i in range(2,nu+1):
    #    U.append((u(i-1),u(i),v1,v0))
    # U.append((u(nu),v3,v1,v0))
    # 
    # UN.append((Uindex(2),Xindex,-1,-1))
    # for i in range(2,nu+1):
    #    UN.append((Uindex(i+1),Uindex(i-1),-1,-1))
    # UN.append((Xindex,Uindex(nu),-1,-1))
    #===============================================================================
    
    def U3index(i):
        if (i > nu3): return -1
        else: return i
    
    def U2index(i):
        if (i > nu2): return -1
        else: return i + U3index(nu3)
    
    U2, U3 = [],[]
    U2N, U3N = [],[]
    
    if (nu3>0):
        U3.append((v2,u(1),v1,v0))
        for i in range(2,nu3+1):
            U3.append((u(i-1),u(i),v1,v0))
    
        U3N.append((U3index(2),Xindex,-1,-1))
        for i in range(2,nu3+1):
            U3N.append((U3index(i+1),U3index(i-1),-1,-1))
    
    if (nu2>0):
        U2.append((v3,u(nu),v0,v1))
        for i in range(nu,nu+1-nu2,-1):
            U2.append((u(i),u(i-1),v0,v1))
    
        U2N.append((U2index(2),Xindex,-1,-1))
        for i in range(2,nu2+1):
            U2N.append((U2index(i+1),U2index(i-1),-1,-1))
    
    # blah blah
    
    U = U3
    U.extend(U2)
    UN = U3N
    UN.extend(U2N)
    
    # generate D tetras
    D = []
    DN = []
    
    def Dindex(i):
        return i+U2index(nu2)
    
    D.append((v0,d(1),v3,v2))
    for i in range(2,nd+1):
        D.append((d(i-1),d(i),v3,v2))
    D.append((d(nd),v1,v3,v2))
    
    DN.append((Dindex(2),Xindex,-1,-1))
    for i in range(2,nd+1):
        DN.append((Dindex(i+1),Dindex(i-1),-1,-1))
    DN.append((Xindex,Dindex(nd),-1,-1))
    
    # finally set the neighbourhoods of X
    # this depends on the permuted vertices
    
    #Xneigh = (Dindex(nd+1),Dindex(1),U2index(1),U3index(1))
    
    Xneighb = [-1,-1,-1,-1]
    Xneighb[vIndex(X,v0)] = Dindex(nd+1)
    Xneighb[vIndex(X,v1)] = Dindex(1)
    Xneighb[vIndex(X,v2)] = U2index(1)
    Xneighb[vIndex(X,v3)] = U3index(1)
    
    assert(len(Xneighb)==4)
    Xneigh = tuple(Xneighb)
    
    # print out .ele and .neigh data
    ele = [X]
    ele.extend(U)
    ele.extend(D)
    
    elefile = open(filename+".ele",'w')
    elefile.write("%d 4 0\n"%(len(ele))) # header
    
    for i in range(len(ele)):
        e = ele[i]    
        elefile.write("%d %d %d %d %d\n"%(i,e[0],e[1],e[2],e[3]))
    
    elefile.close()
    
    neigh = [Xneigh]
    neigh.extend(UN)
    neigh.extend(DN)
    
    neighfile = open(filename+".neigh",'w')
    neighfile.write("%d 4\n"%(len(neigh)))
    
    for i in range(len(neigh)):
        n = neigh[i]
        neighfile.write("%d %d %d %d %d\n"%(i,n[0],n[1],n[2],n[3]))
    
    neighfile.close()
    
    # finished!
    print "finished permutation ", permutation