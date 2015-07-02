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

import unittest, os, subprocess

from sds import serialisation as mod
from sds import core, data, simulation as sim

no_integration_tests = True

class StreamOutputTests(unittest.TestCase):
    @unittest.skipIf(no_integration_tests, "Integration tests are disabled")
    def test_integration(self):
        organism_tetgen = '../../SDS/extra/data/lattice111.1'
        static_mesh_tetgen = '../data/tetgen/simulation-20091209-1209.1'
        organism = core.OrganismTools.load(organism_tetgen)
        pm = core.ProcessModel.create("NoProcessModel")
        organism.setProcessModel(pm)

        static_mesh = core.MeshTools.Load(static_mesh_tetgen)

        world = core.OWorld()
        world.addOrganism(organism)
        world.addStaticMesh(static_mesh)
        world.calculateBounds()
        
        simulation = core.SDSSimulation()
        simulation.cleanup()
        simulation.setWorld(world)
        physics = sim.Physics()
        mod.write_file('test', simulation, physics)
        print("\nInspect the test.cfg and test.bin files now")
        subprocess.call(['bash'])
        os.remove('test.cfg')
        os.remove('test.bin')
