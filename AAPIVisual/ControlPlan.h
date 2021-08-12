#ifndef _ControlPlan_H_
#define _ControlPlan_H_

namespace eei
{

enum ERcode;
struct JunctionSignalInfo;


// partial information for one phase of the control plan
struct PhaseDS
{
	PhaseDS() : duration(0.0), state(-1) { }
	PhaseDS(double dur, int st) : duration(dur), state(st) { }
	PhaseDS(const PhaseDS &pds) : duration(0.0), state(-1) { *this = pds; }

	PhaseDS &operator=(const PhaseDS &pds)
	{
		if (this != &pds)
		{
			duration = pds.duration;
			state = pds.state;
		}
		return *this;
	}

	double duration;
	int state;
};

typedef std::vector<PhaseDS> PhaseSeq;			// phase durations/states sequence

///////////////////////////////////////
//
//	Control plan
//
class ControlPlan
{
// construction/destruction
public:
	ControlPlan();


// operations
public:
	ERcode CalcCycleDuration(int idJunction, int signalGroup, double timeSta);	// calulate phases and total cycle duration whenever current plan changes
	double getRemainingRedTime(JunctionSignalInfo &jsi);	// return the remaining RED time from the current junction signal state - must be called during a RED or YELLOW, else return valid is 0
	double getNextRedTime(JunctionSignalInfo &jsi); // return the total duration of all red after current green
	double getRemainingGreenTime(JunctionSignalInfo &jsi);	// return the remaining GREEN time from the current junction signal state - must be called during a GREEN, else return valid is 0
	double getGreenTime();	// total green time for this cycle regardless of continuity of green phases

// accessors
public:
	double getCycleDuration() { return m_cycle_duration; }

	double getCycleStartTime() { return m_cycle_start; }
	void setCycleStartTime(double t) { m_cycle_start = t; }

	int getControlPlanID() { return m_curCtlPlan; }
	void setControlPlanID(int id) { m_curCtlPlan = id; }

	PhaseSeq phases() { return m_phase_seq; }
private:
	PhaseSeq m_phase_seq;		// control plan phase sequence for a specific signal group
	int m_curCtlPlan;			// current control plan 
	double m_cycle_duration;	// duration of the signal plan cycle
	double m_cycle_start;		// start time of current cycle

	int m_firstGreenPhase;		// phase associated with green state of signal group of interest
};

} // namespace eei

#endif