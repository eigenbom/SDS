/*
 * inspector.h
 *
 *  Created on: 27/10/2009
 *      Author: ben
 */

#ifndef INSPECTOR_H_
#define INSPECTOR_H_

#include <QDialog>
#include <QWidget>

#include "ui_inspector.h"

#include "propertylist.h"
#include "organismviewer.h"
#include "sdssimulation.h"

class Inspector: public QDialog
{
	Q_OBJECT
public:
	Inspector(QWidget* parent, OrganismViewer* ov, SDSSimulation* sim);
	void addProperty(PropertyList::property_type property, bool selectItToo = false);

public slots:
	void clear();
	void infoListTetras();
	void infoListFaces();
	void infoListEdges();
	void infoListVertices();
	void propertiesSelected();

private:
	Ui::Inspector mUI;
	QListWidget* mListWidgetProperties;
	SDSSimulation* mSimulation;
	OrganismViewer* mViewer;
};

#endif /* INSPECTOR_H_ */
