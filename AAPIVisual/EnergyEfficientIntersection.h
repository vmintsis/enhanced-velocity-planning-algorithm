#ifndef _EnergyEfficientIntersection_H_
#define _EnergyEfficientIntersection_H_

#include "EEI_Errors.h"
#include "TrackedVehicles.h"
//#include "Velocityprofile.h"
#include "JunctionSignalInfo.h"
#include "ControlPlan.h"
#include "Section.h"
#include "VehicleTypes.h"
#include "ParameterGroup.h"
#include "EEIObject.h"
#include "Statistics.h"


namespace eei
{

// English - Metric units
enum Units
{
	Invalid  = -1,

	English,
	Metric,

	Last
};

//////////////////////////////////////////////
//
//
class EnergyEfficientIntersection : public EEIObject
{
// construction/destruction
public:
	EnergyEfficientIntersection();
	virtual ~EnergyEfficientIntersection();

	// extended parameter set
	struct EEIParams : public ParameterGroup
	{
		EEIParams() { }
		EEIParams(const EEIParams &prm) { *this = prm; }
		template<typename T>
		EEIParams(const T &prm) { *this = prm; }

		template<typename T>
		EEIParams &operator=(const T &prm)
		{
			ParameterGroup::operator=(prm);
			sectionID(ParameterGroup::sectionID());
			return *this;
		}

		const char *sectionID() const { return ParameterGroup::sectionID(); }

		void sectionID(const char *sid) 
		{ 
			REQUIRE(sid != 0); 
			ParameterGroup::sectionID(sid);
			m_vsection.addSections(section_str(sid).toIDs());
		}

		Section m_vsection;			// virtual section comprising of more than one consecutive sections/unsignalized junctions
	};

// interface methods
public:
//	int LoadScenario();
	int Initialize();
	int Step(double time, double timeSta, double timTrans, double acicle);
	int PostStep(double time, double timeSta, double timTrans, double acicle);
	int Finish();
	int UnloadScenario();
	int ExitVehicleSection(int idveh, int idsection, double time);

// algorithm accessors
public:
	double getCycleDuration() { return m_control_plan.getCycleDuration(); }
	double getCycleStartTime() { return m_control_plan.getCycleStartTime(); }

	bool isvalid() { return m_prm.m_vsection.isvalid() && m_prm.junctionID() != -1 && m_prm.signalGroup() != -1; }
	bool contains(int idSection) { return m_prm.m_vsection.contains(idSection); }
	bool isLastSection(int idSection) { return m_prm.m_vsection.isLastSection(idSection); }

	std::string getSectionIDstr() const
	{
		std::string s = m_prm.sectionID();
		return s;
	}

	// adds comma-separated section/junction IDs to the set of monitored IDs that comprise the virtual section
	void setSectionIDstr(const char *s)
	{
		REQUIRE(s != 0);
		m_prm.sectionID(s);
	}


	void setJunctionID(int id) { REQUIRE(id >0); m_prm.junctionID(id); }
	int getJunctionID() { return m_prm.junctionID(); }
	void setSignalGroup(int sg) { REQUIRE(sg >=0); m_prm.signalGroup(sg); }
	int getSignalGroup() { return m_prm.signalGroup(); }
	
	TrackedVehicles &trackedVehicles() { return m_trackedVeh; }

	Range getRangeToJunction() { return Range(m_prm.startPoint(), m_prm.endPoint()); }
	void setRangeToJunction(Range r) 
	{ 
		setRangeToJunction(r.first, r.second);
	}
	void setRangeToJunction(double d1, double d2) 
	{ 
		if (d1 >= d2) 
			{ m_prm.startPoint(d1); m_prm.endPoint(d2);} 
		else 
			{ m_prm.startPoint(d2); m_prm.endPoint(d1);} 
	}
	bool inRangeToJunction(double pos) { return m_prm.endPoint() <= pos && pos <= m_prm.startPoint(); } 

	Units getUnits() { return m_units; }
	void setUnits(Units u) { m_units = u; }

	double accelFactor() { return m_prm.accelFactor(); }
	void accelFactor(double a) { m_prm.accelFactor(a); }

	void infZoneCalibParam(double f) { m_prm.infZoneCalibParam(f); }
	double infZoneCalibParam() { return m_prm.infZoneCalibParam();}

	bool enableCFR() { return m_prm.enableCFR(); }
	void enableCFR(bool e) { m_prm.enableCFR(e); }

	double minSpeed() { return m_prm.minSpeed(); }
	void minSpeed(double s) { REQUIRE( s >= 0.0); m_prm.minSpeed(s); }

	bool enforceMinSpeed() { return m_prm.enforceMinSpeed(); }
	void enforceMinSpeed(bool b) { m_prm.enforceMinSpeed(b); }

	bool enabled() { return m_enabled; }
	void enable()  { m_enabled = true; }
	void disable()  { m_enabled = false; }

	size_t numVehTypesToTrack() { return m_vehTypes.size(); }
	bool canTrackVehType(int vt) { return m_vehTypes.contains(vt); }
	bool canTrackVehType(const char *vt) { return m_vehTypes.contains(vt); }
	VehicleTypes &vehTypes() { return m_vehTypes; }

	virtual VehicleVector vehicles(char *quote="'") { return vehTypes().toVector(quote); }

	EEIParams &params() { return m_prm; }

	double getGreenTime() { return m_control_plan.getGreenTime(); }

	virtual const char *title();
	virtual void setTitle(const char *t);
	virtual const char *intersectionName();
	virtual const char *roadName();
	virtual int getScenarioID();
	virtual std::string getScenarioName();

//statistics accessors
public:
	Statistics &stat() { return m_stat; }

protected:
	void UpdateCurrentPhaseInfo(double time, double timeSta, int idJunction, int signal_group);
	void adjustVehicleSpeed(InfVeh &iv);
	void updateCarFollowingInformation();

	// Called after a vehicle is added to advisory.
	// Checks if this vehicle is a non-compliant type, and sets it accordingly.
	// TODO: move transform algorithm out of trackedVehicles()
	void checkAndSetNonCompliantType(int type, int idVeh)
	{
		// TODO: change this code to a simple int lookup in pre-calculated vector of int non-compliant types
		const std::vector<std::string> VehNames = { "NC_Car1", "NC_Car2", "NC_Car3", "NC_Car4" };
		for (unsigned int i = 0; i < VehNames.size(); i++)
		{
			VehicleTypes::iterator vi = this->vehTypes().find(VehNames[i].c_str());
			if (vi != this->vehTypes().end() && type == vi->second)
				trackedVehicles().xformNonCompliantVehicleProfile(idVeh, false);
		}
	}

// parameters
private:
	Units m_units;				// metric/english system units (def. Invalid)
	VehicleTypes m_vehTypes;	// vehicle types we are allowed to track (if empty, we track ll)
	EEIParams m_prm;			// configuration parameters

// data
private:
	Statistics m_stat;				// statistics information over the simulation run
	TrackedVehicles	m_trackedVeh;	// tracked vehicles container

	double m_qexit_speed;			// queue exit speed (from simulator settings)
	double m_simStep;			    // calculated simulation step (needed for interpolating to the correct time values)
	JunctionSignalInfo m_jsi;	    // current phase information
	ControlPlan m_control_plan;	    // current control plan
	std::string m_title;			// user-provided name for this intersection
	std::string m_intersectionName;	// internal (Aimsun) name of this intersection
	std::string m_roadName;			// name of road section leading to the intersection

	bool m_enabled;			        // true when this module is functional, false otherwise
	bool m_reported;				// if disabled, print once only

	bool test;						// used for debug printing

};

} // namespace

#endif