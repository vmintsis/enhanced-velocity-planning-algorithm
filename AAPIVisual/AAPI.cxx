#include "Precompiled.h"
//#include "EnergyEfficientIntersection.h"
#include "EEIManager.h"
#include "AAPI.h"


using namespace eei;
// Procedures could be modified by the user

int AAPILoad()
{
//	AKIPrintString("LOAD");
	CLog::log()->init("C:\\Users\\VMintsis\\EEILog.txt");
//	return EnergyEfficientIntersection::instance()->LoadScenario();

	return EEIManager::instance().LoadScenario();
}

int AAPIInit()
{	
//	AKIPrintString("\tInit");
	EEI_ptr eei = EEIManager::instance().add("EEI");

//	EnergyEfficientIntersection *pEEI = EnergyEfficientIntersection::instance();
//	EnergyEfficientIntersection *pEEI = EEIManager::instance().get("EEI");
	EnergyEfficientIntersection *pEEI = eei;

	pEEI->setSectionIDstr("328");
	pEEI->setJunctionID(334);
	pEEI->setSignalGroup(1);
	pEEI->setRangeToJunction(210, 190);


	int r = pEEI->Initialize();
	ANGConnEnableVehiclesInBatch(true);
	return r;
}

int AAPIManage(double time, double timeSta, double timTrans, double acicle)
{
//	AKIPrintString("\tManage");
//	return EnergyEfficientIntersection::instance()->Step(time, timeSta, timTrans, acicle);
	return EEIManager::instance().get("EEI")->Step(time, timeSta, timTrans, acicle);;
}

int AAPIPostManage(double time, double timeSta, double timTrans, double acicle)
{
//	AKIPrintString("\tPostManage");
//	return EnergyEfficientIntersection::instance()->PostStep(time, timeSta, timTrans, acicle);
	return EEIManager::instance().get("EEI")->PostStep(time, timeSta, timTrans, acicle);
}

int AAPIFinish()
{
//	AKIPrintString("\tFinish");
//	return EnergyEfficientIntersection::instance()->Finish();
	return EEIManager::instance().get("EEI")->Finish();
}

int AAPIUnLoad()
{
//	AKIPrintString("UNLOAD");
//	return EnergyEfficientIntersection::instance()->UnloadScenario();
	return EEIManager::instance().get("EEI")->UnloadScenario();
}

int AAPIExitVehicleSection(int idveh, int idsection, double time)
{
//	return EnergyEfficientIntersection::instance()->ExitVehicleSection(idveh, idsection, time);
	return EEIManager::instance().get("EEI")->ExitVehicleSection(idveh, idsection, time);
}