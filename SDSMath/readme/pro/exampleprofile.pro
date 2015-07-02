QT += opengl
CONFIG += console no_lflags_merge
TEMPLATE = app
TARGET = 
DEPENDPATH += . \
    ..\TPS
INCLUDEPATH += . \
    C:/boost/boost_1_34_1 \
    ../TPS \
    ../PGL
unix:LIBS += -L \
    C:/boost/boost_1_34_1/stage/lib \
    -l \
    libboost_regex-mgw34-1_34_1.a \
    -l \
    boost_program_options-mgw34-1_34_1.a \
    -l \
    proc/release/libproc.a
win32:LIBS += C:/boost/boost_1_34_1/stage/lib/libboost_regex-mgw34-1_34_1.a \
    C:/boost/boost_1_34_1/stage/lib/boost_program_options-mgw34-1_34_1.a \
    ../PGL/Debug/libPGL.lib \
    $$QMAKE_LIBS_OPENGL

# QMAKE_CXXFLAGS+=-Wfatal-errors
# Input
HEADERS += window.h \
    edge.h \
    face.h \
    physics.h \
    mesh.h \
    meshview.h \
    tetra.h \
    verlet.h \
    vertex.h \
    stepper.h \
    aabb.h \
    collision.h \
    meshloader.h \
    meshtester.h
SOURCES += main.cpp \
    window.cpp \
    physics.cpp \
    mesh.cpp \
    meshview.cpp \
    verlet.cpp \
    stepper.cpp \
    tetra.cpp \
    collision.cpp \
    aabb.cpp \
    face.cpp \
    meshloader.cpp \
    meshtester.cpp \
    vertex.cpp
