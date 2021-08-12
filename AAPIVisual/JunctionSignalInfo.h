#ifndef _JunctionSignalInfo_H_
#define _JunctionSignalInfo_H_

namespace eei
{


//
// Junction signal information 
//
enum JSI_State
{
	JSI_UNKNOWN=-1,
	JSI_RED,
	JSI_GREEN,
	JSI_YELLOW,

	JSI_Last
};

struct JunctionSignalInfo
{
	JunctionSignalInfo() : start_time(-1), state(-1), phase(-1), time_left(0), duration(0) { }

//	bool valid() { return state != -1 && start_time >= 0 && phase >=1 && duration > 0; }
	bool valid() { return state != -1 && start_time >= 0 && duration > 0; }
	const char *stateName() 
	{
		return state==JSI_RED ? "RED" : state==JSI_GREEN ? "GREEN" : state==JSI_YELLOW ? "YELLOW" : "UNKNOWN STATE";
	}

	double start_time;	// phase start time
	double duration;	// phase duration
	int state;			// phase state (R,G,Y) for a specific signal group
	int phase;			// current phase as defined in the signal control plan
	double time_left;	// time left until end of this phase
};

} // namespace 


#endif