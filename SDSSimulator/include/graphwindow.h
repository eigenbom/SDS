#ifndef GRAPHWINDOW_H
#define GRAPHWINDOW_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPaintEvent>

#include <vector>

class GraphWindow: public QDialog
{
	Q_OBJECT

	public:

	GraphWindow(std::vector<double>* watch, QWidget* qw = NULL);
	void setMaxNumItemsToShow(int m){mMaxNumItemsToShow = m;}

	//protected:
	//     void paintEvent(QPaintEvent *event);
	public slots:
		void newData();

	protected:
	std::vector<double>* mDataToWatch;
	double mMaxValue, mMinValue;
	int mMaxNumItemsToShow;

	QGraphicsView* mGraphicsView;
	QGraphicsScene mScene;


};

#endif
