#include "Precompiled.h"
//#include "EnergyEfficientIntersection.h"
#include "EEIManager.h"
#include "AAPI.h"

#include "Settings.h"
#include "Statistics.h"

using namespace eei;

// Procedures could be modified by the user

//static CLog _dbg;
//CLog &dbg = _dbg;

int AAPILoad()
{
//	AKIPrintString("LOAD");
//	dbg.init(logs_location("\\EEIRefCnt.txt").c_str());
	CLog::log()->init(logs_location("\\EEILog.txt").c_str());

	return EEIManager::instance().LoadScenario();
}

int AAPIInit()
{	
	USES_CONVERSION;
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

//	AKIPrintString("\tInit");

	// get the vehicle type names and add them to the dialog intially selected
	// to allow the user to select which types will be tracked

//	CLog::log("default")->setEnabled(false);


	VehicleTypes vnames;
	int nTypes = SP.f_AKIVehGetNbVehTypes();
	fLogMsg("Vehicle types = %d\n", nTypes);
	for (int i=1; i<=nTypes; i++)
	{
		const unsigned short *pVName = SP.f_AKIVehGetVehTypeName(i);
		if (pVName)
		{
			char *pName = W2A((LPCWSTR)pVName);
			fLogMsg("Adding %d: %s\n", i, pName);
			vnames.add(pName, i);
			SP.f_AKIDeleteUNICODEString( pVName );
//			delete [] pVName;   // <+++ Memory Leak!!!
		}
	}
	EEIManager::instance().vehTypes() = vnames;

//	DebugBreak();

	// create a new GUI dialog, and pass our parameters so it can also check 
	// the current settings file, or whatever settings file the user may select
	EEI_Dlg dlg;
	dlg.SetupParameters(EEIManager::instance().getSettingsPath().c_str(), EEIManager::instance().vehTypes());

	std::string log_folder = logs_location("\\Intersection_Logs");

	if (dlg.begin() == dlg.end())
	{
		EnergyEfficientIntersection *pEEI = EEIManager::instance().add("EEI");
		pEEI->initLogging(log_folder.c_str(), dlg.getLogLevels().c_str());

		EEI_GUI gui = pEEI->params();
		dlg.addXSection("EEI", gui);
	}
	else
	{
		for (EEI_Dlg::iterator it = dlg.begin(); it != dlg.end(); it++)
		{
			EnergyEfficientIntersection *pEEI = EEIManager::instance().add(it->first.c_str());
			pEEI->initLogging(log_folder.c_str(), dlg.getLogLevels().c_str());

			fLogMsg("----- Adding EEI [%s]\n", it->first.c_str());
		}
	}

	// NOTE: Wait for Aimsun to add the requested parameters to init() or load() 
	//		 so we know number of replications and current replication
	if ( dlg.getReplications() > 0 || dlg.DoModal() == IDOK)
	{
		dlg.setReplications(dlg.getReplications() > 0 ? dlg.getReplications()-1 : 0);
		// if user has selected another settings file (not the default),
		// then cleanup and use whatever was in that file
		if (dlg.getSettingsFileName()[0] == 0)
		{
			// if there is no settings file, use defaults
			dlg.SetupParameters(EEIManager::instance().getSettingsPath().c_str(), EEIManager::instance().vehTypes());
		}

		// TODO: check if any intersections were added or deleted before clearing and re-creating
		//		 to avoid unnecessary rebuilds in case just pressed [OK]

		// discard the initial settings and setup again from user choices
		EEIManager::instance().clear();
		for (EEI_Dlg::iterator it = dlg.begin(); it != dlg.end(); it++)
		{
			EnergyEfficientIntersection *pEEI = EEIManager::instance().add(it->first.c_str());
			pEEI->initLogging(log_folder.c_str(), dlg.getLogLevels().c_str());
		}
		EEIManager::instance().setSettingsPath(dlg.getSettingsFileName());
		EEIManager::instance().vehTypes() = vnames;

		// settings pathname may have changed due to user selection
		for (EEI_Dlg::iterator it = dlg.begin(); it != dlg.end(); it++)
		{
			EnergyEfficientIntersection *pEEI = EEIManager::instance().get(it->first.c_str());
			Assert(pEEI != 0);

			//	1. add a name member to each EEI so it knows its name and can also use it to init its log
			//	2. add functionality to each EEI to log required statistics/information all in one struct

			pEEI->params() = it->second;
			pEEI->vehTypes() = dlg.vehTypesSel();

			if (!strncmp(it->first.c_str(), "(+)", 3))
				pEEI->enable();
			else
				pEEI->disable();
		}

		if (!dlg.m_filename.IsEmpty())
		{
			Settings st;
			st.path(dlg.m_filename);
			st.vehTypes() = EEIManager::instance().vehTypes();
			st.vehTypesSel() = dlg.vehTypesSel();
			st.logLevels( dlg.getLogLevels().c_str() );
			st.replications( dlg.getReplications() );

			//  for global use, add vehTypesSel() to settings and save it as one line selected=1,4,..... in settings
			//	so that when we load it, the list selection can be initialized

			for (EEI_Dlg::iterator it = dlg.begin(); it != dlg.end(); it++)
			{
				Settings::paramGroup pg;
				it->second >> pg;
				st.params()[it->first] = pg;
			}
			st.write();
		}
	}
	else
	{
		for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
			it->second->disable();
	}

	int r;
	for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
		r = it->second->Initialize();
	ANGConnEnableVehiclesInBatch(true);
	return 1;
}

int AAPIManage(double time, double timeSta, double timTrans, double acicle)
{
//	AKIPrintString("\tManage");
//	return EnergyEfficientIntersection::instance()->Step(time, timeSta, timTrans, acicle);
	int r;
	for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
		r = it->second->Step(time, timeSta, timTrans, acicle);
	return r; 
}

int AAPIPostManage(double time, double timeSta, double timTrans, double acicle)
{
//	AKIPrintString("\tPostManage");
	int r;
	for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
		r = it->second->PostStep(time, timeSta, timTrans, acicle);
	return r; 
}

int AAPIFinish()
{
//	AKIPrintString("\tFinish");
	int r;
	for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
	{
		Statistics &stat = it->second->stat();

		std::string repName = it->second->logFolder() + "\\" + it->second->baseName() + "_rep.html";
		FILE *f = fopen(repName.c_str(), "wb");
		if (f)
		{
			std::string html = stat.toHTML();
			fwrite(html.c_str(), 1, html.length(), f);
			fclose(f);
		}

//		stat.parent()->logMsg()->fshow(stat.toHTML().c_str());

		r = it->second->Finish();
	}
	return r; 
}

int AAPIUnLoad()
{
//	AKIPrintString("UNLOAD");
	int r;
	for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
		r = it->second->UnloadScenario();
	return r; 
}

int AAPIPreRouteChoiceCalculation(double time, double timeSta)
{
	AKIPrintString("\tPreRouteChoice Calculation");
	return 0;
}

int AAPIEnterVehicle(int idveh, int idsection)
{
	return 0;
}

int AAPIExitVehicle(int idveh, int idsection)
{
	return 0;
}

int AAPIEnterVehicleSection(int idveh, int idsection, double atime)
{
	return 0;
}

int AAPIExitVehicleSection(int idveh, int idsection, double time)
{
//	return EnergyEfficientIntersection::instance()->ExitVehicleSection(idveh, idsection, time);
	int r;
	for (EEIManager::iterator it = EEIManager::instance().begin(); it != EEIManager::instance().end(); it++)
		r = it->second->ExitVehicleSection(idveh, idsection, time);
	return r; 
}

int AAPIEnterPedestrian(int idPedestrian, int originCentroid)
{
	AKIPrintString("A Legion Pedestrian has entered the network");
	return 0;
}

int AAPIExitPedestrian(int idPedestrian, int destinationCentroid)
{
	AKIPrintString("A Legion Pedestrian has exited the network");
	return 0;
}