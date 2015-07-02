/*
 * comments.cpp
 *
 *  Created on: 27/10/2009
 *      Author: ben
 */

#include "simulationparameters.h"

#include <QTabWidget>
#include <QString>
#include <QTextStream>

#include "processmodel.h"
#include "physics.h"
#include "simulator.h"

SimulationParameters::SimulationParameters(QWidget* parent, SDSSimulation* sds)
:QDialog(parent),mSimulation(sds)
{
	mUI.setupUi(this);
	mSceneTable = findChild<QTabWidget*>("tabWidget")->widget(0)->findChild<QTableWidget*>("sceneTableWidget");
	mProcessModelTable = findChild<QTabWidget*>("tabWidget")->widget(1)->findChild<QTableWidget*>("processModelTableWidget");
}

void SimulationParameters::updateParameters()
{
	mSimulation->setStepSize(getValue(mSceneTable,"dt").toDouble());
	mSimulation->setTime(getValue(mSceneTable,"t").toDouble());

	Physics::kD = getValue(mSceneTable,"kD").toDouble();
	Physics::kSM = getValue(mSceneTable,"kSM").toDouble();
	Physics::kV = getValue(mSceneTable,"kV").toDouble();
	Physics::kDamp = getValue(mSceneTable,"kDamp").toDouble();
	Physics::GRAVITY = getValue(mSceneTable,"GRAVITY").toDouble();
	Physics::DENSITY = getValue(mSceneTable,"DENSITY").toDouble();
	Physics::VISCOSITY = getValue(mSceneTable,"VISCOSITY").toDouble();

	mSimulation->setCollisionInterval(getValue(mSceneTable,"Collision Interval").toInt());

	// bounding box...
	QString s = getValue(mSceneTable,"bounds");
	double x, y, z, x2, y2, z2;
	QTextStream t(&s);
	t >> x >> y >> z >> x2 >> y2 >> z2;
	mSimulation->world()->setBounds(AABB(x,y,z,x2,y2,z2));

	// add process model params
	ProcessModel* pm = mSimulation->world()->organism()->processModel();
	if (pm!=NULL)
	{
		BOOST_FOREACH(std::string param, pm->parameters())
		{
			double v = getValue(mProcessModelTable,QString::fromStdString(param)).toDouble();
			pm->set(param,v);
			//std::cout << "Setting \"" << param << "\" to " << v << "\n";
		}
	}

	static_cast<Simulator*>(this->parent())->redraw();
}

void SimulationParameters::populateParameters()
{
	resetTable(mSceneTable);
	resetTable(mProcessModelTable);

	// add global sim params
	addRow(mSceneTable,"dt",QString::number(mSimulation->stepSize()));
	addRow(mSceneTable,"t",QString::number(mSimulation->t()));
	addRow(mSceneTable,"kD",QString::number(Physics::kD));
	addRow(mSceneTable,"kSM",QString::number(Physics::kSM));
	addRow(mSceneTable,"kV",QString::number(Physics::kV));
	addRow(mSceneTable,"kDamp",QString::number(Physics::kDamp));
	addRow(mSceneTable,"GRAVITY",QString::number(Physics::GRAVITY));
	addRow(mSceneTable,"DENSITY",QString::number(Physics::DENSITY));
	addRow(mSceneTable,"VISCOSITY",QString::number(Physics::VISCOSITY));
	addRow(mSceneTable,"Collision Interval",QString::number(mSimulation->getCollisionInterval()));

	// bounding box...
	AABB b = mSimulation->world()->bounds();
	QString bounds;
	bounds.sprintf("%f %f %f %f %f %f", b[0], b[1], b[2], b[3], b[4], b[5]);
	addRow(mSceneTable,"bounds",bounds);

	// add process model params
	ProcessModel* pm = mSimulation->world()->organism()->processModel();
	if (pm!=NULL)
	{
		BOOST_FOREACH(std::string param, pm->parameters())
		{
			addRow(mProcessModelTable,QString::fromStdString(param),QString::number(pm->get(param)));
		}
	}
}

void SimulationParameters::addRow(QTableWidget* table, QString paramname, QString value)
{
	table->insertRow(table->rowCount());
	table->setItem(table->rowCount()-1,0,new QTableWidgetItem(paramname));
	table->setItem(table->rowCount()-1,1,new QTableWidgetItem(value));
}

void SimulationParameters::resetTable(QTableWidget* table)
{
	table->clear();
	table->setColumnCount(2);
	table->setRowCount(0);
	QStringList header;
	header << "param" << "value";
	table->setHorizontalHeaderLabels(header);
}

QString SimulationParameters::getValue(QTableWidget* table, QString paramname)
{
	for(int i=0;i<table->rowCount();i++)
	{
		if (table->item(i,0)->text()==paramname) return table->item(i,1)->text();
	}
	return QString::null;
}
