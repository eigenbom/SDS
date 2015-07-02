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

import unittest, mox

from sds import simulation as mod

class PhysicsManipulatorTests(unittest.TestCase):
    class MockPhysics:
        class PhysProp:
            def __init__(self, value):
                self._value = value
            def fset(self, value):
                self._value = value
            def fget(self):
                return self._value
        def __init__(self):
            self.data = self.PhysProp("data")
            self.DaTa2 = self.PhysProp("data2")

    def setUp(self):
        self.m_physics = self.MockPhysics()
        self.pm = mod.PhysicsManipulator(self.m_physics)

    def tearDown(self):
        self.m_physics = None
        self.pm = None

    def test_get_item_retrieves_property(self):
        self.assertEquals(self.pm["data"], "data")

    def test_get_item_ignores_case(self):
        self.assertEquals(self.pm["data2"], "data2")

    def test_set_item_stores_property(self):
        self.pm["data"] = "altered"
        self.assertEquals(self.m_physics.data.fget(), "altered")

    def test_set_item_ignores_case(self):
        self.pm["data2"] = "altered_data"
        self.assertEquals(self.m_physics.DaTa2.fget(), "altered_data")

    def test_apply_stores_settings(self):
        settings = {'data':'altered', 'data2':'altered_too'}
        self.pm.apply(settings)
        self.assertEquals(self.pm["data"], "altered")
        self.assertEquals(self.pm["data2"], "altered_too")

class PhysicsTest(unittest.TestCase):
    class Property:
        def __init__(self, value):
            self.value = value
        def fget(self):
            return self.value
        def fset(self, value):
            self.value = value
    class Container:
        pass

    def setUp(self):
        test = self.Container()
        test.attr = self.Property(0.6)
        test.attr2 = self.Property(0.5)
        test.AtTr3 = self.Property(0.3)
        self.container = test
        self.physics = mod.PhysicsManipulator(test)
        
    def tearDown(self):
        self.container = None
        self.physics = None

    def test_proxy_to_fget(self):
        self.assertEquals(self.physics["attr"], self.container.attr.fget())
        self.assertEquals(self.physics["attr2"], self.container.attr2.fget())

    def test_proxy_to_fset(self):
        self.physics["attr"] = 1.0
        self.assertEquals(self.container.attr.fget(), 1.0)
        self.physics["attr2"] = 1.0
        self.assertEquals(self.container.attr2.fget(), 1.0)

    def test_make_lowercase(self):
        self.physics["attr3"] = 1.0
        self.assertEquals(self.physics["attr3"], 1.0)

class WorldTests(unittest.TestCase):
    class MockOWorld:
        class Bounds(list):
            pass
        def __init__(self):
            self.organisms = []
            self.bnds = self.Bounds([0,0,0,0,0,0])
            self.calculated = False
        def addOrganism(self, value):
            self.organisms.append(value)
        def calculateBounds(self):
            self.calculated = True
        def bounds(self):
            return self.bnds
        def addStaticMesh(self, core_mesh):
            self.mesh = core_mesh
    class MockOrganism:
        def __init__(self, name):
            self.real = name

    def setUp(self):
        self.organism = self.MockOrganism("organism")
        self.mc = self.MockOWorld()
        self.world = mod.World(self.mc)

    def test_real_contains_real_object(self):
        self.assertIs(self.mc, self.world.real)

    def test_append_adds_instance_to_real_world(self):
        self.world.append_organism(self.organism)
        self.assertSameElements([self.organism.real], self.mc.organisms)

    def test_append_calculates_bounds(self):
        self.world.append_organism(self.organism)
        self.assertTrue(self.mc.calculated)

    def test_world_get_converts_bounds(self):
        self.mc.bnds = [1,2,3,4,5,6]
        expected = ([1,2,3], [4,5,6])
        actual = self.world.bounds
        self.assertListEqual(expected[0], actual[0])
        self.assertListEqual(expected[1], actual[1])

    def test_world_set_converts_bounds(self):
        self.world.bounds = ([1,2,3], [4,5,6])
        expected = [1,2,3,4,5,6]
        actual = self.mc.bnds
        self.assertSameElements(expected, actual)

    def test_no_calculate_when_bounds_set(self):
        self.world.bounds = ([1,2,3], [4,5,6])
        self.world.append_organism(self.organism)
        self.assertFalse(self.mc.calculated)

    def test_append_static_mesh_adds_mesh_to_world(self):
        self.world.append_static_mesh("CORE_MESH")
        self.assertEquals(self.mc.mesh, "CORE_MESH")
        self.world.append_static_mesh("CORE_MESH_2")
        self.assertEquals(self.mc.mesh, "CORE_MESH_2")

class SimulationTests(unittest.TestCase):
    class MockSimulation:
        def setStepSize(self, value):
            self.dt = value
        def setCollisionInterval(self, value):
            self.ci = value
        def setWorld(self, value):
            self.world = value
    class MockWorld:
        def __init__(self, real):
            self.real = real

    def setUp(self):
        self.mock = self.MockSimulation()
        self.simulation = mod.Simulation(self.mock)
        
    def test_assign_dt_calls_setstepsize(self):
        self.simulation.dt = 10
        self.assertEquals(self.mock.dt, 10)

    def test_assign_collision_calls_setcollisioninterval_true(self):
        self.simulation.collisions = True
        self.assertEquals(self.mock.ci, 1)

    def test_assign_collision_calls_setcollisioninterval_false(self):
        self.simulation.collisions = False
        self.assertEquals(self.mock.ci, 0)

    def test_assign_world_calls_setworld(self):
        self.simulation.world = self.MockWorld("world")
        self.assertEquals(self.mock.world, "world")

    def test_real_contains_core_simulation(self):
        self.assertIs(self.mock, self.simulation.real)

class RunningSimulationTests(unittest.TestCase):
    class MockWorld():
        def __init__(self, organism):
            self._organism = organism
        def organism(self):
            return self._organism
    class MockSimulation:
        def __init__(self):
            self.stable = True
            self.unstablestep = -1
            self.sim_steps = 0
            self.error = "Error"
            self.oworld = None
        def substep(self):
            self.substeps -= 1
            if self.unstablestep == self.substeps:
                self.stable = False
            return self.substeps == 0
        def state(self):
            return 1 if self.stable else 2
        def world(self):
            return self.oworld
        def steps(self):
            return self.sim_steps
        def getErrorMessage(self):
            if self.stable:
                raise AssertionError("GetErrorMessage is unavailable when " +
                                     "simulation is stable")
            else:
                return self.error

    def setUp(self):
        self.mock = self.MockSimulation()
        self.mock.oworld = self.MockWorld("organism")
        self.mock.substeps = 10
        self.simulation = mod.Simulation(self.mock)

    def test_step_runs_substep_until_true(self):
        self.simulation.step()
        self.assertEquals(self.mock.substeps, 0)

    def test_step_runs_substep_until_state_unstable(self):
        self.mock.unstablestep = 5
        self.simulation.step()
        self.assertEquals(self.mock.substeps, 5)

    def test_organisms_contains_organism_list(self):
        self.assertListEqual(["organism"], self.simulation.organisms)

    def test_organisms_returns_empty_list_when_no_organisms_exist(self):
        self.mock.oworld = self.MockWorld(None)
        self.assertListEqual([], self.simulation.organisms)

    def test_stable_returns_simulation_stability(self):
        self.assertTrue(self.simulation.stable)
        self.mock.stable = False
        self.assertFalse(self.simulation.stable)

    def test_steps_returns_steps_passed(self):
        self.assertEquals(self.simulation.steps, 0)
        self.mock.sim_steps = 1
        self.assertEquals(self.simulation.steps, 1)
        self.mock.sim_steps = 2
        self.assertEquals(self.simulation.steps, 2)

    def test_error_message_returns_error_when_unstable(self):
        self.mock.stable = False
        self.assertEquals(self.simulation.error_message, self.mock.error)

    def test_error_message_returns_none_when_stable(self):
        self.assertIsNone(self.simulation.error_message)


class MeshFrameBuilderTests(unittest.TestCase):
    class MockSimulation:
        def __init__(self, organism):
            self.organisms = organism
    class MockOrganism:
        def __init__(self, mesh):
            self._mesh = mesh
        def mesh(self):
            return self._mesh
    class MockFactory:
        def create(self, mesh):
            return mesh.lower()
    def setUp(self):
        organisms = [self.MockOrganism("MESH{0}".format(i)) for i in range(6)]
        self.meshes = [o.mesh().lower() for o in organisms]
        self.simulation = self.MockSimulation(organisms)
        self.fb = mod.MeshFrameBuilder(self.MockFactory())
        
    def test_uses_meshfactory_to_convert_organism_mesh(self):
        frame = self.fb.create_frame(self.simulation)
        self.assertListEqual(self.meshes, frame.meshes)

    def test_counts_frames(self):
        frame = self.fb.create_frame(self.simulation)
        self.assertEquals(1, frame.number)
        frame = self.fb.create_frame(self.simulation)
        self.assertEquals(2, frame.number)
        frame = self.fb.create_frame(self.simulation)
        self.assertEquals(3, frame.number)
        

class RunnerTests(unittest.TestCase):
    class MockSimulation:
        def __init__(self):
            self.count = 0
            self.fail_step = 0
            self.stable = True
        def step(self):
            self.count += 1
            self.stable = (self.count != self.fail_step)
    class MockFrameBuilder():
        def create_frame(self, input):
            return "organism"
    def setUp(self):
        self.mock_sim = self.MockSimulation()
        self.mock_fb = self.MockFrameBuilder()
        self.runner = mod.Runner(self.mock_sim, self.mock_fb, 1, 10)

    def test_iterable_calls_fb(self):
        for frame in self.runner:
            self.assertEquals(frame, "organism")
        self.assertGreater(self.mock_sim.count, 0)

    def test_iterable_calls_step(self):
        run = 0
        for frame in self.runner:
            run += 1
            self.assertEquals(self.mock_sim.count, run)

    def test_iterable_calls_step_multiple_times(self):
        runner = mod.Runner(self.mock_sim, self.mock_fb, 2, 10)
        run = 0
        for frame in runner:
            run += 2
            self.assertEquals(self.mock_sim.count, run)

    def test_iterable_returns_maximum_frames(self):
        run = 0
        for frame in self.runner:
            run += 1
        self.assertEquals(10, run)
        
    def test_iterable_stops_upon_unstable(self):
        self.mock_sim.fail_step = 6
        run = 0
        for frame in self.runner:
            run += 1
        self.assertEquals(5, run)

    def test_iterable_is_only_usable_once(self):
        for frame in self.runner:
            pass
        self.mock_sim.count = 0
        for frame in self.runner:
            self.fail("Iterator must not be run")
        self.assertEquals(0, self.mock_sim.count)

    def test_status_returns_status_running(self):
        self.assertEquals(self.runner.status, self.runner.STATUS_RUNNING)

    def test_status_returns_status_finished(self):
        for frame in self.runner:
            pass
        self.assertEquals(self.runner.status, self.runner.STATUS_FINISHED)

    def test_status_returns_status_unstable(self):
        self.mock_sim.fail_step = 6
        for frame in self.runner:
            pass
        self.assertEquals(self.runner.status, self.runner.STATUS_UNSTABLE)

    def test_generated_returns_generated_frame_count(self):
        for frame in self.runner:
            self.assertEquals(self.mock_sim.count, self.runner.generated)

    def test_target_returns_total_frame_count(self):
        for frame in self.runner:
            self.assertEquals(10, self.runner.target)
        
