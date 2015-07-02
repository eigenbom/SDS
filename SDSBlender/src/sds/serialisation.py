# Copyright 2010  Bart Veldstra
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products 
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from sds import core
from sdsio import IOutput

def create_header(core_simulation, physics, binname):
    core_world = core_simulation.world()
    core_organism = core_world.organism()
    core_processmodel = core_organism.processModel()
    header = core.SimulationHeader()
    header.t = core_simulation.t()
    header.dt = core_simulation.stepSize()
    header.collisionInterval = core_simulation.getCollisionInterval()
    header.worldBounds = core_world.bounds()
    header.processModel = core_processmodel
    header.kD = physics['kD']
    header.kV = physics['kV']
    header.kSM = physics['kSM']
    header.kDamp = physics['kDamp']
    header.density = physics['density']
    header.viscosity = physics['viscosity']
    header.gravity = physics['gravity']
    header.frameDataFileName = binname
    return header


def create_header_file(cfgname, header, core_world):
    """Create a simulation header file."""
    core_organism = core_world.organism()
    statics = [core.MeshSegmentIO(m) for m in core_world.getStaticMeshes()]
    writers = [core.MeshSegmentIO(core_organism.mesh()),
               core.OrganismSegmentIO(core_organism),
               core.ProcessModelSegmentIO(core_organism)]
    core.SimulationWriter.writeSimulationHeader(cfgname, header, writers, 
                                                statics)
    return writers, statics

def create_framedata(filename, writers, statics):
    """Write current frame data to stream."""
    output = core.ofstream(filename, core.ofstream.binary)
    core.SimulationWriter.writeFrameDataHeader(output)
    core.SimulationWriter.writeStaticSegments(output, statics)
    core.SimulationWriter.writeFrame(output, 0, 0, 0, writers)

def write_file(prefix, core_simulation, physics):
    cfgfile = prefix + '.cfg'
    binfile = prefix + '.bin'
    header = create_header(core_simulation, physics, binfile)
    core_world = core_simulation.world()
    writers, statics = create_header_file(cfgfile, header, core_world)
    create_framedata(binfile, writers, statics)
