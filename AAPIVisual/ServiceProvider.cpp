#include "Precompiled.h"

#include "ServiceProvider.h"

namespace eei
{

//------------------------------------------------------
//
//------------------------------------------------------
ServiceProvider::ServiceProvider() 
{
	LOG("ServiceProvider CTOR\n");
}

ServiceProvider::~ServiceProvider()
{
	LOG("ServiceProvider DTOR\n");
}

//------------------------------------------------------
//
//------------------------------------------------------
ServiceProvider &ServiceProvider::instance()
{
	static ServiceProvider _sp;
	return _sp;
}


A2KSectionInf ServiceProvider::f_AKIInfNetGetSectionANGInf(int idSec)
{
	return AKIInfNetGetSectionANGInf(idSec);
}

int ServiceProvider::f_AKIVehStateGetNbVehiclesSection(int idSec, bool considerAllSegments)
{
	return AKIVehStateGetNbVehiclesSection(idSec, considerAllSegments);
}

int ServiceProvider::f_AKIVehStateGetNbVehiclesJunction(int idJunction)
{
	return AKIVehStateGetNbVehiclesJunction(idJunction);
}

int ServiceProvider::f_AKIInfNetGetNbTurnsInNode(int idJunction)
{
	return AKIInfNetGetNbTurnsInNode(idJunction);
}

int ServiceProvider::f_AKIInfNetGetDestinationSectionInTurn(int idJnction, int turn_index)
{
	return AKIInfNetGetDestinationSectionInTurn(idJnction, turn_index);
}

int ServiceProvider::f_AKIInfNetGetOriginSectionInTurn(int idJnction, int turn_index)
{
	return AKIInfNetGetOriginSectionInTurn(idJnction, turn_index);
}

A2KTurnInf ServiceProvider::f_AKIInfNetGetTurnInfo(int idnode, int index)
{
	return AKIInfNetGetTurnInfo(idnode, index);
}

A2KTurnInf ServiceProvider::f_AKIInfNetGetTurnInf(int aid)
{
	return AKIInfNetGetTurnInf(aid);
}

InfVeh ServiceProvider::f_AKIVehStateGetVehicleInfSection(int aidSec, int indexveh)
{
	return AKIVehStateGetVehicleInfSection(aidSec, indexveh);
}

InfVeh ServiceProvider::f_AKIVehStateGetVehicleInfJunction(int ajunction, int indexveh)
{
	return AKIVehStateGetVehicleInfJunction(ajunction, indexveh);
}

double ServiceProvider::f_ECIGetStartingTimePhase(int idJunction)
{
	return ECIGetStartingTimePhase(idJunction);
}

int ServiceProvider::f_ECIGetCurrentStateofSignalGroup(int idJunction, int idSignalGroup)
{
	return ECIGetCurrentStateofSignalGroup(idJunction, idSignalGroup);
}

int ServiceProvider::f_ECIGetCurrentPhase(int idJunction)
{
	return ECIGetCurrentPhase(idJunction);
}

int ServiceProvider::f_ECIGetDurationsPhase(int idJunction,int idPhase,double timeSta,double *dur, double *_max, double *_min)
{
	

	return ECIGetDurationsPhase(idJunction, idPhase, timeSta, dur, _max, _min);
}

int ServiceProvider::f_ECIGetNumberPhases(int idJunction)
{
	return ECIGetNumberPhases(idJunction);
}

// NOTE: the Set/Get Advised functionality requires manual addition of the GKSimVehicle::AdvisedAtt
//		 integer attribute.
//		An associated ViewStyle should also be defined, where colors are defined for PARAMETER: Advised
//		with values 0-<not tracked, not advised color>, 1-<tracked color>, 2-<advised color>, 3-<tracked and advised color>.
//		Also an associated ViewMode should be defined for ViewStyle PARAMETER: Advised so it can be used to view the color 
//		changes as EEI-controlled vehicles are tracked, advised, etc.
int	ServiceProvider::f_SetAsAdvised(int aidVeh)
{
	int ret = -1;
	void *advised = ANGConnGetAttribute(AKIConvertFromAsciiString("GKSimVehicle::AdvisedAtt"));
	if (advised != NULL)
	{
		int guiID = ANGConnVehGetGKSimVehicleId(aidVeh);
		int adv = ANGConnGetAttributeValueInt(advised, guiID);
		ANGConnSetAttributeValueInt(advised, guiID , (adv | SP_Advised) );
		ret = 0;
	}
	return ret;
}

int	ServiceProvider::f_SetAsNotAdvised(int aidVeh)
{
	int ret = -1;
	void *advised = ANGConnGetAttribute(AKIConvertFromAsciiString("GKSimVehicle::AdvisedAtt"));
	if (advised != NULL)
	{
		int guiID = ANGConnVehGetGKSimVehicleId(aidVeh);
		int adv = ANGConnGetAttributeValueInt(advised, guiID);
		ANGConnSetAttributeValueInt(advised, guiID , (adv & (~SP_Advised)) & SP_Mask );
		ret = 0;
	}
	return ret;
}
int	ServiceProvider::f_SetAsCompliant(int aidVeh)
{
	int ret = -1;
	void *advised = ANGConnGetAttribute(AKIConvertFromAsciiString("GKSimVehicle::AdvisedAtt"));
	if (advised != NULL)
	{
		int guiID = ANGConnVehGetGKSimVehicleId(aidVeh);
		int adv = ANGConnGetAttributeValueInt(advised, guiID);
		ANGConnSetAttributeValueInt(advised, guiID , (adv | SP_Compliant) );
		ret = 0;
	}
	return ret;
}

int	ServiceProvider::f_SetAsNotCompliant(int aidVeh)
{
	int ret = -1;
	void *advised = ANGConnGetAttribute(AKIConvertFromAsciiString("GKSimVehicle::AdvisedAtt"));
	if (advised != NULL)
	{
		int guiID = ANGConnVehGetGKSimVehicleId(aidVeh);
		int adv = ANGConnGetAttributeValueInt(advised, guiID);
		ANGConnSetAttributeValueInt(advised, guiID , (adv & (~SP_Compliant)) & SP_Mask );
		ret = 0;
	}
	return ret;
}

int	ServiceProvider::f_AKIVehSetAsTracked(int aidVeh)
{
	void *advised = ANGConnGetAttribute(AKIConvertFromAsciiString("GKSimVehicle::AdvisedAtt"));
	if (advised != NULL)
	{
		int guiID = ANGConnVehGetGKSimVehicleId(aidVeh);
		int adv = ANGConnGetAttributeValueInt(advised, guiID);
		ANGConnSetAttributeValueInt(advised, guiID , (adv | SP_Tracked) );
	}
	return AKIVehSetAsTracked(aidVeh);
}

int	ServiceProvider::f_AKIVehSetAsNoTracked(int aidVeh)
{
	void *advised = ANGConnGetAttribute(AKIConvertFromAsciiString("GKSimVehicle::AdvisedAtt"));
	{
		int guiID = ANGConnVehGetGKSimVehicleId(aidVeh);
		int adv = ANGConnGetAttributeValueInt(advised, guiID);
		ANGConnSetAttributeValueInt(advised, guiID , (adv & (~SP_Tracked)) & SP_Mask );
	}
	return AKIVehSetAsNoTracked(aidVeh);
}

int	ServiceProvider::f_AKIVehTrackedModifySpeed(int aidVeh, double newSpeed)
{
	return AKIVehTrackedModifySpeed(aidVeh, newSpeed);
}

StaticInfVeh ServiceProvider::f_AKIVehGetStaticInf(int aidVeh)
{
	static std::map<int, StaticInfVeh> vmp;

	std::map<int, StaticInfVeh>::iterator v = vmp.find(aidVeh);
	StaticInfVeh svi;
	if (v == vmp.end())
		svi = vmp[aidVeh] = AKIVehGetStaticInf(aidVeh);
	else
		svi = v->second;
	return svi;
}

int ServiceProvider::f_ECIGetNumberCurrentControl()
{
	return ECIGetNumberCurrentControl(0);
}

int ServiceProvider::f_ECIGetControlType(int idJunction)
{
	return ECIGetControlType(idJunction);
}

unsigned int ServiceProvider::f_ANGConnGetScenarioId()
{
	return ANGConnGetScenarioId();
}

int ServiceProvider::f_AKIInfNetGetUnits()
{
	return AKIInfNetGetUnits();
}

InfVeh ServiceProvider::f_AKIVehTrackedGetInf(int aidVeh)
{
	return AKIVehTrackedGetInf(aidVeh);
}

extern "C" char *ANGConnGetObjectNameA(int);

const char *ServiceProvider::f_ANGConnGetObjectNameA(int idObject)
{
	return ANGConnGetObjectNameA(idObject);
}

double ServiceProvider::f_ECIGetSignalGroupGreenDuration(int idJunction, int signalGroup, double timeSta )
{
	return ECIGetSignalGroupGreenDuration(idJunction, signalGroup, timeSta );
}

double ServiceProvider::f_ECIGetSignalGroupYellowDuration(int idJunction, int signalGroup, double timeSta )
{
	return ECIGetSignalGroupYellowDuration(idJunction, signalGroup, timeSta );
}

double ServiceProvider::f_ECIGetSignalGroupRedDuration(int idJunction, int signalGroup, double timeSta )
{
	return ECIGetSignalGroupRedDuration(idJunction, signalGroup, timeSta );
}

int ServiceProvider::f_ECIGetNbSignalGroupsPhaseofJunction(int idJunction, int aidphase, double timeSta)
{
	return ECIGetNbSignalGroupsPhaseofJunction(idJunction, aidphase, timeSta);
}

int ServiceProvider::f_ECIGetSignalGroupPhaseofJunction(int idJunction, int aidphase, int indexSG, double timeSta)
{
	return ECIGetSignalGroupPhaseofJunction(idJunction, aidphase, indexSG, timeSta);
}

const unsigned short *ServiceProvider::f_AKIVehGetVehTypeName(int vehTypePos)
{
	return AKIVehGetVehTypeName(vehTypePos);
}
void ServiceProvider::f_AKIDeleteUNICODEString( const unsigned short *string )
{
	AKIDeleteUNICODEString(string );
}

int ServiceProvider::f_AKIVehGetNbVehTypes()
{
	return AKIVehGetNbVehTypes();
}

double	ServiceProvider::f_GetQueueExitSpeed()
{
	double qexspeed = 0.0;
	void *qexitSpeed = ANGConnGetAttribute(AKIConvertFromAsciiString("GKExperiment::queueLeavingSeepAtt"));
	if (qexitSpeed != NULL)
	{
		unsigned int experimentID = ANGConnGetExperimentId();
		qexspeed = ANGConnGetAttributeValueDouble(qexitSpeed, experimentID);
	}
	return qexspeed;
}


} // namespace

