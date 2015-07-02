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

%include "std_vector.i"

%{
#include "sdsutil.h"
#include <string>
#include <vector>
#include <cstring>
%}

namespace std {
    %template(StringVector) vector<string>;
}

class SDSUtil
{
    public:
        static bool runTetgenCommandLine(int argc, char** argv);
};


%extend SDSUtil {
    public:
        static bool tetgen(std::vector<std::string> args) {
            int argc = args.size() + 1;
            char** argv = new char*[argc];

            // Prepend parameters with program name
            std::string argument = "tetgen_builtin";
            argv[0] = new char[argument.length() + 1];
            strcpy(argv[0], argument.c_str());

            // Convert arguments to char
            for (int i = 1; i < argc; i++) {
                std::string argument = args[i - 1];
                argv[i] = new char[argument.length() + 1];
                strcpy(argv[i], argument.c_str());
            }

            bool result = SDSUtil::runTetgenCommandLine(argc, argv);

            // Cleanup
            for (int i = 0; i < argc; i++) {
                delete argv[i];
            }
            delete[] argv;

            return result;
        }
};
