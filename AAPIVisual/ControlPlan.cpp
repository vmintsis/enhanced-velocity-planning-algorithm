#include "Precompiled.h"

#include "EEI_Errors.h"
#include "JunctionSignalInfo.h"
#include "ControlPlan.h"

namespace eei
{

//----------------------------------------------------------
//
//----------------------------------------------------------
ControlPlan::ControlPlan() 
	: m_cycle_duration(0.0), m_cycle_start(0.0), m_curCtlPlan(-1), m_firstGreenPhase(-1)
{

}

//----------------------------------------------------------
//	Called once every time the control plan changes
//----------------------------------------------------------
ERcode ControlPlan::CalcCycleDuration(int idJunction, int signalGroup, double timeSta)
{
	ERcode ret = EC_OK;
	double cycle_time = 0.0;
	m_phase_seq.clear();

	// check if junction has fixed control type, if not - error
	int ctl_type = SP.f_ECIGetControlType(idJunction);
	if (ctl_type == 1)  // fixed
	{
		// sum durations of all phases at current absolute simulation time
		int nPhases = SP.f_ECIGetNumberPhases(idJunction);
		if (nPhases >=0)
		{
			for (int i=0; i<nPhases; i++)
			{
				double dur, dur_max, dur_min;
				int r = SP.f_ECIGetDurationsPhase(idJunction, i+1, timeSta, &dur, &dur_max, &dur_min);
				if (r >= 0)
				{
					cycle_time += dur;
				}
				else
				{
					ret = EC_CannotGetDurations;
					break;
				}

				// according to AimSun, we can detect states by checking if our signal group belongs to the current, since more that one
				// signal groups can belong to a phase. It seems that if our SG is not in a phase, then the state is RED, else it is GREEN or YELLOW
				int num_signalGroups = SP.f_ECIGetNbSignalGroupsPhaseofJunction(idJunction, i+1, timeSta);
				bool inPhase = false;
				for (int t=0; !inPhase && t<num_signalGroups; t++)
				{
					int sg = SP.f_ECIGetSignalGroupPhaseofJunction(idJunction, i+1, t, timeSta);
					if (sg == signalGroup)
						inPhase = true;
				}

				// keep phase durations for this plan, and set states to invalid until we can fill-in their values at runtime
				// to check the principle of single green within a cycle per signal group, and disable our module if it is violated (not likely, but...)
				PhaseDS pds(dur, (inPhase ? JSI_GREEN : JSI_RED) );
				m_phase_seq.push_back(pds);
			}
		}
		m_cycle_duration = cycle_time;
	}
	else
	{
		ret = EC_NotFixedControl;  // fixed signal control expected
	}

	return ret;
}

//----------------------------------------------------------
//	Calculate remaining time of red light.
//	This must be called while in a RED or YELLOW state and
//	starts at the point indicated by jsi, and continues to add any next red phase durations.
//	Returns: remaining red time in seconds or 0.0 if called during a non-red phase
//----------------------------------------------------------
double ControlPlan::getRemainingRedTime(JunctionSignalInfo &jsi)
{
	double time_red = 0.0;
	if (jsi.state != JSI_GREEN)
	{
		time_red = jsi.time_left;
		size_t nextPhase = jsi.phase % m_phase_seq.size(); // next phase to current
		while (m_phase_seq[nextPhase].state != -1 && m_phase_seq[nextPhase].state != JSI_GREEN)
		{
			time_red += m_phase_seq[nextPhase].duration;
			nextPhase = (nextPhase + 1) % m_phase_seq.size();
		}
	}
	return time_red;
}

//----------------------------------------------------------
//	Calculate time of red light AFTER the current green.
//	This must be called while in a GREEN state and
//	starts at the point indicated by jsi, finds the first red after it,
//	and continues to add any next red phase durations until green is encountered again.
//	Returns:  red time after current green in seconds or 0.0 if called during a non-green phase
//----------------------------------------------------------
double ControlPlan::getNextRedTime(JunctionSignalInfo &jsi)
{
	double time_red = 0.0;
	if (jsi.state == JSI_GREEN)
	{
		size_t nextPhase = jsi.phase % m_phase_seq.size(); // next phase to current
		while (m_phase_seq[nextPhase].state != -1 && m_phase_seq[nextPhase].state == JSI_GREEN)
			nextPhase = (nextPhase + 1) % m_phase_seq.size(); // skip all greens after the current

		// now sum durations of following red phases
		while (m_phase_seq[nextPhase].state != -1 && m_phase_seq[nextPhase].state != JSI_GREEN)
		{
			time_red += m_phase_seq[nextPhase].duration;
			nextPhase = (nextPhase + 1) % m_phase_seq.size();
		}
	}
	return time_red;
}


//----------------------------------------------------------
//	Calculate remaining time of green light
//	This must be called while in a GREEN state.
//	This starts at the point indicated by jsi, and continues to add any next green phase durations
//	Returns: remaining green time in seconds or 0.0 if called during a non-green phase
//----------------------------------------------------------
double ControlPlan::getRemainingGreenTime(JunctionSignalInfo &jsi)
{
	double time_green = 0.0;
	if (jsi.state == JSI_GREEN)
	{
		time_green = jsi.time_left;
		size_t nextPhase = jsi.phase % m_phase_seq.size(); // next phase to current
		while (m_phase_seq[nextPhase].state != -1 && m_phase_seq[nextPhase].state == JSI_GREEN)
		{
			time_green += m_phase_seq[nextPhase].duration;
			nextPhase = (nextPhase + 1) % m_phase_seq.size();
		}
	}
	return time_green;

}

//----------------------------------------------------------
//	Returns the total green time allocated to the current signal group in 
//	all phases of the control plan (for statistics)
//----------------------------------------------------------
double ControlPlan::getGreenTime()
{
	double gt = 0.0;
	for (size_t i =0; i<phases().size(); i++)
		if (phases()[i].state == JSI_GREEN)
			gt += phases()[i].duration;
//	if (m_firstGreenPhase >= 1 && m_firstGreenPhase <= (int)m_phase_seq.size())
//		gt = m_phase_seq[m_firstGreenPhase-1].duration;
	return gt;
}

} // namespace 
