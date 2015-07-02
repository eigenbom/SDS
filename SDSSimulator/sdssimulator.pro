QT += opengl \
    xml
CONFIG += console \
    debug \
    no_lflags_merge \
		exceptions
TEMPLATE = app
UI_HEADERS_DIR = includeui
INCLUDEPATH += include \
	includeui \
    ../SDSMath/include \
    ../SDS/include \
    ../SDS/include/processmodels \
    ../SDSCommon/include \

CONFIG(debug, debug|release):LIBS += ../SDS/Release/libSDS.a \
    ../SDSMath/Release/libSDSMath.a \
    ../SDSCommon/Release/libSDSCommon.a \

CONFIG(debug, debug|release):win32 {
    LIBS += ./dependencies/QGLViewer/debug/libQGLViewer2.a
    INCLUDEPATH += dependencies
}

CONFIG(debug, debug|release):unix {
    LIBS += ./dependencies-linux/QGLViewer/release/libqglviewer-qt4.a
    INCLUDEPATH += dependencies-linux
}

#win32:LIBS += ../SDS/Release/libSDS.dll \
#    ./dependencies/QGLViewer/release/libQGLViewer2.a

# Input
HEADERS += include/comments.h \
    include/simulator.h \
    include/glutils.h \
    include/annotation.h \
    include/organismwidget.h \
    include/graphwindow.h \
    include/organismviewer.h \
    include/inspector.h \
    include/simulationparameters.h
SOURCES += src/comments.cpp \
    src/simulator.cpp \
    src/glutils.cpp \
    src/annotation.cpp \
    src/organismwidget.cpp \
    src/graphwindow.cpp \
    src/main.cpp \
    src/organismviewer.cpp \
    src/inspector.cpp \
    src/simulationparameters.cpp
FORMS = ui/organismwidget.ui \
    ui/graphwindow.ui \
    ui/loadDialog.ui \
    ui/mainwindow.ui \
    ui/comments.ui \
    ui/inspector.ui \
    ui/simulationparameters.ui
RESOURCES = res.qrc
