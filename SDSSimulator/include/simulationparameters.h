/*
 *  Created on: 27/10/2009
 *      Author: ben
 */

#ifndef SIMULATIONPARAMETERS_H_
#define SIMULATIONPARAMETERS_H_

#include <QDialog>
#include <QWidget>
#include <QTableWidget>
#include <QString>

#include "ui_simulationparameters.h"

#include "sdssimulation.h"

class SimulationParameters: public QDialog
{
	Q_OBJECT

public:
	SimulationParameters(QWidget* parent, SDSSimulation* sds);

public slots:
	// use the input parameters to modify the current simulation parameters
	void updateParameters();

	// extract the parameters from the simulation and display them
	void populateParameters();

private:
	void addRow(QTableWidget* table, QString paramname, QString value);
	void resetTable(QTableWidget* table);
	QString getValue(QTableWidget* table, QString paramname);


	Ui::SimulationParameters mUI;
	SDSSimulation* mSimulation;
	QTableWidget *mSceneTable, *mProcessModelTable;
};

#endif /* SIMULATIONPARAMETERS_H_ */
