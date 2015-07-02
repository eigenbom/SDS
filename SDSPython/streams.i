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

%{
#include <ostream>
#include <sstream>
#include <fstream>

typedef struct binary_data {
    int size;
    const char* data;
} binary_data;
%}

namespace std {
    class ostringstream: public ostream {
        public:
            explicit ostringstream ( openmode which = ios_base::out );
    };

    class ofstream: public ostream {
        public:
            explicit ofstream (const char * filename, 
                      ios_base::openmode mode = ios_base::out );
    };
}

%extend std::ofstream {
    public:
        void write(const ostringstream& ss) {
            const char* data = ss.str().c_str();
            unsigned int size = ss.str().size();
            $self->write(data, size);
        }
}

%extend std::ostringstream {
    public:
        unsigned int __len__() {
            return $self->str().size();
        }

        %typemap(out) binary_data {
            $result = PyBytes_FromStringAndSize($1.data,$1.size);
        }

        binary_data getvalue() {
            binary_data b;
            b.size = $self->str().size();
            b.data = $self->str().c_str();
            return b;
        }
};

