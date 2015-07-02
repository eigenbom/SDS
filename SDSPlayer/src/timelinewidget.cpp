/*
 * timelinewidget.cpp
 *
 *  Created on: 23/03/2009
 *      Author: ben
 */

#include "timelinewidget.h"

#include <QVBoxLayout>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QTimer>

#include <list>
#include <algorithm>
#include <boost/foreach.hpp>
#include <iostream>

#include "random.h"

#define DEBUG_VIEW


const int TimeLineSlider::Event::MAX_TYPES;
const int TimeLineSlider::MIN_HEIGHT;
const float TimeLineSlider::PROPORTION_OF_HEIGHT_GIVEN_TO_SLIDER;
const float TimeLineSlider::PROPORTION_OF_HEIGHT_GIVEN_TO_EACH_EVENT;

Random TIMELINEWIDGET_sRandom;
TimeLineSlider::TimeLineSlider(QWidget* parent)
:QSlider(Qt::Horizontal,parent)
{
	QSlider::setMinimumHeight(MIN_HEIGHT);
}

void TimeLineSlider::test(std::vector<float>& frametimes)
{
	mEventTimes.clear();
	mFrameTimes.assign(frametimes.begin(),frametimes.end());
	init();
}

void TimeLineSlider::init(IRecordedSimulation *irs)
{
	mCurrentFrame = 0;

	if (irs==NULL)
	{
		mFrameTimes.clear();
		setDisabled(true);
	}
	else // irs!=NULL
	{
		setDisabled(false);
		mFrameTimes.clear();
		BOOST_FOREACH(IFrame* f, irs->frames())
		{
			mFrameTimes.push_back(f->time());
		}
	}

	if (!mFrameTimes.empty())
	{
		mDuration = mFrameTimes[mFrameTimes.size()-1] - mFrameTimes[0];
	}
}

int TimeLineSlider::frame()
{
	return mCurrentFrame;
}

void TimeLineSlider::selectFrame(int frameNumber)
{
	mCurrentFrame = frameNumber;
	int val = (int)(((mFrameTimes[mCurrentFrame]-mFrameTimes[0])/mDuration) * (maximum()-1-minimum())) + minimum();
	this->setValue(val);
	emit frameSelected(mCurrentFrame);
}

void TimeLineSlider::nextFrame()
{
	if (mCurrentFrame<mFrameTimes.size()-1)
		selectFrame(mCurrentFrame+1);
}

void TimeLineSlider::paintEvent ( QPaintEvent * event )
{
	QPainter painter(this);

	const QRect& r = rect();
	painter.setPen(QPen(palette().dark().color()));
	//painter.drawRect(r.x(),r.y(),r.width()-1,r.height()-1);
	painter.drawRect(r.x(),r.y(),r.width()-1,mSliderHeight);

	if (not this->isEnabled())
	{
		painter.fillRect(QRect(r.x(),r.y(),r.width(),r.height()),palette().brush(QPalette::Dark));
		return;
	}
	// else draw the thing


	// draw the tick marks manually
	int h = this->height();

	// compute pixels/second value
	float pps = (maximum()-1-minimum())/mDuration;

	// draw a tick mark at each frametime
	/*
	BOOST_FOREACH(float time, mFrameTimes)
	{
		float dtime = time - mFrameTimes[0];
		int x = minimum() + (int)(pps*dtime);
		// painter.drawLine(x,0,x,mSliderHeight/8);
		painter.drawLine(x,0,x,mSliderHeight);
		// painter.drawLine(x,mSliderHeight,x,mSliderHeight-mSliderHeight/8);
	}*/

	// draw a big line at the current frame
	// painter.setPen(QPen(QColor("black"),2));
	int x = (int)((mFrameTimes[mCurrentFrame]-mFrameTimes[0])*pps) + minimum();

	QColor timelineColor = palette().color(QPalette::Highlight);
	timelineColor.setAlpha(100);
	painter.fillRect(QRect(0,0,x,mSliderHeight),QBrush(timelineColor));

	// now draw the event markers
	BOOST_FOREACH(const Event& e, mEventTimes)
	{
		float dtime = mFrameTimes[e.frame] - mFrameTimes[0];
		int x = minimum() + (int)(pps*dtime);

		int bw = 2;//event box-width/2
		QColor color = QColor(0,0,0,150);
		switch(e.type)
		{
			case 0: color.setRed(255); break;
			case 1: color.setBlue(255); break;
			case 2: color.setGreen(255); break;
		}
		painter.fillRect(QRect(x-bw,e.type*mEventHeight + mSliderHeight,bw*2,mEventHeight),QBrush(color));
	}
}

// snap the slider to the closest temporal event
void TimeLineSlider::mousePressEvent( QMouseEvent* event )
{
	if (isEnabled() and event->button() == Qt::LeftButton and !mFrameTimes.empty())
	{
		int frame = mousePosToNearestFrame(event->x());
		if (frame!=mCurrentFrame)
			selectFrame(frame);
		event->accept();
	}
	else QSlider::mousePressEvent(event);
}

void TimeLineSlider::mouseMoveEvent( QMouseEvent* event ){
	if (isEnabled())
	{
		int frame = mousePosToNearestFrame(event->x());
		if (frame!=mCurrentFrame)
			selectFrame(frame);
		event->accept();
	}
}

void TimeLineSlider::resizeEvent (QResizeEvent* event)
{
	QSlider::resizeEvent(event);
	setRange(0,width());
	mSliderHeight = height()*PROPORTION_OF_HEIGHT_GIVEN_TO_SLIDER;
	mEventHeight = height()*PROPORTION_OF_HEIGHT_GIVEN_TO_EACH_EVENT;
}

void TimeLineSlider::keyPressEvent (QKeyEvent* ev)
{
	if (isEnabled())
	{
		if (ev->key()==Qt::Key_Left)
		{
			if (mCurrentFrame>0)
				selectFrame(mCurrentFrame-1);
		}
		else if (ev->key()==Qt::Key_Right)
		{
			if (mCurrentFrame<mFrameTimes.size()-1)
				selectFrame(mCurrentFrame+1);
		}
		else
			QSlider::keyPressEvent(ev);
	}
}

int TimeLineSlider::mousePosToNearestFrame(int x)
{
	float pps = (maximum()-1-minimum())/mDuration;
	float mouseTime = x/pps + mFrameTimes[0];

	// find closest time
	unsigned int frame = 0;
	for(;frame<mFrameTimes.size();frame++)
	{
		float time = mFrameTimes[frame];
		if (mouseTime < time)
			break;
	}

	if (frame==mFrameTimes.size() or frame==(mFrameTimes.size()-1))
		return mFrameTimes.size()-1;
	else // find the closest frame
		return frame;
}

/*********************************************/
/*********************************************/
/*********************************************/
/// TIMELINE WIDGET

TimeLineWidget::TimeLineWidget(QWidget* parent)
:QWidget(parent),mSlider(NULL),mIsPlaying(false),mPlaybackRate(1)
{
	mSlider = new TimeLineSlider(this);
	init();

	connect(mSlider,SIGNAL(frameSelected(int)),this,SLOT(frameSelectedSlot(int)));
	QVBoxLayout* layout = new QVBoxLayout;
	layout->addWidget(mSlider);
	this->setLayout(layout);
	this->show();
}

TimeLineWidget::~TimeLineWidget()
{

	// TODO Auto-generated destructor stub
}

void TimeLineWidget::init(IRecordedSimulation* irs)
{
	if (irs==NULL)
	{
		mFrameTimes.clear();
		setDisabled(true);
	}
	else // irs!=NULL
	{
		setDisabled(false);
		mFrameTimes.clear();
		BOOST_FOREACH(IFrame* f, irs->frames())
		{
			mFrameTimes.push_back(f->time());
		}
	}

	mSlider->init(irs);
}

void TimeLineWidget::test()
{
	const int NUMTIMES = 100;
	float testtimes[NUMTIMES];
	testtimes[0] = 0;
	for(int i=1;i<NUMTIMES;i++)
	{
		testtimes[i] = testtimes[i-1]+0.1*TIMELINEWIDGET_sRandom.getDouble();
	}
	mFrameTimes.assign(testtimes,testtimes+NUMTIMES);

	mSlider->test(mFrameTimes);
}

/*
void TimeLineWidget::init(IRecordedSimulation* irs)
{

}
*/

/* TimeLineWidget playback system.
 *
 * Loop {
 *   select frame
 *   wait for (PLAYBACKRATE*frame.duration) sections
 * }
 *
 */

void TimeLineWidget::play()
{
	if (isEnabled() and !mIsPlaying)
	{
		// start the playback of frames
		mIsPlaying = true;
		playNextFrame();
	}
}

void TimeLineWidget::stop()
{
	mIsPlaying = false;
}

void TimeLineWidget::rewind()
{
	if (isEnabled())
	{
		mSlider->selectFrame(0);
	}
}

void TimeLineWidget::playNextFrame()
{
	if (mIsPlaying)
	{
		mSlider->nextFrame();
		int frame = mSlider->frame();
		if (frame<mFrameTimes.size()-1)
		{
			float frameDuration = mFrameTimes[frame+1] - mFrameTimes[frame];
			QTimer::singleShot((int)(frameDuration*1000*mPlaybackRate),this,SLOT(playNextFrame()));
		}
		else
			stop();
	}
}

void TimeLineWidget::frameSelectedSlot(int f)
{
	emit frameSelected(f);
}

void TimeLineWidget::gotoFrame(int i)
{

}
