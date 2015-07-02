// Copyright 2010  Bart Veldstra
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products 
//    derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

%include "math.i"
%include "processmodel.i"
%include "std_string.i"
%include "std_list.i"
%include "std_iostream.i"

%{
#include "simulationio.h"
%}

%{
// Trick the compiler into thinking the nested structs are actually an global structs
typedef SimulationIO_Base::SimulationHeader SimulationHeader;
typedef SimulationIO_Base::FrameHeader FrameHeader;
typedef SimulationLoader::LoadException LoadException;
typedef SimulationIO_Base::StaticMesh StaticMesh;
%}

namespace std {
    %template(SegmentWriterList) list<SegmentWriter*>;
    %template(SegmentNameList) list<std::string>;
    %template(StaticMeshList) list<StaticMesh*>;
}

struct StaticMesh
{
  Vector3d pos;
  double sx, sy, sz;
  std::string filePrefix;

  StaticMesh(std::string filePrefix);
};


struct SimulationHeader {
        double t;
        double dt;
        int numFrames;
        int collisionInterval;
        AABB worldBounds;
        double gravity;
        double kD;
        double kSM;
        double kV;
        double kDamp;
        double density;
        double viscosity;
        std::string frameDataFileName;
        //boost::posix_time::ptime time;
        std::string comments;
        ProcessModel* processModel; // may be NULL
        std::list<StaticMesh> staticMeshes;

        SimulationHeader(); // loads default values
};

struct FrameHeader {
        unsigned int sizeInBytes;
        unsigned int number;
        double time;
        unsigned int step;

        static int size();
};

class LoadException {
    LoadException(std::string why);
};

/*
class SimulationIO_Base {};

class SimulationWriter: public SimulationIO_Base
{
    public:
        static void writeSimulationHeader(std::string file, SimulationHeader& sh, std::list<SegmentWriter*> sws = std::list<SegmentWriter*>(), std::list<SegmentWriter*> staticSegments = std::list<SegmentWriter*>());
        static void writeFrame(std::ostream& out, int number, double time, int step, std::list<SegmentWriter*> segmentWriters);

        static void writeFrameDataHeader(std::ostream& o);
        static void writeFrameHeader(std::ostream& o, FrameHeader& fh);
        static void writeSegment(std::ostream& o, SegmentWriter& sw);
};

class SimulationLoader: public SimulationIO_Base
{
    public:
        SimulationLoader(std::string header_filename) throw(SimulationLoader::LoadException);
        SimulationHeader simulationHeader();
        int countFrames();
        bool hasSegment(std::string type);
        std::list<std::string> segments();
        bool initialiseSegmentLoader(SegmentLoader* sl);
        bool loadFrameData();
        const SimulationIO_Base::FrameHeader& currentFrame() const;
        bool nextFrame();
        bool setFrame(int f);
        void loadSegment(SegmentLoader* sl);
};
*/

%include "../SDS/include/simulationio.h"
