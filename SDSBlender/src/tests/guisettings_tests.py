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

from blender.blender250.gui import GuiSettings, SourceNotPresentError

class GuiSettingsFixture:
    class SimpleContainer(object):
        pass

    def setUp(self):
        self.source = self.SimpleContainer()
        self.settings = GuiSettings()
        self.setup_source()

    def tearDown(self):
        self.source = None
        self.settings = None

    def test_retrieve_attribute(self):
        self.source.sds_data = "Gibberish"
        self.source.sds_more_data = True
        self.setup_attributes(['data', 'more_data'])
        self.assertEquals(self.settings['data'], self.source.sds_data)
        self.assertEquals(self.settings['more_data'], self.source.sds_more_data)

    def test_fails_on_nonexisting_attribute(self):
        with self.assertRaises(AttributeError):
            self.settings["data"]

    def test_fails_without_specification(self):
        with self.assertRaises(AttributeError):
            self.source.sds_data = "Gibberish"
            self.settings["data"]

    def test_fails_without_source(self):
        self.settings = GuiSettings()
        self.setup_attributes(['data'])
        try:
            self.settings["data"]
        except SourceNotPresentError as e:
            self.assertEquals(e.source, self.source_string)


class TestGuiSettingsForMesh(GuiSettingsFixture, unittest.TestCase):
    def setup_source(self):
        self.source_string = "mesh"
        self.settings.mesh = self.source

    def setup_attributes(self, attr):
        self.settings.mesh_attributes.extend(attr)


class TestGuiSettingsForScene(GuiSettingsFixture, unittest.TestCase):
    def setup_source(self):
        self.source_string = "scene"
        self.settings.scene = self.source

    def setup_attributes(self, attr):
        self.settings.scene_attributes.extend(attr)

if __name__ == '__main__':
    unittest.main()
