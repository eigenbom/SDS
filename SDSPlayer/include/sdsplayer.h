#ifndef SDSPLAYER_H
#define SDSPLAYER_H

#include <QtGui/QMainWindow>
#include "ui_sdsplayer.h"
#include "timelinewidget.h"
#include "recordedsimulation.h"
#include "viewer.h"
#include "frame.h"

#include <boost/any.hpp>

class SDSPlayer : public QMainWindow
{
    Q_OBJECT

public:
    SDSPlayer(QWidget *parent = 0);
    ~SDSPlayer();

public slots:
    void stop();
	void play();
	void reset();

public slots:
	void frameSelected(int f);
	void open();
	void playbackRateChanged(int);

	// change the selection mode, also notifies viewer
	void selectMode(QString);

	// run when item is selected from viewer
	void itemSelectedInViewer(boost::any&);

	void generateTetTopograph();

private:
	// ui
    Ui::SDSPlayerClass ui;
    TimeLineWidget* mTimeLineWidget;
    QLabel* mLabelFilename;
	QLabel* mLabelVersion;
	QLabel* mLabelDate;
	Viewer* mViewer;

	// simulation
    IRecordedSimulation* mIRecordedSimulation;
    IFrame* mCurrentFrame;

    // selection/etc...
    boost::any mSelectedItem;

};

#endif // SDSPLAYER_H
