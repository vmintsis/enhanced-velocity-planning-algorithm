#ifndef _VelocityProfile_H_
#define _VelocityProfile_H_

#include "EEIObject.h"


#define VN_EndOfProfile -10000.0	// end-of-velocity profile indicator value

namespace eei
{
//
// calculated velocity profile type in the interval between [m_distToJunction, 0]
//
class VelProfile 
{
// construction/destruction
public:
	enum VP_Action
	{
		VP_Invalid,

		VP_Accelerate,
		VP_Decelerate,

		VP_Last,
	};


	VelProfile() : _i(0), _action(VP_Invalid), _valid(false), _accel_factor(ACCCEL_FACTOR), _infl_zone_calib_param(INFLUENCE_ZONE_CALIBRATION_PARAM), m_pParent(0) { }
	VelProfile(const VelProfile &vp) : _i(0), _action(VP_Invalid), _valid(false), _accel_factor(ACCCEL_FACTOR), m_pParent(0) { *this = vp; }

	VelProfile &operator=(const VelProfile &vp)
	{
		if (this != &vp)
		{
			_vel = vp._vel;
			_i = vp._i;
			m_pParent = vp.m_pParent;
			_action = vp._action;
			_valid = vp._valid;
		}
		return *this;
	}

// operations
public:

	VelProfile &setParent(EEIObject *parent) { m_pParent = parent; return *this; }

	bool isValid() { return _valid && _action != VP_Invalid; }

	// Apply user-defined upper and lower bounds to the calculated profile
	void validateAndAdjust(double speed_limit_mps, double min_speed_mps)
	{
		if (isValid())
		{
			if (_action == VP_Accelerate)
			{
				for (size_t i=0; i<_vel.size(); ++i)
				{
					if (_vel[i] > speed_limit_mps)
					{
						_valid = false;
						break;
					}
				}
			}
			else
			{
				for (size_t i=0; i<_vel.size(); ++i)
				{
					if (_vel[i] < min_speed_mps)
					{
//							fLog("Bounding %f to %f,\n", UnitConv().mps2kmph(_vel[i]), UnitConv().mps2kmph(min_speed_mps));
						_vel[i] = min_speed_mps;
					}
				}
			}
		}
	}

	// check if any of the calculated profile speeds is below the minimum allowable speed
	bool aboveMinSpeed(double min_speed_mps)
	{
		bool is_above_limit = true;
		for (size_t i=0; i<_vel.size(); ++i)
		{
			if (_vel[i] < min_speed_mps)
			{
				is_above_limit = false;
				break;
			}
		}
		return is_above_limit;
	}

	// calculate acceleration or deceleration profile
	VelProfile &accelerate(double dist2End, double vs_mps, double time_left, double step, double norm_decel) //, double accel_f=0.0)
	{
//		if (accel_f != 0.0)
//			_accel_factor = accel_f;

		calculate(dist2End, vs_mps, time_left, step, norm_decel, VP_Accelerate);
		return *this;
	}
	VelProfile &decelerate(double dist2End, double vs_mps, double time_left, double step, double norm_decel) //, double accel_f=0.0)
	{
//		if (accel_f != 0.0)
//			_accel_factor = accel_f;

		double vd, vh, s, a;
		double end_speed = calculateEndSpeed(dist2End, vs_mps, time_left, VP_Decelerate, vd, vh, s, a);
		pflogMsg("DECEL END SPEED: %3.4f\n", end_speed);

		time_left += 3.0;
		end_speed = calculateEndSpeed(dist2End, vs_mps, time_left, VP_Decelerate, vd, vh, s, a);
		pflogMsg("DECEL END SPEED (+3 sec): %3.4f\n", end_speed);

		calculate(dist2End, vs_mps, time_left, step, norm_decel, VP_Decelerate);
		return *this;
	}
	
protected:
	// calculate velocity profile using [(1), eq(2), pp. 185]
	void calculate(double dist2End, double vs_mps, double time_left, double step, double norm_decel, VP_Action action)
	{
		_i = 0;
		_step = step;

		// calculate the 3rd part of equation [1] once, then correct dist2End based on 
		// ( 0.5 * v*v ) / normal_deceleration + virtual_veh_length 
		double vd, vh, s, a;
		double end_speed = calculateEndSpeed(dist2End, vs_mps, time_left, action, vd, vh, s, a);

		//TODO: invent an empirical formula to estimate _infl_zone_calib_param (use user-defined value also as hint)
		double influence_zone = (_infl_zone_calib_param * pow(end_speed, 2.0)) / fabs(norm_decel) + 4.5;
		
		if (action == VP_Decelerate)
		{
			dist2End -= influence_zone;
			end_speed = calculateEndSpeed(dist2End, vs_mps, time_left, action, vd, vh, s, a);
		}

//		double vh = dist2End/time_left; // speed vehicle needs to have to get to the junction
//		double vd = action == VP_Decelerate ? (vs_mps - vh) : (vh - vs_mps);		// difference from actual vehicle speed
//		double s = _accel_factor + 3.08 / time_left;  // rate of acceleration parameters: [(1), pp. 185, formula (6)]
//		double a = 0.5 * s *(sqrt( pow((PI/2.0 - time_left*s), 2.0) - 2*PI + 4) - PI/2.0 + time_left*s);

		pflogMsg("\ncalculate: vh=%fkm/h  vd=%fkm/h, s=%f a=%f\n"
							"           infl_zone_calib_param=%f, accel_factor=%f\n"
							"           influence_zone=%fm, dist2End=%fm, time_left=%fsec\n"
							"           norm_decel=%f, end_speed=%fkm/h\n",
				UnitConv().mps2kmph(vh), UnitConv().mps2kmph(vd),s,a, _infl_zone_calib_param, _accel_factor,
				influence_zone, dist2End, time_left, norm_decel, UnitConv().mps2kmph(end_speed));
		// [(1), eq(2) part 1]
		double t1 = PI/(2*s);
		pflogMsg("\nTime t1: (%f - %f), \n", step, t1); 
		_action = action;

		double t;
		if (action == VP_Decelerate)
		{
			for (t = step; t<t1; t+= step)
				_vel.push_back(vh + vd*cos(s*t));	
		}
		else
		{
			for (t = step; t<t1; t+= step)
				_vel.push_back(vh - vd*cos(s*t));	
		}

		pflogMsg("\neq2-part1\n");
		size_t i=0;
		for (; i<_vel.size(); i++) pflog("%f,\n", UnitConv().mps2kmph(_vel[i]));

		// [(1), eq(2) part 2]
		double t2 = t1 + PI/(2*a);
		pflogMsg("\nTime t2: (%f - %f), \n", t, t2); 
		if (action == VP_Decelerate)
		{
			for ( ; t<t2; t+= step)
				_vel.push_back(vh + vd*(s/a)*cos( a*(t - PI/(2*s) + PI/(2*a))) );
		}
		else
		{
			for ( ; t<t2; t+= step)
				_vel.push_back(vh - vd*(s/a)*cos( a*(t - PI/(2*s) + PI/(2*a))) );
		}

		pflogMsg("\neq2-part2\n");
		for (; i<_vel.size(); i++) pflog("%f,\n", UnitConv().mps2kmph(_vel[i]));

		// [(1), eq(2) part 3]
		double t3 = time_left;
		pflogMsg("\nTime t3: (%f - %f) d/vh = %f, \n", t, t3, dist2End/vh); 
//		double end_sp = (action == VP_Decelerate ? vh - vd*(s/a): vh + vd*(s/a));
		for ( ; t<=t3; t+= step)
			_vel.push_back(end_speed);
		pflogMsg("\neq2-part3\n");
		for (; i<_vel.size(); i++) pflog("%f,\n", UnitConv().mps2kmph(_vel[i]));

		_valid = true;	// tentative, pending final criteria validation
	}

	double calculateEndSpeed(double dist2End, double vs_mps, double time_left, VP_Action action, double &vd, double &vh, double &s, double &a)
	{
		vh = dist2End/time_left; // speed vehicle needs to have to get to the junction
		vd = action == VP_Decelerate ? (vs_mps - vh) : (vh - vs_mps);		// difference from actual vehicle speed
		s = _accel_factor + 3.08 / time_left;  // rate of acceleration parameters: [(1), pp. 185]
		a = 0.5 * (s*sqrt( pow((PI/2.0 - time_left*s), 2.0) - 2*PI + 4) - s*PI/2.0 + time_left*s*s);

		// [(1), eq(2) part 3]
		return action == VP_Decelerate ? vh - vd*(s/a): vh + vd*(s/a);
	}

// accessors
public:
	size_t size() { return _vel.size(); }
	double vnext() { return _i<_vel.size() ? _vel[_i++] : VN_EndOfProfile; }

	template<typename X>
	void transForm(X &x) { x.apply(_vel); };

	VelProfile &accelFactor(double f) { if (f != 0.0) _accel_factor = f; return *this; }
	double accelFactor() { return _accel_factor; }
	VelProfile &infZoneCalibParam(double f) { if (f != 0.0) _infl_zone_calib_param = f; return *this; }
	double infZoneCalibParam() { return _infl_zone_calib_param;}

// data
private:
	std::vector<double> _vel;	// speed profile
	size_t _i;					// current speed profile index - determines which profile point we'll use next
	double _step;				// current simulation time step
	VP_Action _action;			// acceleration/deceleration profile
	bool _valid;				// profile passes validation criteria
	double _accel_factor;		// rate of acceleration/deceleration factor (def. 0.03)
	double _infl_zone_calib_param;		// rate of acceleration/deceleration factor (def. 4.0)
	EEIObject *m_pParent;
};

} // namespace

#endif