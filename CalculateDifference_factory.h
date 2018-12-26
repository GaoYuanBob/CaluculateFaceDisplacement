#pragma once

#include <QObject>
#include <common/interfaces.h>

class CalculateDiffFactory : public QObject, public MeshEditInterfaceFactory
{
	Q_OBJECT
	MESHLAB_PLUGIN_IID_EXPORTER(MESH_EDIT_INTERFACE_FACTORY_IID)
	Q_INTERFACES(MeshEditInterfaceFactory)

public:
	CalculateDiffFactory();
	virtual ~CalculateDiffFactory() { delete editSample; }

	//gets a list of actions available from this plugin
	QList<QAction *> actions() const override;
	
	//get the edit tool for the given action
	MeshEditInterface* getMeshEditInterface(QAction *) override;
    
	//get the description for the given action
	QString getEditToolDescription(QAction *) override;
	
private:
	QList <QAction *> actionList;
	
	QAction *editSample;
};