TEMPLATE = app
TARGET = SDSPlayer
QT += opengl \
    xml
CONFIG += console \
    no_lflags_merge
INCLUDEPATH += include \
    ../SDSMath/include \
    ../SDS/include \
    ../SDSCommon/include \
    dependencies    
CONFIG(debug, debug|release):win32:LIBS += ../SDS/Debug/libSDS.a \
    ../SDSMath/Debug/libSDSMath.a \
    ../SDSCommon/Release/libSDSCommon.a \
    ./dependencies/QGLViewer/debug/libQGLViewer2.a
else:win32:LIBS += ../SDS/Debug/libSDS.a \
    ../SDSMath/Release/libSDSMath.a \
    ../SDSCommon/Release/libSDSCommon.a \
    ./dependencies/QGLViewer/release/libQGLViewer2.a
    
win32:LIBS += $$QMAKE_LIBS_OPENGL
DEFINES += DEBUG
HEADERS += include/meshdrawer.h \
    include/organismdrawer.h \
    include/palette.h \
    include/sdsplayer.h \
    include/timelinewidget.h \
    include/viewer.h
SOURCES += src/meshdrawer.cpp \
    src/viewer.cpp \
    src/main.cpp \
    src/organismdrawer.cpp \
    src/sdsplayer.cpp \
    src/timelinewidget.cpp
FORMS += sdsplayer.ui
RESOURCES += res.qrc
