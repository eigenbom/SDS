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

class IOrganism:
    def get_mesh(self):
        """Return IMesh describing the organism mesh."""

    def get_world_bounds(self):
        """Return the world boundaries.

        This method returns a tuple with two lists containing the lower and
        upper axis values. The three values in each list represent the X, Y and
        Z axis, respectively.
        """

    def get_index_paint(self):
        """Get the paint values.

        The key is the vertex index, the value a tuple with the RGB values. The
        channel values range between 0 and 1.
        """

    def get_process_model_configuration(self):
        """Return a string with the ProcessModel configuration

        The text should be formatted as a libconfig setting string.
        """

class IMesh:
    def get_tetgen_format(self):
        """Convert mesh to tetgen format.

        This is a context manager and should be used with the 'with' statement.

        The file format for this file is completely up to the implementing
        class, as long as it is compatible with the tetgen library.
        """


class IOutput:
    def render_frame(self, frame):
        """Render a frame.

        The organism contained in the frame can be either rendered as part of
        an animation or just dumped along other frames.
        """

    def create_frame_builder(self):
        """Return a frame builder.

        The returned builder must be capable of producing frames compatable 
        with the render frame method.
        """

    def log_error(self, error_msg):
        """Log an error"""


class IFrameBuilder:
    def create_frame(self, simulation):
        """Create a frame.

        This will create a frame based on the current state of the simulation.
        """
