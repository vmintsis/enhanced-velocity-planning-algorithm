#ifndef _ServiceProvider_H_
#define _ServiceProvider_H_

namespace eei
{


class ServiceProvider
{
	enum SP_State
	{
		SP_Tracked	= 0x0001,
		SP_Advised	= 0x0002,
		SP_Compliant= 0x0004,
		SP_AdvisedAndCompliant = SP_Advised | SP_Compliant,

		SP_Mask		= 0xFFFF
	};

	ServiceProvider();
public:
	~ServiceProvider();

	static ServiceProvider &instance();
	
	A2KSectionInf f_AKIInfNetGetSectionANGInf(int idSec);
	int f_AKIVehStateGetNbVehiclesSection(int idSec, bool considerAllSegments);
	int f_AKIVehStateGetNbVehiclesJunction(int idJunction);
	int f_AKIInfNetGetNbTurnsInNode(int idJunction);
	int f_AKIInfNetGetDestinationSectionInTurn(int idJnction, int turn_index);
	int f_AKIInfNetGetOriginSectionInTurn(int idJnction, int turn_index);
	A2KTurnInf f_AKIInfNetGetTurnInfo(int idnode, int index);
	A2KTurnInf f_AKIInfNetGetTurnInf(int aid);
	InfVeh f_AKIVehStateGetVehicleInfSection(int aidSec, int indexveh);
	InfVeh f_AKIVehStateGetVehicleInfJunction(int ajunction, int indexveh);

	double f_ECIGetStartingTimePhase(int idJunction);
	int f_ECIGetCurrentStateofSignalGroup(int idJunction, int idSignalGroup);
	int f_ECIGetCurrentPhase(int idJunction);
	int f_ECIGetDurationsPhase(int idJunction,int idPhase,double timeSta,double *dur, double *max, double *min);
	int f_ECIGetNumberPhases(int idJunction);

	int	f_AKIVehSetAsTracked(int aidVeh);
	int	f_AKIVehSetAsNoTracked(int aidVeh);

	int	f_AKIVehTrackedModifySpeed(int aidVeh, double newSpeed);
	StaticInfVeh f_AKIVehGetStaticInf(int aidVeh);

	int f_ECIGetNumberCurrentControl();
	int f_ECIGetControlType(int idJunction);

	int f_AKIInfNetGetUnits();
	InfVeh f_AKIVehTrackedGetInf(int aidVeh);

	unsigned int f_ANGConnGetScenarioId();

	const char *f_ANGConnGetObjectNameA(int idObject);

	double f_ECIGetSignalGroupGreenDuration(int idJunction, int signalGroup, double timeSta );
	double f_ECIGetSignalGroupYellowDuration(int idJunction, int signalGroup, double timeSta );
	double f_ECIGetSignalGroupRedDuration(int idJunction, int signalGroup, double timeSta );

	int f_ECIGetNbSignalGroupsPhaseofJunction(int idJunction, int aidphase, double timeSta);
	int f_ECIGetSignalGroupPhaseofJunction(int idJunction, int aidphase, int indexSG, double timeSta);

	const unsigned short *f_AKIVehGetVehTypeName(int vehTypePos);
	void f_AKIDeleteUNICODEString( const unsigned short *string );
	int f_AKIVehGetNbVehTypes();

	// added interface functions for advised vehicles
	int	f_SetAsAdvised(int aidVeh);
	int	f_SetAsNotAdvised(int aidVeh);
	int	f_SetAsCompliant(int aidVeh);
	int	f_SetAsNotCompliant(int aidVeh);

	double	f_GetQueueExitSpeed();
};

#define SP ServiceProvider::instance()

} // namespasce

#endif

