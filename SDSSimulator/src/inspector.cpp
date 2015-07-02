/*
 * inspector.cpp
 *
 *  Created on: 27/10/2009
 *      Author: ben
 */

#include "inspector.h"
#include <boost/foreach.hpp>

Inspector::Inspector(QWidget* parent, OrganismViewer* ov, SDSSimulation* sim)
:QDialog(parent),mViewer(ov),mSimulation(sim)
{
	mUI.setupUi(this);
	mListWidgetProperties = findChild<QListWidget*>("listWidgetProperties");
}

void Inspector::clear()
{
	// clear properties list
	// clear current property list and actions
	QListWidgetItem* lw;
	while (lw=mListWidgetProperties->takeItem(0))
	{
		delete lw;
		// XXX: remove associated hook?
	}
}

void Inspector::infoListTetras()
{
	BOOST_FOREACH(Tetra* t,mSimulation->world()->organism()->mesh()->tetras())
	{
		addProperty(PropertyList::makeProperty("tetra",t));
	}
}

void Inspector::infoListFaces()
{
	BOOST_FOREACH(Face* f,mSimulation->world()->organism()->mesh()->faces())
	{
		addProperty(PropertyList::makeProperty("face",f));
	}
}

void Inspector::infoListEdges()
{
	BOOST_FOREACH(Edge* e,mSimulation->world()->organism()->mesh()->edges())
	{
		addProperty(PropertyList::makeProperty("edge",e));
	}
}

void Inspector::infoListVertices()
{
	BOOST_FOREACH(Vertex* v,mSimulation->world()->organism()->mesh()->vertices())
	{
		addProperty(PropertyList::makeProperty("vertex",v));
	}
}

class QPropertyItem: public QListWidgetItem
{
public:
	QPropertyItem(PropertyList::property_type p):QListWidgetItem()
	{
		QString text = QString::fromStdString(p.first);
		if (p.second.type() == typeid(std::string))
		{
			std::string value = boost::any_cast<std::string>(p.second);
			text = text + " : " + QString::fromStdString(value);
		}
		setText(text);
		property = p;
	}

	PropertyList::property_type property;
};

void Inspector::addProperty(PropertyList::property_type p, bool selectItToo)
{
	QPropertyItem* qpi = new QPropertyItem(p);
	mListWidgetProperties->addItem(qpi);
	qpi->setSelected(selectItToo);
	propertiesSelected();
}

void Inspector::propertiesSelected()
{
	// for all the selected properties, draw them in the world view
	// they will persist until they selection changes by stepping forward or choosing other elements

	mViewer->clearAnnotations();

	BOOST_FOREACH(QListWidgetItem* qlwi, mListWidgetProperties->selectedItems())
	{
		PropertyList::property_type property = (static_cast<QPropertyItem*>(qlwi))->property;
		const boost::any& value = property.second;

		if (value.empty() or value.type() == typeid(std::string))
		{
			// ignore it
			qlwi->setSelected(false);
		}
		else if (value.type() == typeid(Tetra*))
		{
			// draw this tetrahedra
			mViewer->addAnnotation(new TetraAnnotation(boost::any_cast<Tetra*>(value)));
		}
		else if (value.type() == typeid(Vertex*))
		{
			mViewer->addAnnotation(new VertexAnnotation(boost::any_cast<Vertex*>(value)));
		}
		else if (value.type() == typeid(Cell*))
		{
			mViewer->addAnnotation(new CellAnnotation(boost::any_cast<Cell*>(value)));
		}
		else if (value.type() == typeid(Edge*))
		{
			mViewer->addAnnotation(new EdgeAnnotation(boost::any_cast<Edge*>(value)));
		}
		else if (value.type() == typeid(Face*))
		{
			mViewer->addAnnotation(new FaceAnnotation(boost::any_cast<Face*>(value)));
		}
		else if (value.type() == typeid(boost::tuple<Vertex*,Vertex*>))
		{
			//mViewer->drawTemp(boost::any_cast<boost::tuple<Vertex*,Vertex*> >(value));
		}
		else if (value.type() == typeid(boost::tuple<Vertex*,Vertex*,Vertex*>))
		{
			//mViewer->drawTemp(boost::any_cast<boost::tuple<Vertex*,Vertex*,Vertex*> >(value));
			mViewer->addAnnotation(new FaceAnnotation(boost::any_cast<boost::tuple<Vertex*,Vertex*,Vertex*> >(value)));
		}
		else if (value.type() == typeid(boost::tuple<Vertex*,Vertex*,Vertex*,Vertex*>))
		{
			//mViewer->drawTemp(boost::any_cast<boost::tuple<Vertex*,Vertex*,Vertex*,Vertex*> >(value));
		}
		else if (value.type() == typeid(Vector3d))
		{
			//mViewer->drawTempPoint(boost::any_cast<Vector3d>(value));
		}
		else if (value.type() == typeid(boost::tuple<Vector3d,Vector3d>))
		{
			//boost::tuple<Vector3d,Vector3d> t = boost::any_cast<boost::tuple<Vector3d,Vector3d> >(value);
			//mViewer->drawTempVec(t.get<0>(),t.get<1>());
		}
		else
		{
			std::cout << "System cannot display object of this type.\n";
		}
	}

	mViewer->update();
}
