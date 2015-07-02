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

%include "std_string.i"
%include "exception.i"

%{
#include "vector3.h"
#include <sstream>
%}

template<typename T = double>
class Vector3 {
  public:
    Vector3(){}
    Vector3(T x, T y, T z);

    T x() const;
    T y() const;
    T z() const;
    T x(T x);
    T y(T y);
    T z(T z);
    void setData(T x, T y, T z);
};

typedef Vector3<double> Vector3d;
typedef Vector3<int> Vector3i;

%rename(operator_times_aabb_d) operator*(const AABB& a, double b);
%rename(operator_times_d_aabb) operator*(double b, const AABB& a);
%ignore AABB::operator[](int i);
%include "../SDSMath/include/aabb.h"
%extend AABB {
    public:
        double __getitem__(int i) {
            if (i > 6 || i < 0) {
                std::stringstream errMsg;
                errMsg << "Index out of bounds: " << i;
                throw errMsg.str();
            }
            return $self->operator[](i);
        }

        void __setitem__(int i, double val) {
            if (i > 6 || i < 0) {
                std::stringstream errMsg;
                errMsg << "Index out of bounds: " << i;
                throw errMsg.str();
            }
            $self->operator[](i) = val;
        }

        int __len__() {
            return 6;
        }
};
