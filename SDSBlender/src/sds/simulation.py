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
from sdsio import IFrameBuilder

class Physics(dict):
    def __init__(self):
        self['kD'] = 0
        self['kSM'] = 1
        self['kDamp'] = 0
        self['kV'] = 0
        self['density'] = 1
        self['gravity'] = 0
        self['viscosity'] = 0.1

class PhysicsManipulator:
    def __init__(self, real=core.Physics):
        self._real = real

    def apply(self, physics):
        """Apply physics settings."""
        for key in physics:
            self[key.lower()] = physics[key]

    def _retrieve_attribute(self, name):
        name = name.lower()
        prop = None
        for real_name in dir(self._real):
            if real_name.lower() == name:
                prop = getattr(self._real, real_name)
        return prop

    def __getitem__(self, name):
        prop = self._retrieve_attribute(name)
        return prop.fget()

    def __setitem__(self, name, value):
        prop = self._retrieve_attribute(name)
        prop.fset(value)

class World:
    def create():
        return World(core.OWorld())

    @property
    def real(self):
        """The real world object."""
        return self._real

    @property
    def bounds(self):
        """The minimum and maximum boundary of the world."""
        bounds = self._real.bounds()
        minimum = [bounds[0], bounds[1], bounds[2]]
        maximum = [bounds[3], bounds[4], bounds[5]]
        return (minimum, maximum)

    @bounds.setter
    def bounds(self, value):
        minimum = list(value[0])
        maximum = list(value[1])
        minimum.extend(maximum)
        bounds = self._real.bounds()
        for i in range(6):
            bounds[i] = minimum[i]
        self._autobounds = False

    def __init__(self, real):
        self._real = real
        self._autobounds = True

    def append_organism(self, organism):
        """Add a new organism to the world.
        
        If no bounds are set manually, this will also recalculate the world 
        bounds.
        """
        self._real.addOrganism(organism.real)
        if self._autobounds:
            self._real.calculateBounds()

    def append_static_mesh(self, mesh):
        """Add a static mesh to the world."""
        self._real.addStaticMesh(mesh)

class Simulation:
    @staticmethod
    def create():
        real = core.SDSSimulation()
        real.cleanup()
        real.setTime(0)
        simulation = Simulation(real)
        return simulation

    @property
    def real(self):
        """The actual simulation object."""
        return self._real

    @property
    def dt(self):
        """Steps per frame."""
        raise AttributeError("Write-only property")
    @dt.setter
    def dt(self, value):
        self._real.setStepSize(value)

    @property
    def stable(self):
        """Is the simulation still stable."""
        return self._real.state() != core.SDSSimulation.UNSTABLE
    
    @property
    def error_message(self):
        """Get the error message."""
        if self.stable:
            return None
        else:
            return self._real.getErrorMessage()

    @property
    def steps(self):
        """Number of simulation steps taken"""
        return self._real.steps()

    @property
    def collisions(self):
        """Is collision detection preformed."""
        raise AttributeError("Write-only property")
    @collisions.setter
    def collisions(self, value):
        if value:
            self._real.setCollisionInterval(1)
        else:
            self._real.setCollisionInterval(0)

    @property
    def world(self):
        """The simulation world."""
        raise AttributeError("Write-only property")
    @world.setter
    def world(self, value):
        self._real.setWorld(value.real)

    def __init__(self, real):
        self._real = real

    def step(self):
        """Perform one full simulation step."""
        while not self._real.substep():
            if self._real.state() == core.SDSSimulation.UNSTABLE:
                return

    @property
    def organisms(self):
        """The organisms in the simulation.

        Note that this method always returns the first organism, due to the
        implementation of SDS.
        """
        world = self._real.world()
        organism = world.organism()
        if organism is None:
            return []
        else:
            return [organism]

class MeshFrameBuilder(IFrameBuilder):
    def __init__(self, factory):
        self._meshfactory = factory
        self._frame_counter = 0
    def create_frame(self, simulation):
        """Create a frame based on the current simulation state."""
        self._frame_counter += 1
        number = self._frame_counter
        core_meshes = (o.mesh() for o in simulation.organisms)
        meshes = [self._meshfactory.create(m) for m in core_meshes]
        return Frame(number, meshes)

class Frame:
    def __init__(self, number, meshes):
        self._number = number
        self._meshes = meshes

    @property
    def number(self):
        return self._number

    @property
    def meshes(self):
        return self._meshes

class Runner:
    STATUS_FINISHED = "The simulation is finished."
    STATUS_UNSTABLE = "The simulation has become unstable."
    STATUS_RUNNING = "The simulation is still running."

    def __init__(self, simulation, framebuilder, frame_step, frame_total):
        self._fb = framebuilder
        self._simulation = simulation
        self._steps = frame_step
        self._total = frame_total
        self._left = 0
        self._status = self.STATUS_RUNNING

    @property
    def status(self):
        """Return the internal status of the runner."""
        return self._status

    @property
    def generated(self):
        """The amount of generated frames."""
        return self._left

    @property
    def target(self):
        """The number of frames to be generated."""
        return self._total

    def __iter__(self):
        return self

    def __next__(self):
        if self._left == self._total:
            self._status = self.STATUS_FINISHED
            raise StopIteration()
        steps_left = self._steps
        while steps_left and self._simulation.stable:
            self._simulation.step()
            steps_left -= 1
        if not self._simulation.stable:
            self._status = self.STATUS_UNSTABLE
            raise StopIteration()
        self._left += 1
        return self._fb.create_frame(self._simulation)
