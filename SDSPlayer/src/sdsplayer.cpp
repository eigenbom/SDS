#include "sdsplayer.h"
#include "timelinewidget.h"

#include "mesh.h"
#include "vertex.h"
#include "cell.h"
#include "organism.h"
#include "sdsutil.h"

#include <iostream>
#include <exception>

#include <QFileDialog>
#include <QProgressDialog>
#include <QThread>
#include <QMessageBox>

SDSPlayer::SDSPlayer(QWidget *parent)
    : QMainWindow(parent),mIRecordedSimulation(NULL),mSelectedItem(),mCurrentFrame(NULL)
{
	ui.setupUi(this);

	// FileMenu
	connect(findChild<QAction*>("action_Open"),SIGNAL(triggered()),this,SLOT(open()));
	connect(findChild<QAction*>("actionQuit"),SIGNAL(triggered()),this,SLOT(close()));

	connect(findChild<QPushButton*>("pushButtonTetTopograph"), SIGNAL(clicked()),
			this, SLOT(generateTetTopograph()));

	// TimeLineWidget
	mTimeLineWidget = findChild<TimeLineWidget*>("timeline");
	connect(mTimeLineWidget,SIGNAL(frameSelected(int)),this,SLOT(frameSelected(int)));

	// playbackrate widget
	// already connected ...

	// information labels
	mLabelFilename = findChild<QLabel*>("labelFilename");
	mLabelVersion = findChild<QLabel*>("labelVersion");
	mLabelDate = findChild<QLabel*>("labelDate");

	// viewer widget
	mViewer = findChild<Viewer*>("viewer");
	connect(mViewer,SIGNAL(itemSelected(boost::any&)),this,SLOT(itemSelectedInViewer(boost::any&)));
}

SDSPlayer::~SDSPlayer()
{

}

void SDSPlayer::stop()
{
	mTimeLineWidget->stop();
}

void SDSPlayer::play()
{
	mTimeLineWidget->play();
}

void SDSPlayer::reset()
{
	// reset the system to use the new irecordedsimulation
	mTimeLineWidget->init(mIRecordedSimulation);

	if (mIRecordedSimulation!=NULL)
	{
		mLabelFilename->setText(QString("Filename: ") + QString::fromStdString(mIRecordedSimulation->filename()));
		mLabelVersion->setText(QString("Version: ") + QString::fromStdString(mIRecordedSimulation->version()));
		mLabelDate->setText(QString("Date: ") + QString::fromStdString(mIRecordedSimulation->date()));

		frameSelected(0);
		mViewer->reset();
	}
	else
	{
		mLabelFilename->setText("");
		mLabelVersion->setText("");
		mLabelDate->setText("");
	}
}

void SDSPlayer::frameSelected(int f)
{
	// std::cout << "SDSPlayer::frameSelected(" << f << ")" << std::endl;

	if (mIRecordedSimulation)
	{
		if (f < mIRecordedSimulation->frames().size())
		{
			IFrame* fr = mIRecordedSimulation->frames()[f];
			if (fr->isValid())
				mViewer->setOrganism(fr->organism());
			mCurrentFrame = fr;
		}
	}
}

void SDSPlayer::playbackRateChanged(int i)
{
	// i in (0,100), default = 50
	// map i to seconds/seconds

	double val = (i - 50)/50.; // (-1,1)
	val += 1; // (0,2)
	val = val*val*val*val;
	val += 0.0001; // non zero rate allowed, sorry!

	mTimeLineWidget->setPlaybackRate(val);
}

void SDSPlayer::selectMode(QString str)
{
	mViewer->setSelectMode(str);
}

// run when item is selected from viewer
void SDSPlayer::itemSelectedInViewer(boost::any& item)
{
	mSelectedItem = item;
}


void SDSPlayer::generateTetTopograph()
{
	// check to see if a vertex has been selected...
	if (mSelectedItem.type() == typeid(Vertex*))
	{
		// then we're all good

		if (mCurrentFrame->isValid())
		{
			Vertex* v = boost::any_cast<Vertex*>(mSelectedItem);
			Organism* o = mCurrentFrame->organism();
			Cell* c = o->getAssociatedCell(v);

			std::map<Tetra*, std::list<Tetra*> > adjList = SDSUtil::getTetTopoGraph(c,o);

			// parse it into a nice format
			std::map<Tetra*, int> tetNumbers;
			std::map<int, Tetra*> tetNumbersInverse;
			std::map<Tetra*, std::list<Tetra*> >::iterator it = adjList.begin();

			int number = 0;
			for(;it!=adjList.end();it++,number++)
			{
				tetNumbers[it->first] = number;
				tetNumbersInverse[number] = it->first;
			}

			// print it out, in .dot acceptable format
			QString graph = "graph G {\n";

			for(int i=0;i<number;i++)
			{
				BOOST_FOREACH(Tetra* t, adjList[tetNumbersInverse[i]])
				{
					int tetN = tetNumbers[t];
					if (tetN>i)
						graph += QString("\t") + QString::number(i) + " -- " + QString::number(tetNumbers[t]) + ";\n";
				}
			}
			graph += "}\n";

			findChild<QTextEdit*>("textEditConsole")->append(graph);
		}
	}
	else
	{
		QMessageBox::warning(this,"Error","No vertex selected!");
	}
}









/*
 * Simulation loading logic.
 */

class IRSRunner: public QThread
{
public:
	IRSRunner(QWidget* p, IRecordedSimulation* i):QThread(p),irs(i){}
	void run()
	{
		irs->startLoading();
	}
	IRecordedSimulation* irs;
};

void SDSPlayer::open()
{
	stop();
	QString filename = QFileDialog::getOpenFileName(this,
		     tr("Open Simulation File"), ".", tr("Simulation Files (*.bin)"));
	if (filename.length()!=0)
	{
		// first check file
		// if it is okay then clear all current data
		// then load it

		std::cout << "opening file: \"" <<  filename.toStdString() << "\"" << std::endl;

		try
		{
			IRecordedSimulation* newSim = new IRecordedSimulation(filename.toStdString());

			// show a progress dialog with a cancel button that loads the frames
			//ProgressDialog* pd = new ProgressDialog(this,filename,newSim);
			//pd->start();
			// start a thread to start loading frames
			QProgressDialog progress("Loading simulation...","Cancel",0,newSim->totalBytes(),this);
			progress.setLabelText(tr("Loading \"%1\" of %2m...")
			                                     .arg(filename).arg((double)newSim->totalBytes()/(1024*1024)));
			progress.setWindowModality(Qt::WindowModal);
			progress.show();

			QThread* th = new IRSRunner(NULL,newSim);
			th->start();
			while(th->isRunning())
			{
				if (progress.wasCanceled())
				{
					newSim->cancelLoading();
					th->quit();
					break;
				}
				else
				{
					progress.setValue(newSim->bytesRead());
				}

				// pause for a second?
				QCoreApplication::processEvents();
			}

			progress.setValue(newSim->bytesRead());
			progress.close();
			delete th;

			// at this point newSim has been successfully loaded
			// so delete mIRecordedSimulation, and redirect the pointer to newSim
			delete this->mIRecordedSimulation;
			this->mIRecordedSimulation = newSim;

			reset();
		}
		catch (std::exception& e)
		{
			std::cout << "sorry, I can't seem to load the file, error msg: " << e.what() << "\n";
		}
	}

}
