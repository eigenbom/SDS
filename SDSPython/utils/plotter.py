# This utility extracts the heights of the vertices at each time step
# and plots it. 
# BP 271109

import sys

from sds import *

try:
	from pylab import plot, scatter
except:
	print("You need matplotlib to run this script.")
	sys.exit()

def frames(file):
	loader = SimulationLoader(file)
	header = loader.simulationHeader()
	if loader.loadFrameData():
		while True:
			frame = loader.currentFrame()
			if loader.hasSegment("mesh"):
				m = Mesh()
				ml = MeshSegmentIO(m)
				loader.initialiseSegmentLoader(ml)
				loader.loadSegment(ml)

				yield(m)

				if not loader.nextFrame(): break

if __name__=="__main__":
	if len(sys.argv)<2:
		print("Expecting filename/s as argument")
	else:
		for file in sys.argv[1:]:
			print("Processing file: " + file)
			timeline = []
			allpos = []
			for i,mesh in enumerate(frames(file)):
				pos = []
				for v in mesh.vertices():
					#pos.append(v.x().x())
					pos.append(v.x().y())
					#pos.append(v.x().z())
				allpos.extend(pos)
				timeline.extend([i]*len(pos))
			scatter(timeline,allpos,s=1)
	# wait until we close the window
	w = input()





