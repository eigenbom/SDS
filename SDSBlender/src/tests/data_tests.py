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

import unittest
import mox

from sds import data, core

class TestOrganism(unittest.TestCase):
    def setUp(self):
        pm = core.ProcessModel.create("NoProcessModel")
        self._organism = data.Organism.create_default(processmodel=pm)
    
    def tearDown(self):
        self._organism = None

    def test_constructor_accepts_data_processmodel(self):
        core_pm = core.ProcessModel.create("LimbBudModel")
        assign = data.ProcessModel(core_pm)
        organism = data.Organism.create_default(processmodel=assign)
        assert assign is organism.processmodel

    def test_cells_contain_organism_cells(self):
        cells = self._organism.cells
        self.assertEquals(len(cells), 4)

    def test_vertices_correspond_with_mesh_vertices(self):
        self.assertEquals(len(self._organism.vertices), 
                     len(self._organism.mesh.vertices()))

    def test_tetrahedrae_correspond_with_mesh_tetras(self):
        self.assertEquals(len(self._organism.tetrahedrae), 
                     len(self._organism.mesh.tetras()))

    def _vertex_generator(self, organism):
        for vertex in organism.vertices:
            yield vertex.x().x(), vertex.x().y(), vertex.x().z() 

class MorphogensTests(unittest.TestCase):
    class MockContents:
        def __init__(self):
            self.morphs = {}
        def setMorphogen(self, key, value):
            self.morphs[key] = value

    def setUp(self):
        self.contents = self.MockContents()
        self.morphogens = data.Morphogens(self.contents, norm_level=1)

    def test_morphogens_applied_to_contents_when_set(self):
        self.morphogens[0] = 0.5
        self.morphogens[1] = 0.7
        self.assertDictEqual({0:0.5, 1:0.7}, self.contents.morphs)

    def test_morphogens_clamps_index_to_0_and_higher(self):
        for index in (-50, -1, 0):
            self.morphogens[index] = 0.5
            self.assertDictEqual({0:0.5}, self.contents.morphs)
            self.contents.morphs = {}

    def test_morphogens_clamps_level_between_0_and_1(self):
        values = (0,1,-0.1,1.1)
        expected = (0,1,0,1)
        for key, value in enumerate(values):
            self.morphogens[key] = value
        expected = dict(enumerate(expected))
        self.assertDictEqual(expected, self.contents.morphs)

    def test_morphogens_normalizes_level(self):
        contents = self.MockContents()
        morphogens = data.Morphogens(contents, norm_level=2)
        expected = {0:0, 1:1, 2:2}
        morphogens[0] = 0
        morphogens[1] = 0.5
        morphogens[2] = 1
        self.assertDictEqual(expected, contents.morphs)

    def test_norm_level_contains_normalisation_level(self):
        morphogens = data.Morphogens("Mock", norm_level=2)
        self.assertEquals(morphogens.norm_level, 2)
        morphogens = data.Morphogens("Mock", norm_level=3)
        self.assertEquals(morphogens.norm_level, 3)
        


class MockProcessModel:
    def __init__(self, dict, name="Mock"):
        self.dict = dict
        self.n = name

    def name(self):
        return self.n

    def parameters(self):
        return self.dict

    def get(self, index):
        try:
            return self.dict[index]
        except:
            return 0

    def set(self, index, value):
        self.dict[index] = value

class TestParameterProcessModel(unittest.TestCase):
    def setup_pm(self, param):
        mock = MockProcessModel(param)
        return mock, data.ProcessModel(mock)


    def test_len_returns_amount_of_parameters(self):
        mock, pm = self.setup_pm([])
        self.assertEquals(len(pm), 0)
        mock, pm = self.setup_pm(['1'])
        self.assertEquals(len(pm), 1)
        mock, pm = self.setup_pm(['1', '2'])
        self.assertEquals(len(pm), 2)

    def test_keys_returns_a_copy_parameters(self):
        mock, pm = self.setup_pm(['1','2'])
        keys = pm.keys()
        self.assertListEqual(keys, mock.dict)
        self.assertIsNot(keys, mock.dict)

    def test_get_parameter_returns_value(self):
        params = {'1':0, '2':1, '3':2}
        mock = MockProcessModel(params)
        pm = data.ProcessModel(mock)
        for key in params:
            self.assertEquals(pm[key], params[key])

    def test_get_raises_KeyError_with_unknown_parameter(self):
        mock = MockProcessModel({})
        pm = data.ProcessModel(mock)
        try:
            pm['NotExisting']
            self.fail("Should throw exception")
        except KeyError as e:
            self.assertEquals(e.args[0], 'NotExisting')

    def test_set_sets_parameter_value(self):
        params = {'1':0, '2':1, '3':2}
        mock = MockProcessModel(params)
        pm = data.ProcessModel(mock)
        changes = {'1':0.1, '2':1.2, '3':2.3}
        for change in changes:
            pm[change] = changes[change]
        self.assertDictEqual(params, changes)

    def test_set_raises_KeyError_with_unknown_parameter(self):
        mock = MockProcessModel({})
        pm = data.ProcessModel(mock)
        try:
            pm['NotExisting'] = 1
            self.fail("Should throw exception")
        except KeyError as e:
            self.assertEquals(e.args[0], 'NotExisting')

    def test_set_raises_TypeError_with_non_double_parameter(self):
        mock = MockProcessModel({'1':0})
        pm = data.ProcessModel(mock)
        with self.assertRaises(ValueError):
            pm['1'] = "str"

    def test_name_contains_processmodel_name(self):
        mock = MockProcessModel(None, "Cheeze")
        pm = data.ProcessModel(mock)
        self.assertEquals(pm.name, "Cheeze")

    def test_real_contains_core_processmodel(self):
        mock = "ProcessModel"
        pm = data.ProcessModel(mock)
        self.assertIs(pm.real, mock)

class CellTests(unittest.TestCase):
    def test_cell_creates_normalized_morphogens(self):
        class MockCell:
            def getCellContents(self):
                return "CONTENTS"
            def vol(self):
                return 20
        mock = MockCell()
        cell = data.Cell.create(mock)
        self.assertEquals(cell.morphogens._norm_level, 20)
