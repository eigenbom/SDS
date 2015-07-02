/*
 * timelinewidget.h
 *
 * Presents a "timeline" view of the current RecordedSimulation's timeline.
 * TimeLineWidget contains
 * - A slider for selecting a frame
 * - Marks on the slider that indicate frame positions in a timeline
 * - Other annotations the indicate the time of certain events
 *
 * TimeLineWidget sends a signal when a frame is selected that indicates which frame was selected.
 * TimeLineWidget receives gotoFrame(i) messages that sets the slider position accordingly.
 *
 *  Created on: 23/03/2009
 *      Author: ben
 */

#ifndef TIMELINEWIDGET_H_
#define TIMELINEWIDGET_H_

#include <QWidget>
#include <QSlider>
#include "recordedsimulation.h"

/// A Custom Slider class
class TimeLineSlider: public QSlider
{
	Q_OBJECT

public:
	TimeLineSlider(QWidget* parent);

	void test(std::vector<float>& frametimes);
	void init(IRecordedSimulation *irs = NULL);

	int frame();

public slots:
	void selectFrame(int frameNumber);
	void nextFrame();

signals:
	void frameSelected(int frameNumber);

protected:
	void paintEvent ( QPaintEvent * event );
	void mousePressEvent( QMouseEvent* event );
	void mouseMoveEvent( QMouseEvent* event );
	void resizeEvent (QResizeEvent* event);
	void keyPressEvent (QKeyEvent* ev);

private:
	int mousePosToNearestFrame(int x);

	// an ordered list of frame times, used to draw the tick marks at the proper spots
	std::vector<float> mFrameTimes;
	// a list of event times and types
	struct Event
	{
		static const int MAX_TYPES = 3;
		int type; // 0,1,2
		int frame;
	};
	std::vector<Event> mEventTimes;

	// derived:
	float mDuration;
	// currentFrame
	int mCurrentFrame;

	// appearance
	static const int MIN_HEIGHT = 20;
	static const float PROPORTION_OF_HEIGHT_GIVEN_TO_SLIDER = .6;
	static const float PROPORTION_OF_HEIGHT_GIVEN_TO_EACH_EVENT = (1 - .6)/Event::MAX_TYPES;

	// derived
	float mSliderHeight;
	float mEventHeight;
};



class TimeLineWidget: public QWidget
{
	Q_OBJECT

public:
	TimeLineWidget(QWidget* parent);
	~TimeLineWidget();

	/// (re-)initialise the timeline view with new simulation data
	/// if irs==NULL then disable the widget
	void init(IRecordedSimulation* irs=NULL);
	void test();

public slots:
	void gotoFrame(int frameNumber);

	// interfaces to the control buttons
	void play();
	void stop();
	void rewind();
	void setPlaybackRate(double r){mPlaybackRate = r;}

protected slots:
	void frameSelectedSlot(int f);
	void playNextFrame();

signals:
	void frameSelected(int frameNumber);

protected:
	TimeLineSlider* mSlider;
	bool mIsPlaying;
	std::vector<float> mFrameTimes;

	double mPlaybackRate; // real seconds = PLAYBACKRATE * simulation seconds
};

#endif /* TIMELINEWIDGET_H_ */
