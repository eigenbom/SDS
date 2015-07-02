#ifndef WINDOW_H
#define WINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QTextStream>
#include <QListWidget>
#include <QLabel>
#include <QString>
#include <QVariant>
#include <QMap>
#include <QKeyEvent>
#include <QSlider>
#include <QFile>

#include <iostream>
#include <string>
#include <vector>

#include "ui_mainwindow.h"
#include "graphwindow.h"
#include "inspector.h"
#include "comments.h"
#include "simulationparameters.h"

#include "mesh.h"
#include "organismviewer.h"
#include "organismwidget.h"

#include "oworld.h"
#include "organism.h"
#include "propertylist.h"

#include "sdssimulation.h"

#include "simulationio.h"

#include "random.h"


/// Contains all info about the current scene being simulated
struct SceneInfo
{
	std::string name; // name of scene, as passed to MeshTester or TestWorlds
	double dt;
	double kD;
	double kV;
	double kDamp;
	double gravity;
	double density;
	std::string outputFileName;

	int collisionInterval;

	/// reset to a safe scene
	void reset()
	{
		name = "..\\TestData\\TPSMeshes\\smallsphere.1";
		dt = 0.001;
		kD = 5;
		kV = 5;
		kDamp = 1;
		gravity = 0;
		density = 10;
		outputFileName = "dat/output";

		collisionInterval = 1;
	}
};

class Simulator: public QMainWindow
{
	Q_OBJECT

	public:
	Simulator(QApplication* app, int argc, char** argv);
	~Simulator();

	bool run();

public slots:

	// simulation controls
	void start();
	void record();
	void stop();
	void reset();
	void keepStepping();
	bool step();
	void step1(){step();}
	void step10(){runForNSteps(10);}
	void step50(){runForNSteps(50);}
	void stopAtEventChanged(int);

	// sim inspection, loading, etc..
	void info();
	void load();
	void showInspector();
	void showComments();
	void showSimulationParameters();

	// in playback mode, selects the frame to load
	void chooseFrame(int);

	// select ops
	void selectCell(Cell*);
	void selectFace(Face*);
	void selectEdge(Edge*);
	void selectVertex(Vertex*);

	// divide ops
	void divideRandomCell();
	void divideAlongBoundary();
	void divideHighest();
	void divideHighestAway();
	void divideAll();

	// complexify ops
	void complexifyAll();

	// presets
	void presetOneTet();
	void presetOneCube();

	void redraw();
protected:

	void unstable();
	void finished();
	void runForNSteps(int n);
	void loadNewScene();

	/**
	 * Loads a specific frame from a simulation file.
	 * frameNumber = -1 indicates the last frame
	 */
	void loadSceneFromSimulationFile(std::string header, int frameNumber = 0);
	void loadSceneFromSimulationFileForPlayback(std::string header);
	void loadFrame(int f);

	void outputCurrentFrame();

	void updateMsSim(int ms);
	void updateMsDraw(int ms);
	void updateTime(double t);


	// user interface setup
	Ui::MainWindow mUI;

	// simulation info, simulator, observer and recorder
	SceneInfo mSceneInfo;
	boost::filesystem::path mStartFile;
	OrganismViewer* mViewer;
	OrganismWidget* mOrganismWidget;

	// mode
	enum Mode{Simulation,Playback};
	Mode mode;

	// Simulation Mode
	SDSSimulation* mSimulation;
	std::list<SegmentWriter*> mSegmentWriters;
	std::ofstream mOutputFile;
	int mNumOutputFrames;
	bool mRunInDirtyMode;

	bool mLogTimings;
	QFile* mTimingLogFile;
	QTime mFullStepTimer;

	// timer for stepping the simulator
	QTimer *mTimer;
	unsigned int mMostSteps;
	bool mStopAtEvent;
	bool mPaused;
	bool mRecording;

	// Playback Mode
	unsigned int mTotalNumberOfFrames;
	SimulationLoader* mLoader;

	// system
	Random mRandom;
	static QApplication* pAPP;

	// dynamic gui elements
	Inspector* mInspector;
	Comments* mComments;
	SimulationParameters* mSimulationParameters;

	QPushButton* mPushButtonRun;
	QPushButton* mPushButtonStop;
	QLineEdit* mLineEditStatus;
	QLineEdit* mLineEditMsSim;
	QLineEdit* mLineEditMsDraw;
	QLineEdit* mLineEditTime;
	QSlider* mFrameSlider;
};

#endif
