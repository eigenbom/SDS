#include "simulator.h"

#include <boost/program_options.hpp>
#include <boost/any.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cfloat>

#include <QMessageBox>
#include <QFileDialog>
#include <QTime>

#include "ui_loadDialog.h"

#include "testworlds.h"
#include "physics.h"

#include "organism.h"
#include "organismtools.h"
#include "transform.h"
#include "sdssimulation.h"
#include "limbbudmodel.h"

#include "simulationio.h"

QApplication* Simulator::pAPP;

namespace po = boost::program_options;
namespace fi = boost::filesystem;
using std::string;
using std::cout;
using std::cerr;

Simulator::Simulator(QApplication* app, int argc, char** argv)
	:QMainWindow(NULL)
	,mode(Simulation)
	,mTimer(NULL)

	,mPushButtonRun(NULL)
	,mPushButtonStop(NULL)

	,mLineEditStatus(NULL)
	,mLineEditMsSim(NULL)
	,mLineEditMsDraw(NULL)
	,mLineEditTime(NULL)

	,mMostSteps(0)
	,mPaused(false)
	,mRecording(false)
	,mNumOutputFrames(0)
	,mInspector(NULL)
	,mComments(NULL)
	,mTotalNumberOfFrames(0)

	,mRunInDirtyMode(false)
	,mLogTimings(false)

	,mTimingLogFile(NULL)

{
	mSceneInfo.reset();
	mSimulation = new SDSSimulation();

	mUI.setupUi(this);

	mOrganismWidget = this->findChild<OrganismWidget*>("organismWidget");
	mViewer = mOrganismWidget->findChild<OrganismViewer*>("viewer");

	mPushButtonRun = findChild<QPushButton*>("pushButtonRun");
	mPushButtonStop = findChild<QPushButton*>("pushButtonStop");
	mLineEditStatus = findChild<QLineEdit*>("lineEditStatus");
	mLineEditMsSim = findChild<QLineEdit*>("lineEditMsSim");
	mLineEditMsDraw = findChild<QLineEdit*>("lineEditMsDraw");
	mLineEditTime = findChild<QLineEdit*>("lineEditTime");
	mFrameSlider = this->findChild<QSlider*>("frameSlider");

	/* populate preset list */
	QMenu* presets = findChild<QMenu*>("menuPresets");
	connect(presets->addAction("1-tet"), SIGNAL(triggered()), this, SLOT(presetOneTet()));
	connect(presets->addAction("1-cube"), SIGNAL(triggered()), this, SLOT(presetOneCube()));

	mStopAtEvent = false;

	mComments = new Comments(this);
	mComments->hide();

	mInspector = new Inspector(this,mViewer,mSimulation);
	mInspector->hide();

	mSimulationParameters = new SimulationParameters(this,mSimulation);
	mSimulationParameters->hide();

	/* extract command line options */

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("dirty", "Try to fix errors in a hacky way -- may result in a longer simulation but with stranger results")
		("log-timings", "Log timing > timings.log")
	;

	try
	{
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << "\n";
			return;
		}

		if (vm.count("dirty"))
		{
			mRunInDirtyMode = true;
			mSimulation->setContinueOnError(true);

			std::cout << "Running in dirty mode." << std::endl;
		}

		if (vm.count("log-timings"))
		{
			mLogTimings = true;
			std::cout << "Logging timing to timings.log" << std::endl;

			mTimingLogFile = new QFile(QString("timings.log"), this);
			mTimingLogFile->open(QIODevice::WriteOnly);
		}
	}
	catch(...) // due to probs with program_options, gcc, and fvisibility, we can't see the exception being thrown
	{
		std::cerr << "** Error parsing command line. **\n" << desc;
		return;
	}

	/*
	if (argc==2)
	{
		mStartFile = fi::path(argv[1]);
	}
	else
	{
		mStartFile = fi::path("data") / fi::path("saus2.cfg");
	}
	*/

	pAPP = app;
}

Simulator::~Simulator()
{
	mSimulation->cleanup();

	if (mTimer)
	{
		mTimer->stop();
		delete mTimer;
	}
}

bool Simulator::run()
{
	presetOneTet();
	return true;

	// SKIP ALL THIS JUNK
	/*

	try
	{
		std::cout << "Loading file \"" << mStartFile.file_string() << "\".\n";
		loadSceneFromSimulationFile(mStartFile.file_string(),0);
	}
	catch(std::runtime_error& e)
	{
		std::cerr << "Cannot load \"" << mStartFile.file_string() << "\"" << std::endl;
		std::cerr << e.what() << "\n";
		return false;
	}
	catch(SimulationLoader::LoadException& e)
	{
		std::cerr << "Cannot load \"" << mStartFile.file_string() << "\"" << std::endl;
		std::cerr << e.why() << "\n";
		return false;
	}
	catch(...)
	{
		std::cerr << "Cannot load \"" << mStartFile.file_string() << "\"\nUnknown error." << std::endl;
		return false;
	}

	return true;
	*/
}

void Simulator::loadNewScene()
{
	// First - clean up existing scene, delete meshes etc.,
	mSimulation->cleanup();

	// Second
	Physics::kD = mSceneInfo.kD;
	Physics::kV = mSceneInfo.kV;
	Physics::kDamp = mSceneInfo.kDamp;
	Physics::GRAVITY = mSceneInfo.gravity;
	Physics::DENSITY = mSceneInfo.density;
	mSimulation->setStepSize(mSceneInfo.dt);

	// Third - load the new scene
	// TestOrganism::load may reset the above parameters if it feels the need
	try {
		Organism* o = OrganismTools::load(mSceneInfo.name);
		o->setProcessModel(new LimbBudModel());
		OWorld* ow = new OWorld();
		ow->addOrganism(o);
		ow->defaultBounds();
		mOrganismWidget->setWorld(ow);
		// mOrganismWidget->setWorld(ow);
		mSimulation->setWorld(ow);
	}
	catch (std::exception& e)
	{
		std::cerr << "Window: Error loading organism. " << e.what() << std::endl;
		mOrganismWidget->setWorld(NULL);
		return;
	}
	catch(...)
	{
		std::cerr << "Window: Error loading organism. (didn't catch primary exception!)" << std::endl;
		mOrganismWidget->setWorld(NULL);
		return;
	}

	// TestOrganism::load may have changed these params so set them accordingly
	mSceneInfo.kD = Physics::kD;
	mSceneInfo.kV = Physics::kV;
	mSceneInfo.kDamp = Physics::kDamp;
	mSceneInfo.gravity = Physics::GRAVITY;
	mSceneInfo.density = Physics::DENSITY;
	//mSceneInfo.viscosity = Physics::VISCOSITY;
	mSceneInfo.dt = mSimulation->stepSize();

	repaint();
}

void Simulator::loadSceneFromSimulationFile(std::string header, int frameNumber)
{
	SimulationLoader loader(header);
	SimulationIO_Base::SimulationHeader hdr = loader.simulationHeader();

	if (!(loader.hasSegment("mesh") and
		loader.hasSegment("organism") and
		loader.hasSegment("processinfo")))
	{
		throw(std::runtime_error("Simulation file must contain mesh, organism and processinfo segments."));
	}

	// create the static segment loaders
	// at the moment we just deal with Mesh type static objects
	std::list<SegmentLoader*> staticSegmentLoaders;
	std::list<Mesh*> staticMeshes;

	BOOST_FOREACH(std::string seg, loader.staticSegments())
	{
		if (seg!=MeshSegmentIO::type)
		{
			std::cerr << "Encountered a static object of type " << seg << ". Can only read \"mesh\" objects." << std::endl;
		}
		else
		{
			Mesh* m = new Mesh();
			staticMeshes.push_back(m);

			MeshSegmentIO* io = new MeshSegmentIO(m);
			staticSegmentLoaders.push_back(io);
		}
	}

	if (loader.loadData())
	{
		// read in static component
		if (not staticMeshes.empty())
		{
			std::cout << "Reading in static mesh info ... ";

			if (not loader.loadStaticSegments(staticSegmentLoaders))
			{
				std::cerr << "error!" << std::endl;
				return;
			}

			std::cout << "finished." << std::endl;
		}

		if (frameNumber==-1)
		{
			frameNumber = loader.countFrames()-1;
		}

		// skip forward to the required frame
		for(int i=0;i<frameNumber;i++)
		{
			if (!loader.nextFrame()) throw(std::runtime_error("Simulation file doesn't have that many frames."));
		}

		SimulationIO_Base::FrameHeader frame = loader.currentFrame();

		// Set up scene
		// First - clean up existing scene, delete meshes etc.,
		if (mViewer) mOrganismWidget->setWorld(NULL);
		mSimulation->cleanup();

		mSimulation->initialiseFromHeader(hdr);

		mComments->setText(QString::fromStdString(hdr.comments));

		// third, read in organism data
		Mesh* m = new Mesh();
		MeshSegmentIO ml(m);
		loader.initialiseSegmentLoader(&ml);
		loader.loadSegment(&ml);

		Organism* o = new Organism(m);
		OrganismSegmentIO oio(o);
		loader.initialiseSegmentLoader(&oio);
		loader.loadSegment(&oio);

		ProcessModel* p = hdr.processModel;

		if (p)
		{
			o->setProcessModel(p);
			ProcessModelSegmentIO pml(o);
			if (loader.initialiseSegmentLoader(&pml))
			{
				loader.loadSegment(&pml);
			}
			else
				throw(std::runtime_error("Couldn't load process info!"));
		}

		OWorld* ow = new OWorld();
		ow->addOrganism(o);
		ow->setBounds(hdr.worldBounds);

		BOOST_FOREACH(Mesh* m, staticMeshes)
		{
			ow->addStaticMesh(m);
			mViewer->addStaticMesh(m);
		}

		mOrganismWidget->setWorld(ow);
		mSimulation->setWorld(ow);

		// update info screens
		mSimulationParameters->populateParameters();
	}
	else
		throw(std::runtime_error("Simulation frame data cannot be read!"));
}

void Simulator::loadSceneFromSimulationFileForPlayback(std::string header)
{
	mLoader = new SimulationLoader(header);
	SimulationIO_Base::SimulationHeader hdr = mLoader->simulationHeader();

	if (!(mLoader->hasSegment("mesh") and
		mLoader->hasSegment("organism") and
		mLoader->hasSegment("processinfo")))
	{
		throw(std::runtime_error("Simulation file must contain mesh, organism and processinfo segments."));
	}

	// create the static segment loaders
	// at the moment we just deal with Mesh type static objects
	std::list<SegmentLoader*> staticSegmentLoaders;
	std::list<Mesh*> staticMeshes;

	BOOST_FOREACH(std::string seg, mLoader->staticSegments())
	{
		if (seg!=MeshSegmentIO::type)
		{
			std::cerr << "Encountered a static object of type " << seg << ". Can only read \"mesh\" objects." << std::endl;
		}
		else
		{
			Mesh* m = new Mesh();
			staticMeshes.push_back(m);

			MeshSegmentIO* io = new MeshSegmentIO(m);
			staticSegmentLoaders.push_back(io);
		}
	}

	if (mLoader->loadData())
	{
		// read in static component
		if (not staticMeshes.empty())
		{
			std::cout << "Reading in static mesh info ... ";

			if (not mLoader->loadStaticSegments(staticSegmentLoaders))
			{
				std::cerr << "error!" << std::endl;
				return;
			}

			std::cout << "finished." << std::endl;
		}

		mTotalNumberOfFrames = mLoader->countFrames()-1;

		// enable and set bounds for slider
		mFrameSlider->setEnabled(true);
		mFrameSlider->setMaximum(mTotalNumberOfFrames);

		// Set up global parameters of scene
		if (mViewer) mOrganismWidget->setWorld(NULL);

		mSimulation->initialiseFromHeader(hdr);

		mComments->setText(QString::fromStdString(hdr.comments));

		// third, read in the first frame
		mSimulation->cleanup();
		loadFrame(0);

		BOOST_FOREACH(Mesh* m, staticMeshes)
		{
			mSimulation->world()->addStaticMesh(m);
			mViewer->addStaticMesh(m);
		}

		// update info screens
		mSimulationParameters->populateParameters();
	}
	else
		throw(std::runtime_error("Simulation frame data cannot be read!"));
}

void Simulator::loadFrame(int f)
{
	// std::cout << "Load frame " << f << std::endl;

	if (mLoader->setFrame(f))
	{
		SimulationIO_Base::FrameHeader frame = mLoader->currentFrame();
		SimulationIO_Base::SimulationHeader hdr = mLoader->simulationHeader();

		if (mSimulation->world()==NULL)
		{
			/* load new data */
			Mesh* m = new Mesh();
			MeshSegmentIO ml(m);
			mLoader->initialiseSegmentLoader(&ml);
			mLoader->loadSegment(&ml);

			Organism* o = new Organism(m);
			OrganismSegmentIO oio(o);
			mLoader->initialiseSegmentLoader(&oio);
			mLoader->loadSegment(&oio);

			ProcessModel* p = hdr.processModel;
			if (p)
			{
				o->setProcessModel(p);
				ProcessModelSegmentIO pml(o);
				if (mLoader->initialiseSegmentLoader(&pml))
				{
					mLoader->loadSegment(&pml);
				}
				else throw(std::runtime_error("Couldn't load process info!"));
			}

			OWorld* ow = new OWorld();
			ow->addOrganism(o);
			ow->setBounds(hdr.worldBounds);
			mOrganismWidget->setWorld(ow);
			mSimulation->setWorld(ow);
		}
		else // re-use data structures
		{
			/* load new data */
			OWorld* ow = mSimulation->world();
			Organism* o = ow->organism();
			ow->removeOrganisms();
			o->setProcessModel(NULL);
			delete o;

			Mesh* m = new Mesh();
			MeshSegmentIO ml(m);
			mLoader->initialiseSegmentLoader(&ml);
			mLoader->loadSegment(&ml);

			o = new Organism(m);
			OrganismSegmentIO oio(o);
			mLoader->initialiseSegmentLoader(&oio);
			mLoader->loadSegment(&oio);

			ProcessModel* p = hdr.processModel;
			o->setProcessModel(p);

			if (p)
			{
				ProcessModelSegmentIO pml(o);
				if (mLoader->initialiseSegmentLoader(&pml))
				{
					mLoader->loadSegment(&pml);
				}
				else
					throw(std::runtime_error("Couldn't load process info!"));
			}

			ow->addOrganism(o);
			ow->setBounds(hdr.worldBounds);
			mViewer->setWorld(ow, false);
			mSimulation->setWorld(ow);

			// mViewer->update();
			// mOrganismWidget->setWorld(mSimulation->world());
			//mSimulation->setWorld(ow);
		}
	}
	else
		throw(std::runtime_error("Simulation frame data cannot be read!"));

}

void Simulator::unstable()
{
	stop();

	// signal instability, disable buttons
	QString msg = QString::fromStdString(std::string("UNSTABLE: ") + mSimulation->getErrorMessage());
	mLineEditStatus->setText(msg);
	std::cerr << msg.toStdString() << std::endl;
}

void Simulator::finished()
{
	std::cout << "Finished." << std::endl;
	pAPP->exit();
}

void Simulator::start()
{
	if (not mTimer)
	{
		mTimer = new QTimer(this);
		mTimer->setSingleShot(true);
		connect(mTimer,SIGNAL(timeout()),this,SLOT(keepStepping()));
	}

	if (mTimer->isActive())
	{
		stop();
	}
	else
	{
		mTimer->start(); // process asap

		mLineEditStatus->setText("stable");
	}

	if (mLogTimings)
	{
		mFullStepTimer.restart();
	}
}

void Simulator::record()
{
	std::cerr << "Recording is no longer available in this tool." << std::endl;
	return;

	/*
	// dump the simulation header and set up the segment writers
	SimulationIO_Base::SimulationHeader header;

	mSimulation->writeHeader(header);
	header.frameDataFileName = "simulation.bin";
	header.comments = "Generated by SDSSimulator";

	// build the segment writers
	MeshSegmentIO* mio = new MeshSegmentIO(mSimulation->world()->organism()->mesh());
	OrganismSegmentIO* oio = new OrganismSegmentIO(mSimulation->world()->organism());
	ProcessModelSegmentIO* pio = new ProcessModelSegmentIO(mSimulation->world()->organism());

	BOOST_FOREACH(SegmentWriter* sw, mSegmentWriters)
		delete sw;
	mSegmentWriters.clear();

	mSegmentWriters.push_back(mio);
	mSegmentWriters.push_back(oio);
	mSegmentWriters.push_back(pio);

	// write the simulation configuration file
	SimulationWriter::writeSimulationHeader("simulation.cfg",header,mSegmentWriters);

	// open the binary file
	mOutputFile.open(header.frameDataFileName.c_str(),std::ios::binary);
	if (!mOutputFile)
	{
		std::cerr << "Cannot open " << header.frameDataFileName << " for output!\nContinuing without file output.\n";
	}
	else
	{
		SimulationWriter::writeFrameDataHeader(mOutputFile);
	}

	mRecording = true;
	mNumOutputFrames = 0;

	start();
	*/
}

void Simulator::keepStepping()
{
	bool stepped = step();

	if (mSimulation->state() == mSimulation->UNSTABLE or (!stepped and mStopAtEvent))
	{
		mInspector->clear();

		BOOST_FOREACH(PropertyList::property_type p, mSimulation->lastStepProperties.all())
			mInspector->addProperty(p);
		BOOST_FOREACH(PropertyList::property_type p, Transform::properties.all())
		mInspector->addProperty(p);
		BOOST_FOREACH(PropertyList& pl, Transform::allProperties)
			BOOST_FOREACH(PropertyList::property_type p, pl.all())
				mInspector->addProperty(p);
	}
	else if (mMostSteps and mSimulation->steps() > mMostSteps)
	{
		// then finished, so stop
		stop();
		mMostSteps = 0;
	}
	else
	{
		// keep stepping
		mTimer->start();
	}
}

bool Simulator::step()
{
	QTime t; t.start();

	bool hasCompletedAStep = mSimulation->substep();

	mViewer->update();

	if (mRecording)
		outputCurrentFrame();

	// check state
	if (mSimulation->state()==mSimulation->UNSTABLE)
		unstable();

	// update simulation information
	updateMsSim(t.elapsed());
	updateTime(mSimulation->t());

	if (hasCompletedAStep and mLogTimings)
	{
		double timeElapsed = mFullStepTimer.elapsed();
		QTextStream stream(mTimingLogFile);
		stream << mSimulation->t() << " " << mSimulation->world()->organism()->cells().size() << " " << timeElapsed << "\n";

		mFullStepTimer.restart();
	}

	if (mStopAtEvent)
	{
		// clear properties list
		// clear current property list and actions
		mInspector->clear();

		BOOST_FOREACH(PropertyList::property_type p, mSimulation->lastStepProperties.all())
			mInspector->addProperty(p);
		BOOST_FOREACH(PropertyList& pl, Transform::allProperties)
			BOOST_FOREACH(PropertyList::property_type p, pl.all())
				mInspector->addProperty(p);
	}

	return hasCompletedAStep;
}

void Simulator::stop()
{
	if (mTimer) mTimer->stop();

	mRecording = false;
	if (mOutputFile) mOutputFile.close();
}

void Simulator::runForNSteps(int n)
{
	mMostSteps = mSimulation->steps() + n;
	start();
}

void Simulator::info()
{
	std::cerr << "Simulation::info() is deprecated." << std::endl;

	/*
	// show a dialog box with the latest important info
	std::ostringstream oss;
	oss << "Information" << std::endl;

	// simulation info
	oss << "Scene Parameters" << std::endl
	<< "name: " << mSceneInfo.name << std::endl
	<< "dt: " << mSceneInfo.dt << std::endl
	<< "kD: " << mSceneInfo.kD << std::endl
	<< "kV: " << mSceneInfo.kV << std::endl
	<< "kDamp: " << mSceneInfo.kDamp << std::endl
	<< "density: " << mSceneInfo.density << std::endl;

	// collision info
	//oss << "Collision Info" << std::endl
	//	<< mController->collision().str() << std::endl;

	QMessageBox::information(this,"Information",QString::fromStdString(oss.str()));
	*/
}

void Simulator::reset()
{
	stop();
	run();

	mLineEditStatus->setText("not started");
}

void Simulator::load()
{
	stop();

	QString fileName = QFileDialog::getOpenFileName(this,
	     tr("Open Simulation"), ".", tr("Simulation Header Files (*.cfg)"));
	if (!fileName.isNull())
	{
		try
		{
			QMessageBox msgBox;
			msgBox.setText("Which frame(s) should I load?");
			QPushButton *firstButton = msgBox.addButton(tr("First"), QMessageBox::ActionRole);
			QPushButton *lastButton = msgBox.addButton(tr("Last"), QMessageBox::ActionRole);
			QPushButton *allButton = msgBox.addButton(tr("All"), QMessageBox::ActionRole);
			QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
			msgBox.exec();

			int frame = 0;
			if (msgBox.clickedButton() == firstButton) {
			     frame = 0;
			}
			else if (msgBox.clickedButton() == lastButton)
			{
				frame = -1;
			}
			else if (msgBox.clickedButton() == allButton)
			{
				mode = Playback;
			}

			if (msgBox.clickedButton() != abortButton) {
				if (mode==Playback)
					loadSceneFromSimulationFileForPlayback(fileName.toStdString());
				else
					loadSceneFromSimulationFile(fileName.toStdString(),frame);
			}
		}
		catch(std::runtime_error& e)
		{
			QMessageBox::critical(this,tr("Could not load file."),e.what());
		}
		catch(...)
		{
			QMessageBox::critical(this,tr("Could not load file."),tr("Unknown error."));
		}
	}
}

void Simulator::outputCurrentFrame()
{
	if (mOutputFile)
	{
		// first compute frame size by writing the segment data to a temporary buffer
		std::ostringstream oss(std::ios::binary);
		BOOST_FOREACH(SegmentWriter* sw, mSegmentWriters)
			SimulationWriter::writeSegment(oss, *sw);
		std::string os = oss.str();

		// then construct the appropriate header
		SimulationIO_Base::FrameHeader fhdr;
		fhdr.sizeInBytes = (unsigned int)(os.length());
		fhdr.number = mNumOutputFrames++;
		fhdr.time = mSimulation->t();
		fhdr.step = mSimulation->steps();

		// now write the frame header and frame data to out
		SimulationWriter::writeFrameHeader(mOutputFile,fhdr);
		mOutputFile << os;
	}
}

void Simulator::redraw()
{
	mOrganismWidget->setWorld(mSimulation->world());
}

void Simulator::updateMsSim(int ms)
{
	mLineEditMsSim->setText(QString("sim: ") + QString::number(ms));
}

void Simulator::updateMsDraw(int ms)
{
	mLineEditMsDraw->setText(QString("draw: ") + QString::number(ms));
}

void Simulator::updateTime(double t)
{
	mLineEditTime->setText(QString("time: " + QString::number(t) + "s"));
}

void Simulator::stopAtEventChanged(int e)
{
	mStopAtEvent = (e==Qt::Checked);
}

void Simulator::divideRandomCell()
{
	// select a random cell, a random tetrahedra next to it, and call the divide transformation
	int size =mSimulation->world()->organism()->cells().size();
	int choose = mRandom.getInt(0,size);

	int num = 0;
	Cell* chooseCell = NULL;
	BOOST_FOREACH(Cell* c,mSimulation->world()->organism()->cells())
	{
		if (num==choose)
		{
			chooseCell = c;
			break;
		}
		num++;
	}

	assert(chooseCell);

	// choose a random attached tetrahedra

	// XXX: need to adjust the mesh DS to track vertex->tetra
	std::vector<Tetra*> neighbours;
	BOOST_FOREACH(Tetra* t,mSimulation->world()->organism()->mesh()->tetras())
	{
		if (t->contains(chooseCell->v()))
			neighbours.push_back(t);
	}

	Tetra* t = neighbours[mRandom.getInt(0,neighbours.size())];

	Transform::DivideTetra(chooseCell,t,mSimulation->world()->organism());
}

void Simulator::divideAlongBoundary()
{
	// select a random surface cell and face and divide one into the other

	// XXX: we do this ..the other way round... select  a random surface face then a random vert from it

	int size =mSimulation->world()->organism()->mesh()->outerFaces().size();
	int choose = mRandom.getInt(0,size);

	int num = 0;
	Cell* chooseCell = NULL;
	Face* chooseFace = NULL;
	BOOST_FOREACH(Face* f,mSimulation->world()->organism()->mesh()->outerFaces())
	{
		if (num==choose)
		{
			chooseFace = f;
			break;
		}
		num++;
	}

	assert(chooseFace);

	chooseCell =mSimulation->world()->organism()->getAssociatedCell(&chooseFace->v(mRandom.getInt(0,3)));

	assert(chooseCell);

	Transform::DivideTetra(chooseCell,chooseFace,mSimulation->world()->organism());
}

void Simulator::divideHighest()
{
	// XXX: we do this ..the other way round... select the highest face
	Cell* chooseCell = NULL;
	Face* chooseFace = NULL;
	double highestCentroid = -DBL_MAX;
	BOOST_FOREACH(Face* f,mSimulation->world()->organism()->mesh()->outerFaces())
	{
		double cy = f->center().y();
		if (cy > highestCentroid)
		{
			highestCentroid = cy;
			chooseFace = f;
		}
	}

	assert(chooseFace);

	chooseCell =mSimulation->world()->organism()->getAssociatedCell(&chooseFace->v(mRandom.getInt(0,3)));
	assert(chooseCell);

	Transform::DivideTetra(chooseCell,chooseFace,mSimulation->world()->organism());
}

void Simulator::divideHighestAway()
{
	// XXX: we do this ..the other way round... select the highest face
	Cell* chooseCell = NULL;
	Face* chooseFace = NULL;
	double highestCentroid = -DBL_MAX;
	BOOST_FOREACH(Face* f,mSimulation->world()->organism()->mesh()->outerFaces())
	{
		double cy = f->center().y();
		if (cy > highestCentroid)
		{
			highestCentroid = cy;
			chooseFace = f;
		}
	}

	assert(chooseFace);

	chooseCell =mSimulation->world()->organism()->getAssociatedCell(&chooseFace->v(mRandom.getInt(0,3)));
	assert(chooseCell);

	Transform::DivideAway(chooseCell,chooseFace,mSimulation->world()->organism());
}

void Simulator::divideAll()
{
	// Copy the list of cells
	std::vector<Cell*> randomOrder;
	randomOrder.insert(randomOrder.begin(),mSimulation->world()->organism()->cells().begin(),mSimulation->world()->organism()->cells().end());
	std::random_shuffle(randomOrder.begin(),randomOrder.end());

	// iterate over the random list, dividing each cell into an adjacent tet
	BOOST_FOREACH(Cell* c, randomOrder)
	{
		// select a neighbouring tet randomly
		std::vector<Tetra*> vt;
		BOOST_FOREACH(Tetra* t,mSimulation->world()->organism()->mesh()->tetras())
		{
			if (t->contains(c->v()))
				vt.push_back(t);
		}
		assert(vt.size()>0);

		// divide away
		Transform::DivideTetra(c,vt[mRandom.getInt(0,vt.size())],mSimulation->world()->organism());
	}
}

void Simulator::complexifyAll()
{
	// Apply the complexify transform on all the tetrahedra in the organism
	Organism* o = mSimulation->world()->organism();

	if (not Transform::Complexify(o->cells(),o->mesh()->tetras(), o))
	{
		mSimulation->setErrorMessage("Complexify went wrong");
		unstable();
	}
}

// presets
void Simulator::presetOneTet()
{
	Organism* o = OrganismTools::loadOneTet();
	if (mViewer) mOrganismWidget->setWorld(NULL);
	mSimulation->cleanup();

	ProcessModel* p = ProcessModel::create("NoProcessModel");
	o->setProcessModel(p);

	OWorld* ow = new OWorld();
	ow->addOrganism(o);
	ow->calculateBounds();

	mOrganismWidget->setWorld(ow);
	mSimulation->setWorld(ow);

	// set params
	mSimulation->setStepSize(0.01);
	mSimulation->setCollisionInterval(1);
	Physics::kD = 7;
	Physics::kV = 7;
	Physics::GRAVITY = 1;

	// update info screens
	mSimulationParameters->populateParameters();
}

void Simulator::presetOneCube()
{
	Organism* o = OrganismTools::loadMesh("data/lattice111.1");
	if (mViewer) mOrganismWidget->setWorld(NULL);
	mSimulation->cleanup();

	ProcessModel* p = ProcessModel::create("NoProcessModel");
	o->setProcessModel(p);

	OWorld* ow = new OWorld();
	ow->addOrganism(o);
	ow->calculateBounds();

	mOrganismWidget->setWorld(ow);
	mSimulation->setWorld(ow);

	// set params
	mSimulation->setStepSize(0.01);
	mSimulation->setCollisionInterval(1);
	Physics::kD = 7;
	Physics::kV = 7;
	Physics::GRAVITY = 1;

	// update info screens
	mSimulationParameters->populateParameters();
}

void Simulator::showInspector()
{
	mInspector->show();
}

void Simulator::showComments()
{
	mComments->show();
}

void Simulator::showSimulationParameters()
{
	mSimulationParameters->populateParameters();
	mSimulationParameters->show();
}

void Simulator::chooseFrame(int i)
{
	if (mode==Playback)
	{
		try
		{
			loadFrame(i);
		}
		catch (std::runtime_error& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}
}

void Simulator::selectCell(Cell* c)
{
	mInspector->addProperty(PropertyList::makeProperty("cell",c),true);
}

void Simulator::selectFace(Face* f)
{
	mInspector->addProperty(PropertyList::makeProperty("face",f),true);
}

void Simulator::selectEdge(Edge* e)
{
	mInspector->addProperty(PropertyList::makeProperty("edge",e),true);
}

void Simulator::selectVertex(Vertex* v)
{
	mInspector->addProperty(PropertyList::makeProperty("vertex",v),true);
}










