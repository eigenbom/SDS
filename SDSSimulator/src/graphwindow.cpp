#include "graphwindow.h"
#include "ui_graphwindow.h"

#include <QPainter>
#include <QBrush>
#include <QColor>

#include <cmath>
#include <cfloat>
#include <cassert>

GraphWindow::GraphWindow(std::vector<double>* watch, QWidget* qw)
:QDialog(qw)
,mDataToWatch(watch)
,mMaxValue(-DBL_MAX)
,mMinValue(DBL_MAX)
,mMaxNumItemsToShow(1000)
,mGraphicsView(NULL)
{
	Ui_Dialog gw;
	gw.setupUi(this);

	mGraphicsView = findChild<QGraphicsView*>("graphicsView");
	mGraphicsView->setScene(&mScene);
	mGraphicsView->scale(1,-1);

	assert(mGraphicsView);
}

/*
void GraphWindow::paintEvent(QPaintEvent *event)
{
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(event->rect(), QBrush(QColor(64, 32, 64)));
	painter.end();
}
*/

void GraphWindow::newData()
{
	// add the last point of mDataToWatch to the scene
	int itemNumber = mDataToWatch->size() - 1;
	double value = (*mDataToWatch)[itemNumber];
	if (value > mMaxValue)
		mMaxValue = value;
	if (value < mMinValue)
		mMinValue = value;

	// mScene.addLine(QLineF(QPointF(itemNumber,0),QPointF(itemNumber,value)));
	mScene.addRect(QRectF(itemNumber,0,1,value),Qt::NoPen,QBrush(Qt::blue));

	if (mMaxValue > mMinValue)
		mGraphicsView->fitInView(QRectF(-1 + std::max(0,itemNumber-mMaxNumItemsToShow),mMinValue,mMaxNumItemsToShow,mMaxValue-mMinValue));
}
