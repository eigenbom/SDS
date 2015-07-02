/*
 * organismwidget.cpp
 *
 *  Created on: 21/09/2009
 *      Author: ben
 */

#include "organismwidget.h"
#include "organismviewer.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalMapper>
#include "organism.h"
#include "processmodel.h"
#include "cell.h"

OrganismWidget::OrganismWidget(QWidget* parent)
:QWidget(parent),mComboBoxMorphogens(NULL)
{
	ui.setupUi(this);

	mOV = findChild<OrganismViewer*>("viewer");
	connect(mOV,SIGNAL(cellSelected(Cell*)),this,SIGNAL(cellSelected(Cell*)) );
	connect(mOV,SIGNAL(vertexSelected(Vertex*)),this,SIGNAL(vertexSelected(Vertex*)) );
	connect(mOV,SIGNAL(edgeSelected(Edge*)),this,SIGNAL(edgeSelected(Edge*)) );
	connect(mOV,SIGNAL(faceSelected(Face*)),this,SIGNAL(faceSelected(Face*)) );

	mComboBoxMorphogens = findChild<QComboBox*>("comboBoxMorphogens");
}

void OrganismWidget::setWorld(OWorld* o)
{
	mOV->setWorld(o);
	updateMorphogenComboBox();
}

void OrganismWidget::selectMorphogen(QString id)
{
	if (id=="none")
		mOV->selectMorphogen(-1);
	else if (id.startsWith("m"))
		mOV->selectMorphogen(id.remove(0,1).toInt(NULL));
	else if (id.startsWith("v"))
		mOV->selectVariable(id.remove(0,1).toInt(NULL));
}

void OrganismWidget::updateMorphogenComboBox()
{
	if (mComboBoxMorphogens==NULL) return;
	mComboBoxMorphogens->clear();

	OWorld* ow = mOV->getWorld();
	if (ow!=NULL)
	{
		Organism* o = ow->organism();
		if (o!=NULL)
		{
			ProcessModel* pm = o->processModel();
			if (pm!=NULL)
			{
				mComboBoxMorphogens->insertItem(0, QString("none"));

				// o is !NULL
				int nm = pm->numMorphogens();
				for(int i=0;i<nm;i++)
					mComboBoxMorphogens->insertItem(1+i, QString("m") + QString::number(i));

				int nv = pm->numVars();
				for(int i=0;i<nv;i++)
					mComboBoxMorphogens->insertItem(1+i+nm, QString("v") + QString::number(i));
			}
		}
	}
}
