#include "Precompiled.h"
#include "EnergyEfficientIntersection.h"
#include "EEIManager.h"

namespace eei
{

//----------------------------------------------------------
//
//----------------------------------------------------------
EnergyEfficientIntersection::EnergyEfficientIntersection()
	: m_simStep(0), m_units(Invalid), m_enabled(false), m_reported(false), m_intersectionName(""), m_roadName(""),
	  m_qexit_speed(0.0)
{
	m_prm.m_vsection.setParent(this);
	m_trackedVeh.setParent(this);
	m_stat.setParent(this);

	test = false;


	LOG("EnergyEfficientIntersection CTOR %p\n", this);
}

//----------------------------------------------------------
//
//----------------------------------------------------------
EnergyEfficientIntersection::~EnergyEfficientIntersection()
{
	LOG("EnergyEfficientIntersection DTOR %p\n", this);
}

//----------------------------------------------------------
//
//----------------------------------------------------------
const char *EnergyEfficientIntersection::title() 
{
	return m_title.empty() ? "" : m_title.c_str(); 
}

//----------------------------------------------------------
//
//----------------------------------------------------------
void EnergyEfficientIntersection::setTitle(const char *t) 
{ 
	REQUIRE(t != 0); 
	m_title = t; 
}

//----------------------------------------------------------
//
//----------------------------------------------------------
const char *EnergyEfficientIntersection::intersectionName()
{
	if (m_intersectionName == "")
	{
		if (m_title != "")
		{
			// get it from the user-defined definition
			char *p = strchr((char *)m_title.data(), ':');
			if (p)
			{
				m_intersectionName = p[1] == ' ' ? p+2 : p+1;
			}
		}
//		if (params().junctionID > 0)
//		{
//			m_intersectionName = SP.f_ANGConnGetObjectNameA(params().junctionID);
//		}
		else
			return "undefined_in";
	}
	return m_intersectionName.c_str();
}

//----------------------------------------------------------
//
//----------------------------------------------------------
const char *EnergyEfficientIntersection::roadName()
{
	if (m_roadName == "")
	{
		if (params().m_vsection.isvalid())
		{
			std::vector<int> sections = params().m_vsection.getSectionIDs();
			m_roadName = SP.f_ANGConnGetObjectNameA(sections[sections.size()-1]);
		}
		else
			return "undefined_rn";
	}

	return m_roadName.c_str();
}

//----------------------------------------------------------
//
//----------------------------------------------------------
int EnergyEfficientIntersection::getScenarioID()
{
	return EEIManager::instance().scenarioId();
}

//----------------------------------------------------------
//
//----------------------------------------------------------
std::string EnergyEfficientIntersection::getScenarioName()
{
	return EEIManager::instance().scenarioName();
}

//----------------------------------------------------------
//
//----------------------------------------------------------
/*int EnergyEfficientIntersection::LoadScenario()
{
	slog("EEI Load (%s)\n", enabled() ? "enabled" : "disabled" );
	if (!enabled()) return -1;

	return 0;
}
*/
//----------------------------------------------------------
//
//----------------------------------------------------------
int EnergyEfficientIntersection::Initialize()
{
	slog("EEI Init (%s)\n", enabled()? "enabled" : "disabled" );
	if (!enabled()) return -1;

	int u = SP.f_AKIInfNetGetUnits();
	switch(u)
	{
		case 1 : m_units = Metric; break;
		case 0 :
				m_units = English; 
				m_prm.startPoint( UnitConv().m2f(m_prm.startPoint()) ); // convert our meters to feet
				m_prm.endPoint( UnitConv().m2f(m_prm.endPoint()) ); // convert our meters to feet
				break;
		default:
			m_units = Invalid;
	}

	m_qexit_speed = SP.f_GetQueueExitSpeed();

	// TODO: After getting all the necessary parameters from the GUI/config file, set a valid/invalid state accordingly.
	//		 If state is invalid, do not perform any calculations, just log inability to function based on non-validity of user-supplied parameters.
	//		 Alternatively, if we have valid defaults, use them and log this fact.

	m_prm.m_vsection.updateSectionInfo();
	if (!m_prm.m_vsection.isvalid())
	{
		slogErr("*** Virtual section is not valid, disabling EEI\n");
		flogErr("*** Virtual section is not valid, disabling EEI\n");
		disable();
	}


	if (vehTypes().empty())
		flogMsg("No Equipped Vehicles specified. Using all vehicles\n");
	else
	for (VehicleTypes::iterator it = vehTypes().begin(); it != vehTypes().end(); it++)
		flogMsg("Equipped Vehicle: %s - %d\n", it->first.c_str(), it->second);

/*	std::string base = title();
	if (base[0] == '(')
	{
		if (base[1] == '-')
			base = base.substr(5);
		else
			base = base.substr(4);
		size_t pos = base.find(":");
		if (pos != std::string::npos)
			base = base.substr(0, pos);
		for (std::string::iterator it = base.begin(); it != base.end(); it++)
			if (*it == ' ') *it = '_';
	}
	std::string fname = logs_location(("\\count_"+base).c_str());
	CLog::log("counter")->init((fname + ".txt").c_str());
*/

//	trackedVehicles().setNonCompliantVehicleXForm(new randomScaleProfileXform(0.65, 1.8) );

	//TODO: Create a XForm generator based on a configuration file that defines the XForm (possibly based on a graphic representation input).
	//		This code can then use a XFormGenerator class instance to create the transformation choice tree.

	// User parameters: 
	//	0 - action(accelerate, decelerate), COPT_XF_TYPE
	//	1 - trafficLightDist
	//	2 - secondsTillChange
	//
	compositeOffsetProfileXForm *xf = new compositeOffsetProfileXForm("PROFILE_XFORM", this);
	compositeOffsetProfileXForm *accel_xf = new compositeOffsetProfileXForm("ACCEL", this);
	compositeOffsetProfileXForm *decel_xf = new compositeOffsetProfileXForm("DECEL", this);
	xf->addXForm(XF_ACCELERATE, accel_xf);
	xf->addXForm(XF_DECELERATE, decel_xf);
	xf->setRule([](IVehComplianceProfileXForm *xform) -> bool
	{
		//	first, check what type of action we are performing: acceleration or deceleration
		//	XF_User + 0: action type: acceleration or deceleration as defined in profile transformations
		XF_Type type = (XF_Type)(int)xform->owner()->get("xform_type");
		// now, get the corresponding xform type from the owner and see if it is the one passed
		// and if so, accept it, else reject it
		compositeOffsetProfileXForm *cxf = (compositeOffsetProfileXForm *)xform->owner();
		IVehComplianceProfileXForm *reg_xform = cxf->getXform(type);
		return reg_xform == xform;
	});

	accel_xf->addXForm(XF_Type::XF_ACCELERATE, new offsetProfileXform("ACCEL_IMPL", this, OPX_CriteriaRange(0.0, 0.0), OPX_ValuesRange(-11.917667, 19.906667)));
	accel_xf->setRule([](IVehComplianceProfileXForm *xform) -> bool
		{
			return true;
		});

	decel_xf->addXForm(XF_Type::XF_DECELERATE, new offsetProfileXform("DECEL_TLD_A", this, OPX_CriteriaRange(0.0, 178.31), OPX_ValuesRange(-20.0, -11.917667)));
	compositeOffsetProfileXForm *decel_xf_tld = new compositeOffsetProfileXForm("DECEL_TLD_B", this);
	decel_xf_tld->set("crt0", 178.31).set("crt1", 350.64);  // user-set criteria since this is a composite
	decel_xf->addXForm(XF_Type::XF_DECELERATE, decel_xf_tld);
		decel_xf_tld->addXForm(XF_Type::XF_DECELERATE, new offsetProfileXform("DECEL_STC_A", this, OPX_CriteriaRange(0.0, 47.5), OPX_ValuesRange(-11.917667, 19.906667)));
		decel_xf_tld->addXForm(XF_Type::XF_DECELERATE, new offsetProfileXform("DECEL_STC_B", this, OPX_CriteriaRange(47.5, 10000.0), OPX_ValuesRange(-20.0, -11.917667)));
		decel_xf_tld->setRule([](IVehComplianceProfileXForm *xform) -> bool
		{
			double t0 = xform->get("crt0");	// get criteria
			double t1 = xform->get("crt1");

			// check current runtime value against criteria interval and return true if this xform is the correct one to apply, else false
			double secTillChange = xform->owner()->owner()->owner()->get("secondsTillChange");
			return (secTillChange > t0 && secTillChange <= t1);
		});
		decel_xf->addXForm(XF_Type::XF_DECELERATE, new offsetProfileXform("DECEL_TLD_C", this, OPX_CriteriaRange(350.64, 10000.0), OPX_ValuesRange(-11.917667, 19.906667)));
	decel_xf->setRule([](IVehComplianceProfileXForm *xform) -> bool
	{
		double d0 = xform->get("crt0");	// get criteria
		double d1 = xform->get("crt1");

		// check current runtime value against criteria interval and return true if this xform is the correct one to apply, else false
		double trafficLightDist = xform->owner()->owner()->get("trafficLightDist");
		return (trafficLightDist > d0 && trafficLightDist <= d1);
	});

	trackedVehicles().setNonCompliantVehicleXForm(xf);

	return 0;
}

//----------------------------------------------------------
//
//----------------------------------------------------------
int EnergyEfficientIntersection::Step(double time, double timeSta, double timTrans, double acicle)
{
//	slog("EEI Step\n");

	return 0;
}

//----------------------------------------------------------
//
//  [1] - Dynamic ECO-Driving for Arterial Corridors, Barth M., et al.
//----------------------------------------------------------
int EnergyEfficientIntersection::PostStep(double time, double timeSta, double timTrans, double acycle)
{
	if (!m_reported)
	{
		flogMsg("\n----------------------------------\n==== EEI ENTER PostStep time=%f  timeSta=%f (%s)====\n", time, timeSta, enabled()? "enabled" : "disabled");
		m_reported = true;
	}
	if (!enabled()) return -1;

	// Keep current simulation step (sec) and do a couple of sanity checks.
	// On failure, return one of AIMSUN's error codes
	m_simStep = acycle;
	m_prm.m_vsection.updateSectionInfo();

	if (!m_stat.isEnabled())
	{
		if (time >= timTrans) m_stat.enable();
	}

//	static bool test = false;
	if (!test)
	{
		test = true;
		flog("Queue Exit Speed: %3.4f\n", m_qexit_speed);

		flog("============ BEGIN SECTION MAP SEQUENCE ==============\n");
		for (Section::sectionMap_t::iterator it = m_prm.m_vsection.smap().begin(); it != m_prm.m_vsection.smap().end(); it++)
			flog("id = %d  %d   %d  {%s}\n", (int)it->first, it->first.index(), it->first.id(), it->second.toString().c_str() );
		flog("============ END SECTION MAP SEQUENCE ==============\n");
	}

//	if (getSectionID() < 0)
	if (!m_prm.m_vsection.isvalid())
	{
		EEI_Error::err()->log_error(EC_InvalidSectionID);
		disable();
		return -EC_InvalidSectionID; // AimSun: invalid parameter
	}
	if (getUnits() == Invalid)
	{
		EEI_Error::err()->log_error(EC_UnknownUnits);
		disable();
		return -EC_UnknownUnits;		// AimSun: in lack of a units-related error code, we use this one to indicate unknown units condition
	}

	//--------------------------------------------
	// for the current control plan, calculate the control cycle duration
	// once at the beginning and then every time the plan changes (if it ever does)
	int curPlan = SP.f_ECIGetNumberCurrentControl();
	if (m_control_plan.getControlPlanID() != curPlan)
	{
		m_control_plan.setControlPlanID(curPlan);
		ERcode r = m_control_plan.CalcCycleDuration(m_prm.junctionID(), getSignalGroup(), timeSta);
		if (r != EC_OK)
		{
			EEI_Error::err()->log_error(r);
			return -r;
		}
		else
		{
			flogMsg("----- PHASE SEQUENCE FOR SG=%d -----\n", getSignalGroup());
			for (size_t i=0; i<m_control_plan.phases().size(); i++)
			{
				flogMsg(" Phase: %d - dur=%3.2f state=%s\n", i+1, 
					m_control_plan.phases()[i].duration,
					m_control_plan.phases()[i].state==JSI_RED ? "RED" : 
					m_control_plan.phases()[i].state==JSI_GREEN ? "GREEN" : 
					m_control_plan.phases()[i].state==JSI_YELLOW ? "YELLOW" : "UNKNOWN STATE");
			}
			flog("Cycle Duration=%f\n", m_control_plan.getCycleDuration());
		}
	}

	//--------------------------------------------
	// update signal phase information and init phase sequence (until we get a response from Aimsun!)
	UpdateCurrentPhaseInfo(time, timeSta, getJunctionID(), getSignalGroup());

	if (m_jsi.state != JSI_GREEN)
		flog("Remaining red time=%f\n", m_control_plan.getRemainingRedTime(m_jsi));

	// Wait until we have some vehicles entering our section
//	int nVehicles = AKIVehStateGetNbVehiclesSection(getSectionID(), true);
	int nVehicles = m_prm.m_vsection.numVehicles();
	if (nVehicles >0)
	{
		// Note: AimSun sets section or junction id to -1 if vehicle is not in section or junction respectively.
		for (TrackedVehicles::iterator it = trackedVehicles().begin(); it != trackedVehicles().end(); )
		{
			InfVeh iv1 = SP.f_AKIVehTrackedGetInf(it->idVeh());
			int id = iv1.idSection == -1 ? iv1.idJunction : iv1.idSection;
			if (!m_prm.m_vsection.contains(id))
			{
				flog("EEI PostStep: Vehicle %d has exited vsection from a side street.\n", iv1.idVeh);
				trackedVehicles().removeVehicle(it->idVeh());
				it = trackedVehicles().begin();
			}
			else
				it++;
		}

		// check and update our information regarding any cars that may have entered car-following state
		// NOTE: this is according to *our* rule; AimSun may allow closer distances
		updateCarFollowingInformation();

		// Iterate over all vehicles and control the tracked, and see if any new ones entered our communication zone
//		flog("EEI PostStep: numVehicles=%d\n", nVehicles);
		for (int i=0; i<nVehicles; i++)
		{
			// get each vehicle state information
			InfVeh iv = m_prm.m_vsection.GetVehicleInfSection(i);
			if (iv.report == 0)
			{
				// commented out 1-Jun-2015 (if we track everything here, we'll never count then in OnEnteredZone() below)
//				int r = trackedVehicles().trackVehicle(iv.idVeh);
//				if (r < 0)
//				{
//					flogErr("***EEI PostStep: track vehicle (%d: ID=%d) error=%d\n", i, iv.idVeh, r);
//					continue;
//				}

				// if it's a known vehicle then it's speed has been calculated, so set it and go to the next one
				if (trackedVehicles().advisedVehicle(iv.idVeh))
				{
					// check is this vehicles has any other in front of it 
					if (enableCFR() && trackedVehicles().isCarFollowing(iv.idVeh))
					{
						// let the driver take control from now on
						// remove only from advisory, but keep it tracked and also record the reason for removal
						trackedVehicles().removeVehicle(iv.idVeh, 
									TrackedVehicles::REMOVE_ADVISE | 
									TrackedVehicles::REMOVE_REASON_CAR_FOLLOWING); 
						m_stat.OnUnadvised(iv.type);
						flog("EEI PostStep: Vehicle %d unadvised due to car-following rule\n", iv.idVeh);
					}
					else
					if (enforceMinSpeed() && !trackedVehicles().vehicleProfile(iv.idVeh).aboveMinSpeed( UnitConv().kmph2mps(minSpeed()) ))
					{
						// If min speed has been defined to be > 0, and if any speed in the profile falls below min speed
						// unadvise this vehicle, and let the driver take control from now on.
						// Remove only from advisory, but keep it tracked and also record the reason for removal
						trackedVehicles().removeVehicle(iv.idVeh, 
								TrackedVehicles::REMOVE_ADVISE |
								TrackedVehicles::REMOVE_REASON_MIN_SPEED); 
						m_stat.OnUnadvised(iv.type);
						flog("EEI PostStep: Vehicle %d unadvised due to minimum allowable speed rule\n", iv.idVeh);
					}
					else
						adjustVehicleSpeed(iv);
					continue;
				}
				else
					flog("(NO PROFILE) vehID=%d distance to end=%f meters (cur_speed=%f)\n", iv.idVeh, iv.distance2End, iv.CurrentSpeed);

				// Note: seems that this distance includes the node turning distance, however, the traffic light where we stop is NOT at the end of this
				//			but at the end of the current section, so, we need to subtract the length of the turning in the node to calculate the correct distance
				//			to the traffic light!
				// 1-Feb-2014: We now use StaticVehicleInfo and the formula given by AimSun (see Accelerate)
				// iv.distance2End -= 10; // UnitConv().kmph2mps(iv.CurrentSpeed)*m_simStep;


				// slog("EEI Step: vehicle %d: ID=%d  dist=%f speed=%f \n", i, iv.idVeh, iv.distance2End, iv.CurrentSpeed);
				// activate our algorithm when a vehicle enters the specified area before the junction within the specified section

				// use vehTypes() to determine which type to track and which to ignore
//				StaticInfVeh siv = SP.f_AKIVehGetStaticInf(iv.idVeh);
//				flog(">>>VehType : %d  contained:%s\n", siv.type, (vehTypes().contains(siv.type) ? "true" : "false"));
//				flog(">>>VehType : %d  pos=%f sp=%f ep=%f in range %s\n", siv.type,
//										iv.distance2End, m_prm.startPoint(), m_prm.endPoint(),
//										(inRangeToJunction(iv.distance2End) ? "true" : "false"));
//				if (inRangeToJunction(iv.distance2End) && vehTypes().contains(siv.type))
				if (inRangeToJunction(iv.distance2End) && vehTypes().contains(iv.type))
				{
					flog("Vehicle %d within zone-----\n", iv.idVeh);

					StaticInfVeh siv = SP.f_AKIVehGetStaticInf(iv.idVeh);
					flog(">>>VehType : %d  contained:%s\n", siv.type, (vehTypes().contains(siv.type) ? "true" : "false"));
					flog(">>>VehType : %d  pos=%f start_point=%f end_point=%f in_range=%s\n", siv.type,
						iv.distance2End, m_prm.startPoint(), m_prm.endPoint(),
						(inRangeToJunction(iv.distance2End) ? "true" : "false"));

					// if this vehicle is already tracked but unadvised due to one of our rules above,
					// do not add it to the advisory again
					if ( (trackedVehicles().vehicleInfo(iv.idVeh).flags() & (
								TrackedVehicles::REMOVE_REASON_CAR_FOLLOWING |
								TrackedVehicles::REMOVE_REASON_MIN_SPEED)) != 0)
					{
						flog("Vehicle : %d ignored due to %s%s\n", iv.idVeh,
							((trackedVehicles().vehicleInfo(iv.idVeh).flags() & TrackedVehicles::REMOVE_REASON_CAR_FOLLOWING) != 0 ? " Car Following" : ""),
							((trackedVehicles().vehicleInfo(iv.idVeh).flags() & TrackedVehicles::REMOVE_REASON_MIN_SPEED) != 0 ? " Min Speed" : "")
							);
						continue;
					}

//					int r = SP.f_AKIVehSetAsTracked(iv.idVeh);
//					int r = trackedVehicles().trackVehicle(iv.idVeh);
//					if (r >= 0)
//					{
						if (m_prm.m_vsection.isvalid())
						{
							double section_speed_limit = m_prm.m_vsection.speedLimit(); // one of the two will be valid
							if (!trackedVehicles().isTracked(iv.idVeh))
							{
								m_stat.OnEnteredZone(iv.type, iv.CurrentSpeed);
//								CLog::log("counter")->fshow("Vehicle %d  type=%d speed=%3.5f\n", iv.idVeh, iv.type, iv.CurrentSpeed);
							}
							int r = trackedVehicles().trackVehicle(iv.idVeh);
							if (r < 0)
							{
								flogErr("***EEI PostStep: track vehicle (%d: ID=%d) error=%d\n", i, iv.idVeh, r);
								continue;
							}

							double vs_mps = UnitConv().kmph2mps(iv.CurrentSpeed);           // current speed in m/s
							double speed_limit_mps = UnitConv().kmph2mps(section_speed_limit);	// speed limit in m/s

							if (m_jsi.state == JSI_GREEN) // current state is green
							{
								double green_time_left = m_control_plan.getRemainingGreenTime(m_jsi);
								double dist = vs_mps * green_time_left;  // distance vehicle will cover with current speed
								if (vs_mps < m_qexit_speed)
								{
									//trackedVehicles().trackVehicle(iv.idVeh);
									// let it go: its speed is lower than the current Q exit speed, so the vehicle is most likely in a queue(?)
								}
								else
								if (dist < iv.distance2End) // if vehicle cannot reach the junction with current speed...
								{
									flog("Vehicle %d within bounds (%s)-----\n", iv.idVeh, m_jsi.stateName());
									double si_dist = speed_limit_mps * green_time_left;  // distance that can be covered with speed limit
								
									flog("Vehicle current speed %f Km/h -----\n", UnitConv().mps2kmph(vs_mps));          // VagCode: Print current speed
									flog("Section's speed limit %f Km/h -----\n", UnitConv().mps2kmph(speed_limit_mps)); // VagCode: Print speed limit
									flog("-----Vehicle %d speed profile-----\n", iv.idVeh);
									flog("b. vehID=%d distance to end=3.3%f meters,  distance at SL:%3.3f\n", iv.idVeh, iv.distance2End, si_dist);

									if (si_dist > iv.distance2End)	// acceleration: vehicle can make it through the light
									{
										flog("\nVehicle %d acceleration mode (GREEN) -----\n", iv.idVeh);
										trackedVehicles().vehicleProfile(iv.idVeh).
															accelFactor(accelFactor()).infZoneCalibParam(infZoneCalibParam()).
															accelerate(iv.distance2End, vs_mps, green_time_left, m_simStep, siv.normalDeceleration); //, accelFactor());
										m_stat.OnAdvised(iv.type, Statistics::AR_ACCELERATE, iv.CurrentSpeed);

										// TODO: add to statistics, check if this is the right place to perturb profile
										IVehComplianceProfileXForm *xform = trackedVehicles().getNonCompliantVehicleXForm();
										xform->set("xform_type", XF_Type::XF_ACCELERATE);
										checkAndSetNonCompliantType(iv.type, iv.idVeh);
									}
									else	// deceleration: vehicle can't make it even with speed limit
									{
										flog("\nVehicle %d deceleration mode (GREEN) -----\n", iv.idVeh);
										double decel_time_left = m_control_plan.getNextRedTime(m_jsi) + green_time_left ;
										trackedVehicles().vehicleProfile(iv.idVeh).
															accelFactor(accelFactor()).infZoneCalibParam(infZoneCalibParam()).
															decelerate(iv.distance2End, vs_mps, decel_time_left, m_simStep, siv.normalDeceleration); //, accelFactor());
										m_stat.OnAdvised(iv.type, Statistics::AR_DECELERATE, iv.CurrentSpeed);

										// TODO: add to statistics, check if this is the right place to perturb profile

										//	0 - action(accelerate, decelerate), COPT_XF_TYPE
										//	1 - trafficLightDist
										//	2 - secondsTillChange
										IVehComplianceProfileXForm *xform = trackedVehicles().getNonCompliantVehicleXForm();
										xform->set("xform_type", XF_Type::XF_DECELERATE);
										xform->set("trafficLightDist", iv.distance2End);
										xform->set("secondsTillChange", green_time_left);
										checkAndSetNonCompliantType(iv.type, iv.idVeh);
									}

									//	If we have not enforced min speed rule (i.e. any speed in the profile falls below min speed,
									//	in which case we unadvise the vehicle), and if we have defined a min speed > 0, then
									//	set user-defined min_speed parameter (20.0Km/h)
									if (!enforceMinSpeed() && minSpeed() > 0.0)
										trackedVehicles().vehicleProfile(iv.idVeh).validateAndAdjust(speed_limit_mps, UnitConv().kmph2mps(minSpeed()) );

									// now vehicle is tracked and we have a profile for it - time to control it
//									if (m_vmap[iv.idVeh].isValid())
//										r = AKIVehTrackedModifySpeed(iv.idVeh, UnitConv().mps2kmph(m_vmap[iv.idVeh].vnext()) );
//									else
									if (!trackedVehicles().hasProfile(iv.idVeh))
										flog("Velocity profile IGNORED vehID=%d\n", iv.idVeh);

								}
								else // let it go, it will make it past the junction while green is on
								{
									//trackedVehicles().trackVehicle(iv.idVeh);
									// 24-Aug-2014: do not untrack, since we need all advised and not advised vehicles to 
									//				correctly calculate car-following
									// 07-Sep-2014: If we need to track/untrack we now do this via the trackedVehicles() collection
//									SP.f_AKIVehSetAsNoTracked(iv.idVeh);
								}
							}
							else	// current state is yellow or red
							{
								double red_time_left = m_control_plan.getRemainingRedTime(m_jsi);	// time left to next green
								double dist = vs_mps * red_time_left;  // distance vehicle will cover with current speed within remaining red time

								if (vs_mps < m_qexit_speed)
								{
									// let it go: its speed is lower than the current Q exit speed, so the vehicle is most likely in a queue(?)
									//trackedVehicles().trackVehicle(iv.idVeh);
								}
								else
								if (dist > iv.distance2End)	// if vehicle overshoots the junction, we need to decelerate (it's going too fast and will stop otherwise)
								{
									flog("\nVehicle %d deceleration mode (%s) time_left=%f red_time_left:%f -----\n", iv.idVeh, m_jsi.stateName(), m_jsi.time_left, red_time_left);
									trackedVehicles().vehicleProfile(iv.idVeh).
												accelFactor(accelFactor()).infZoneCalibParam(infZoneCalibParam()).
												decelerate(iv.distance2End, vs_mps, red_time_left, m_simStep, siv.normalDeceleration); //, accelFactor());

									m_stat.OnAdvised(iv.type, Statistics::AR_DECELERATE, iv.CurrentSpeed);

									// TODO: add to statistics, check if this is the right place to perturb profile
									//	0 - action(accelerate, decelerate), COPT_XF_TYPE
									//	1 - trafficLightDist
									//	2 - secondsTillChange
									IVehComplianceProfileXForm *xform = trackedVehicles().getNonCompliantVehicleXForm();
									xform->set("xform_type", XF_Type::XF_DECELERATE);
									xform->set("trafficLightDist", iv.distance2End);
									xform->set("secondsTillChange", red_time_left);
									checkAndSetNonCompliantType(iv.type, iv.idVeh);

									// set user-defined min_speed parameter (20.0Km/h)
									if (!enforceMinSpeed() &&minSpeed() > 0.0)
										trackedVehicles().vehicleProfile(iv.idVeh).validateAndAdjust(speed_limit_mps, UnitConv().kmph2mps(minSpeed()) );

									// now vehicle is tracked and we have a profile for it - time to control it
//									if (m_vmap[iv.idVeh].isValid())
//										r = AKIVehTrackedModifySpeed(iv.idVeh, UnitConv().mps2kmph(m_vmap[iv.idVeh].vnext()) );
//									else
									if (!trackedVehicles().hasProfile(iv.idVeh))
										flog("Velocity profile IGNORED vehID=%d\n", iv.idVeh);

								}
								else // let it go, it will catch up to next green with current speed
								{
									// 24-Aug-2014: do not untrack, since we need all advised and not advised vehicles to 
									//				correctly calculate car-following
//									SP.f_AKIVehSetAsNoTracked(iv.idVeh);
									//trackedVehicles().trackVehicle(iv.idVeh);
								}
							}

						}
						else
							EEI_Error::err()->log_error(EC_GetSectionInfFailed);
//					}
//					else
//					{
//						flogErr("***EEI PostStep: track vehicle (%d: ID=%d) error=%d\n", i, iv.idVeh, r);
//					}

				} // if in range ....
			}
			else // error
			{
				flogErr("***EEI PostStep: getVehicleInfo(%d: ID=%d) error=%d\n", i, iv.idVeh, iv.report);
			}
		} // for each vehhicle ...
	}
	else
	if (nVehicles < 0)
		flogErr("***EEI PostStep: numVehicles error=%d\n", nVehicles);
	else
		flog("EEI PostStep: No Vehicles yet, waiting...\n");

//	flog("EEI LEAVE PostStep\n");
	return 0;
}

//----------------------------------------------------------
//
//----------------------------------------------------------
int EnergyEfficientIntersection::ExitVehicleSection(int idVeh, int idSection, double time)
{
	if (!enabled()) return -1;

	// If vehicle leaves our last section, we untrack it.
	// Other cases (leaving via an intermediate junction) are handled in PostStep() by checking if any tracked vehicles have left our vsection
	if (isLastSection(idSection))
	{
		bool tracked = trackedVehicles().isTracked(idVeh);

		if (trackedVehicles().removeVehicle(idVeh))
			flog("Advised Vehicle %d exiting monitored section %d! time=%f)\n", idVeh, idSection, time);
		else
		if (tracked)
			flog("Tracked Vehicle %d exiting monitored section %d! time=%f)\n", idVeh, idSection, time);
		else
			flog("***Untracked Vehicle %d exiting monitored section %d! time=%f)\n", idVeh, idSection, time);

	}
	return 0;
}

//----------------------------------------------------------
//
//----------------------------------------------------------
int EnergyEfficientIntersection::Finish()
{
	slog("EEI Finish\n");
	return 0;
}

//----------------------------------------------------------
//
//----------------------------------------------------------
int EnergyEfficientIntersection::UnloadScenario()
{
	slog("EEI Unload\n");

	return 0;
}


} // namespace

