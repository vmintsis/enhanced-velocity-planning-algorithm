#include "Precompiled.h"
#include "EnergyEfficientIntersection.h"

namespace eei
{

//----------------------------------------------------------
//
//----------------------------------------------------------
void EnergyEfficientIntersection::UpdateCurrentPhaseInfo(double time, double timeSta, int idJunction, int signalGroup)
{
	double start_phase_time = SP.f_ECIGetStartingTimePhase(idJunction);
	int state = SP.f_ECIGetCurrentStateofSignalGroup(idJunction, signalGroup);
	int phase = SP.f_ECIGetCurrentPhase(idJunction);
	double pt_dur, pt_max, pt_min;
	int r = SP.f_ECIGetDurationsPhase(idJunction, phase, timeSta, &pt_dur, &pt_max, &pt_min);

	if (start_phase_time != m_jsi.start_time) // transition
	{
		// if we transitioned to a green, reset our phase sequence
//		if (state == JSI_GREEN)
//			m_control_plan.initPhaseSeq(phase, state, (state == JSI_GREEN ? greenDur : state == JSI_YELLOW ? yellowDur : redDur), state != m_jsi.state);

		m_jsi.start_time = start_phase_time;
		m_jsi.state = state;
		m_jsi.duration = pt_dur;
		m_jsi.phase = phase;

		// keep starting time of current cycle
		if (phase == 1) m_control_plan.setCycleStartTime(start_phase_time);

		if (time < start_phase_time)
		{
			m_jsi.time_left = pt_dur + (start_phase_time - time);
flog("EEI PhaseInfo: TRANS(t < pt) time=%f start=%f left=%f phase=%d state=%s\n", time, start_phase_time, m_jsi.time_left, phase, m_jsi.stateName());
		}
		else
		if (time > start_phase_time)
		{
//			double prev_time = time - m_simStep;
//			double lerp = 1.0 - (start_phase_time - prev_time)/m_simStep;
//			m_jsi.time_left = pt_dur - m_simStep*lerp;
			m_jsi.time_left = pt_dur - (time - start_phase_time); 
flog("EEI PhaseInfo: TRANS(t > pt: LERP) time=%f start=%f left=%f phase=%d state=%s\n", time, start_phase_time, m_jsi.time_left, phase, m_jsi.stateName());
		}
		else
		{
			m_jsi.time_left = pt_dur;
flog("EEI PhaseInfo: TRANS(t == pt) time=%f start=%f left=%f phase=%d state=%s\n", time, start_phase_time, m_jsi.time_left, phase, m_jsi.stateName());
		}
	}
	else
	{
		// Aimsun says we are in the same phase, but the time left is less than the current time-step.
		// So we really are in the next phase.
//		if (m_jsi.time_left < m_simStep)
//		{
//			m_jsi.time_left = pt_dur + (start_phase_time - time);
//		}
//		else
			m_jsi.time_left -= m_simStep;

flog("EEI PhaseInfo: CURR time=%f start=%f left=%f phase=%d state=%s\n", time, start_phase_time, m_jsi.time_left, phase, m_jsi.stateName());
	}
}

//----------------------------------------------------------
//
//----------------------------------------------------------
void EnergyEfficientIntersection::adjustVehicleSpeed(InfVeh &iv)
{
	// if this is a 'known' vehicle - i.e. one that we already calculated its profile
	REQUIRE (trackedVehicles().advisedVehicle(iv.idVeh) || "***adjustVehicleSpeed called with unknown vehicle!"==0);

	if (trackedVehicles().hasProfile(iv.idVeh))
	{
		// move to this vehicle's next velocity profile value
		double vn = trackedVehicles().vehicleProfile(iv.idVeh).vnext();
		flog("(PROFILE) vehID=%d distance to end=%f meters (cur_speed=%f, next prof_speed=%f)\n", iv.idVeh, iv.distance2End, iv.CurrentSpeed, UnitConv().mps2kmph(vn));
		if (vn >=0.0)
			SP.f_AKIVehTrackedModifySpeed(iv.idVeh, UnitConv().mps2kmph(vn) );
		else
		if (vn == VN_EndOfProfile)
			flogErr("***Velocity profile ERROR: too few calculations vehID=%d (%d)\n", iv.idVeh, trackedVehicles().vehicleProfile(iv.idVeh).size());
		else
			flogErr("***Velocity profile ERROR: negative velocity vehID=%d (%f)\n", iv.idVeh, vn);
	}
//	else
//		slog("Velocity profile IGNORED vehID=%d\n", iv.idVeh);
}

//----------------------------------------------------------
//
//----------------------------------------------------------
void EnergyEfficientIntersection::updateCarFollowingInformation()
{
	// Update distances from start of section for all vehicles we are tracking once here
	struct CF_TrackedInfo : public TrackedVehicles::CF_Info
	{
		explicit CF_TrackedInfo(Section &section) : _section(section) { }
		void operator()(int idVeh)
		{
			setIdVeh(idVeh);

			double dist = -1.0;
			for (int i= _section.numVehicles()-1; i>=0; i--)
			{
				InfVeh iv = _section.GetVehicleInfSection(i);
				if (iv.idVeh == idVeh)
				{
					StaticInfVeh siv = SP.f_AKIVehGetStaticInf(idVeh);
					setVehicleLength( siv.length );
					setNormalDeceleration(siv.normalDeceleration);
					setReactionTime(siv.reactionTime);

					setDistFromStart(iv.CurrentPos);
					setCurrentSpeed(UnitConv().kmph2mps(iv.CurrentSpeed));
					setLane(iv.numberLane);
					break;
				}
			}
		}
		Section &_section;
	};
	trackedVehicles().updateCarFollowingInformation( CF_TrackedInfo(m_prm.m_vsection) );
}


} // namespace 

