#include "CalculateDifference_factory.h"
#include "CalculateDifference.h"

CalculateDiffFactory::CalculateDiffFactory()
{
	editSample = new QAction(QIcon(":/images/icon_Diff.png"),"Calculate Difference between begin and end teeth by GaoY", this);
	actionList << editSample;	
	foreach(QAction *editAction, actionList)
		editAction->setCheckable(true); 	
}
	
//gets a list of actions available from this plugin
QList<QAction *> CalculateDiffFactory::actions() const
{
	return actionList;
}

//get the edit tool for the given action
MeshEditInterface* CalculateDiffFactory::getMeshEditInterface(QAction *action)
{
	if(action == editSample)
	{
		return new CalculateDiffPlugin();
	} else assert(0); //should never be asked for an action that isnt here
}

QString CalculateDiffFactory::getEditToolDescription(QAction *)
{
	return CalculateDiffPlugin::Info();
}

MESHLAB_PLUGIN_NAME_EXPORTER(CalculateDiffFactory)
