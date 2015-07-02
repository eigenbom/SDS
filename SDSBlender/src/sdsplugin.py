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

import sys
sys.path.append('.')

import blender
from sds import data, util, simulation, geometry, serialisation, tetgen

class SetupError(Exception):
    pass


class OrganismBuilder:
    def __init__(self, util, process_model_factory, organism_factory):
        """Create a new organism builder"""
        self._util = util
        self._pm_factory = process_model_factory
        self._o_factory = organism_factory

    def _create_processmodel(self, source):
        config = source.get_process_model_configuration()
        if config:
            pm = self._pm_factory.from_string(config)
        else:
            pm = self._pm_factory.from_name("NoProcessModel")
        if not pm:
            raise SetupError("Could not create a process model")
        return pm

    def create(self, source):
        """Create a new organism based on a sds source"""
        pm = self._create_processmodel(source)
        with source.get_mesh().get_tetgen_format() as mesh:
            organism = self._o_factory.create_from_tetramesh(mesh, pm)
        index_paint = source.get_index_paint()
        if index_paint:
            self._util.import_morphogens(organism, index_paint)
            pm.real.setup() #bp
        return organism


class PhysicsBuilder:
    def create_from_settings(self, settings):
        physics = simulation.Physics()
        for parameter in physics:
            physics[parameter] = settings[parameter.lower()]
        return physics


class WorldBuilder:
    def __init__(self, organism_factory, mesh_factory):
        self._organism_factory = organism_factory
        self._mesh_factory = mesh_factory

    def create(self, organisms, bounds, static_meshes):
        world = simulation.World.create()
        if bounds:
            world.bounds = bounds
        if not organisms:
            # Prevent segfault caused by SDS when no organisms are present
            raise SetupError("At least one organism must be selected.")
        for organism in organisms:
            sds_organism = self._organism_factory.create(organism)
            world.append_organism(sds_organism)
        for mesh in static_meshes:
            with mesh.get_tetgen_format() as tet:
                sds_mesh = self._mesh_factory.create_from_tetramesh(tet)
            world.append_static_mesh(sds_mesh)
        return world


class SimulationBuilder:
    def create(self, world, settings):
        sim = simulation.Simulation.create()
        sim.dt = settings['delta_t']
        sim.collisions = settings['collision']
        sim.world = world
        return sim


class RunnerBuilder:
    def _apply_physics(self, physics):
        """Apply the physics to the simulation."""
        pm = simulation.PhysicsManipulator()
        pm.apply(physics)

    def _create_simulation_runner(self, simulator, frame_builder, settings):
        frame_step = settings['frame_step']
        frame_total = settings['frame_total']
        return simulation.Runner(simulator, frame_builder, frame_step, 
                                 frame_total)

    def create(self, sim, settings, physics, frame_builder):
        """Create a simulation runner."""
        self._apply_physics(physics)
        return self._create_simulation_runner(sim, frame_builder, settings)
        

class SDSStatus:
    def __init__(self, message="", error=False):
        self.message = message
        self.error = error

class SDSPlugin:
    def __init__(self, world_builder, simulation_builder, physics_builder, 
                 runner_builder):        
        self._world_builder = world_builder
        self._simulation_builder = simulation_builder
        self._physics_builder = physics_builder
        self._runner_builder = runner_builder

    def _get_process_string(self, frame_number, total_frames):
        percentage = int(frame_number / total_frames * 100)
        process = "[{:>3}%] Frame {} out of {} frames completed"
        return process.format(percentage, frame_number, total_frames)

    def __call__(self, organisms, static_meshes, settings, output):
        """Run the SDSPlugin."""
        bounds = settings.get_world_bounds()
        fb = output.create_frame_builder()
        try:
            physics = self._physics_builder.create_from_settings(settings)
            world = self._world_builder.create(organisms, bounds, static_meshes)
            sim = self._simulation_builder.create(world, settings)
            runner = self._runner_builder.create(sim, settings, physics, fb)
        except Exception as e:
            error = str(e)
            message = "Error while setting up the simulation: {}".format(error)
            return SDSStatus(message, True)
        if settings['dump_initial']:
            serialisation.write_file('out', sim.real, physics)
            return
        if settings['dirty_mode']:
            sim.real.setContinueOnError(True)
        try:
            for frame in runner:
                output.render_frame(frame)
                current = runner.generated
                total = runner.target
                progress = self._get_process_string(current, total)
                output.log_error(progress)
            output.log_error(runner.status)
        except KeyboardInterrupt:
            message = "Simulation aborted by user."
            output.log_error(message)
        except Exception as e:
            error = str(e)
            message = "Unknown error during simulaton: {}".format(error)
            return SDSStatus(message, True)
        return SDSStatus()

def register():
    organism_builder = OrganismBuilder(util, data.ProcessModel, data.Organism)
    mesh_factory = geometry.CoreMeshFactory.create()
    world_builder = WorldBuilder(organism_builder, mesh_factory)
    physics_builder = PhysicsBuilder()
    simulation_builder = SimulationBuilder()
    runner_builder = RunnerBuilder()
    plugin = SDSPlugin(world_builder, simulation_builder, physics_builder, 
                       runner_builder)
    blender.register(plugin)

if __name__ == "__main__":
    register()
