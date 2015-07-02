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

from sds.exception import SDSException
from sds import core

class UnknownProcessModelError(SDSException):
    @property
    def requested(self):
        return self._pm

    def __init__(self, requested, message="Process model '%s' not found."):
        SDSException.__init__(self, message % requested)
        self._pm = requested


class Organism(object):
    @staticmethod
    def create_from_tetramesh(mesh, processmodel="NoProcessModel"):
        real = core.OrganismTools.load(mesh.commonprefix)       
        organism = Organism(real, processmodel)
        return organism

    @staticmethod
    def create_default(processmodel="NoProcessModel"):
        real = core.OrganismTools.loadOneTet()
        organism = Organism(real, processmodel)
        return organism

    _cells = None

    @property
    def real(self):
        return self._real

    @property
    def cells(self):
        """The cells making up the organism."""
        if self._cells is None:
            self._cells = self._generate_cells()
        return self._cells

    @property
    def vertices(self):
        """The vertices of the organism."""
        return self._real.mesh().vertices()

    @property
    def tetrahedrae(self):
        """The tetrahedrae of the organism."""
        return self._real.mesh().tetras()

    @property
    def mesh(self):
        """The raw mesh data of the organism."""
        return self._real.mesh()

    @property
    def processmodel(self):
        """The process model of the organism."""
        return self._pm

    def __init__(self, mesh, processmodel):
        """Create a new organism based on a TetrahedralizedMesh.
        
        By default, the process model of the organism is set to NoProcessModel,
        and it will create a tetrahedra as organism.

        """
        self._real = mesh
        self._set_processmodel(processmodel)

    def _set_processmodel(self, pm):
        try:
            real = pm._real
        except AttributeError:
            return False
        self._real.setProcessModel(real)
        self._pm = pm

    def _generate_cells(self):
        """Create organism cells."""
        cells = [Cell.create(c) for c in self._real.cells()]
        return cells
        

class Morphogens:
    def __init__(self, real, norm_level=1):
        self._raw = real
        self._norm_level = norm_level

    @property
    def norm_level(self):
        """The normalisation applied to level"""
        return self._norm_level
            
    def __setitem__(self, key, value):
        """Set the level of a specific morphogen. 

        Index is clamped at a minimum of 0
        Level is clamped between 0 and 1

        """
        if int(key) < 0:
            key = 0
        if float(value) > 1:
            value = 1
        elif float(value) < 0:
            value = 0
        self._raw.setMorphogen(key, value * self._norm_level)


class Cell(object):
    _morphogens = None
    @staticmethod
    def create(raw):
        volume = raw.vol()
        morphogens = Morphogens(raw.getCellContents(), volume)
        return Cell(raw, morphogens)

    def __init__(self, raw, morphogens):
        self._morphogens = morphogens
        self._data = raw

    @property
    def morphogens(self):
        """The cell morphogens."""
        return self._morphogens

class ProcessModel(object):
    def __init__(self, real):
        self._real = real

    @property
    def real(self):
        return self._real

    @property
    def name(self):
        return self._real.name()

    def __len__(self):
        """Return the amount of parameters present in the processmodel"""
        return len(self._real.parameters())

    def keys(self):
        """Return a copy of the parameter names in the processmodel"""
        return list(self._real.parameters())

    def __getitem__(self, parameter):
        """Retrieves the parameter value"""
        if parameter not in self.keys():
            raise KeyError(parameter)
        return self._real.get(parameter)

    def __setitem__(self, parameter, value):
        """Assigns a value to the parameter"""
        if parameter not in self.keys():
            raise KeyError(parameter)
        value = float(value)
        self._real.set(parameter, value)

    def __bool__(self):
        """Evaluate ProcessModel

        This method always returns True to override default python behaviour
        for processmodels without parameters: len(pm) == 0 == False
        """
        return True

    @staticmethod
    def from_name(name):
        """Create a process model based on its name
        
        Returns None if no suitable process model can be found.
        """
        real = core.ProcessModel.create(name)
        if not real:
            return None
        return ProcessModel(real)
    
    @staticmethod
    def from_string(settings):
        """Create a process model based on a libconfig setting
    
        Returns None if the model cannot be created.
        """
        real = core.ProcessModel.loadFromString(settings)
        if not real:
            return None
        return ProcessModel(real)
