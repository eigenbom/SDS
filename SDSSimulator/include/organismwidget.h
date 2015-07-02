/*
 * organismwidget.h
 *
 *  Created on: 21/09/2009
 *      Author: ben
 */

#ifndef ORGANISMWIDGET_H_
#define ORGANISMWIDGET_H_

#include "oworld.h"

#include <QWidget>
#include <QHBoxLayout>
#include "ui_organismwidget.h"

class OrganismWidget: public QWidget
{
	Q_OBJECT

public:
	OrganismWidget(QWidget* parent = NULL);

	void setWorld(OWorld* o);

	/**
	 * The signals emitted by the organismwidget include selected elements.
	 * Depending on the selection mode, different types of elements can be selected.
	 */
	signals:
	void cellSelected(Cell*);
	void vertexSelected(Vertex*);
	void edgeSelected(Edge*);
	void faceSelected(Face*);

	protected slots:
	// void morphogenButtonToggled(const QString& id);
	void selectMorphogen(QString id);

private:
	// void updateMorphogenButtons();
	void updateMorphogenComboBox();

private:
	Ui::organismWidget ui;
	OrganismViewer* mOV;
	// QHBoxLayout* mMorphogenButtons;
	QComboBox* mComboBoxMorphogens;
};

#endif /* ORGANISMWIDGET_H_ */
