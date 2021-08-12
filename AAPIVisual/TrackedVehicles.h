#ifndef _TrackedVechicles_H_
#define _TrackedVechicles_H_

#include "Velocityprofile.h"
#include "VehComplianceProfileXForm.h"
#include "EEIObject.h"

namespace eei
{

/////////////////////////////////////////////
//
//
class TrackedVehicles
{
public:

	// tracking and advisory state flags
	enum 
	{ 
		// default: vehicle is being tracked (initial value of tracking state flags)
		TRACKED_DEFAULT = 0x0000,

		// actions
		REMOVE_ACTIONS_MASK		= 0x00ff,
		REMOVE_ADVISE			= 0x0001,	// remove vehicle from advisory only
		REMOVE_TRACKING			= 0x0002,	// remove vehicle from tracking list only
		REMOVE_COMPLIANCE		= 0x0004,	// set vehicle as non-compliant to speed advise

		// reasons
		REMOVE_REASONS_MASK			= 0xff00,
		REMOVE_REASON_CAR_FOLLOWING = 0x0100,	// vehicle is removed due to car following
		REMOVE_REASON_MIN_SPEED		= 0x0200,	// vehicle is removed due to minimum speed threshold rule

		REMOVE_ALL = REMOVE_ADVISE | REMOVE_TRACKING | REMOVE_COMPLIANCE	// remove vehicle completely

		// Note: Compliance attribute semantics:
		//		 1 - Vehicle is definitely following speed advise
		//		 0 - Vehicle is following a 'perturbed' profile that simulates non-compliance to proper speed advise.
		//			 In effect, non-compliance indicates an indeterminate compliance state (driver may or may not follow advise).
		//		     (Tentative, semantics subject to future review/clarification)
	};

	// vehicle tracking information
	struct CF_Info
	{
		CF_Info() : _idVeh(-1), _distFromStart(0.0), 
					_vehLength(0.0), _normDecel(0.0), 
					_curSpeed(-1.0), _reactionTime(0.0),
					_flags(TRACKED_DEFAULT)
		{ }

		CF_Info(int id) : _idVeh(id), _distFromStart(0.0), 
						  _vehLength(0.0), _normDecel(0.0), 
						  _curSpeed(-1.0), _reactionTime(0.0),
						  _flags(TRACKED_DEFAULT)
		{ }

		bool isValid() { return _idVeh != -1 && _distFromStart != 0.0 && 
								_vehLength > 0.0 && _normDecel != 0.0 && 
								_curSpeed >= 0.0 && _reactionTime > 0.0; }

		int idVeh() const { return _idVeh; }
		double vehicleLength() const { return _vehLength; }
		double distFromStart() const { return _distFromStart; }
		double normalDeceleration() const { return _normDecel; }
		double currentSpeed() const { return _curSpeed; }
		double reactionTime() const { return _reactionTime; }
		int lane() const { return _lane; }

		void setIdVeh(int i) { _idVeh = i; }
		void setVehicleLength(double  l) { _vehLength = l; }
		void setDistFromStart(double d) { _distFromStart = d; }
		void setNormalDeceleration(double a) { _normDecel = a; }
		void setCurrentSpeed(double v) { _curSpeed = v; }
		void setReactionTime(double t) { _reactionTime = t; }
		void setLane(int  n) { _lane = n; }

		unsigned int flags() const { return _flags; }
		void addflag(unsigned int f) { _flags |= f; }
		void removeflag(unsigned int f){ _flags &= ~f; }

	private:
		int _idVeh;				// vehicle ID
		double _vehLength;		// vehicle length (m)
		double _distFromStart;	// distance to traffic lights ahead (m)
		double _normDecel;		// normal deceleration (m/s^2) (negative)
		double _curSpeed;		// current vehicle speed (m/s)
		double _reactionTime;	// driver reaction time (s)
		int _lane;				// lane number (scalar)

		unsigned int _flags;	// flags indicating conditions under which 
								// this vehicle is being tracked (def. TRACKED_DEFAULT)
	};

	typedef std::map<int, VelProfile> VelProfMap;	// velocity profiles type of all vehicles entering the above interval
	typedef std::vector<CF_Info> VehID;			    // collection of tracked vehicle IDs
	typedef VehID::iterator iterator;

	// construction/destruction
public:
	TrackedVehicles(EEIObject *parent=0);
	virtual ~TrackedVehicles();

public:
	void setParent(EEIObject *parent) { m_pParent = parent; }

	bool advisedVehicle(int idVeh);
	bool hasProfile(int idVeh);

	iterator begin() { return m_tracked.begin(); }
	iterator end() { return m_tracked.end(); }
	bool isTracked(int idVeh) 
	{ 
		for (iterator it = m_tracked.begin(); it != m_tracked.end(); ++it)
			if (it->idVeh() == idVeh) return true;
		return false;
	}

	int trackVehicle(int idVeh);
	bool removeVehicle(int idVeh, int remove=REMOVE_ALL); // if unadvise_only == true, the vehicle remains tracked, but will not receive any more advise
	void xformNonCompliantVehicleProfile(int idVeh, bool c);
	void setNonCompliantVehicleXForm(IVehComplianceProfileXForm *xform)
	{
		if (_ncvXform) delete _ncvXform;
		_ncvXform = xform;
	}
	IVehComplianceProfileXForm *getNonCompliantVehicleXForm() { REQUIRE(_ncvXform != NULL); return _ncvXform; }

	template<class F>
	void updateCarFollowingInformation(F getInfo)
	{
		for (size_t i=0; i<m_tracked.size(); i++)
		{
			getInfo(m_tracked[i].idVeh()); // call functor to obtain the needed vehicle information

			m_tracked[i].setVehicleLength( getInfo.vehicleLength() );
			m_tracked[i].setDistFromStart( getInfo.distFromStart() );
			m_tracked[i].setNormalDeceleration( getInfo.normalDeceleration() );
			m_tracked[i].setCurrentSpeed( getInfo.currentSpeed() );
			m_tracked[i].setReactionTime( getInfo.reactionTime() );
			m_tracked[i].setLane( getInfo.lane() );
		}

		struct OrderByDistDesc
		{
			bool operator()(const CF_Info &c1, const CF_Info &c2) { return c1.distFromStart() > c2.distFromStart(); }
		};
		std::sort(m_tracked.begin(), m_tracked.end(), OrderByDistDesc() );

		for (size_t i=0; i<m_tracked.size(); i++)
			pflog("CF[%d]: vehID=%d  distFromStart=%f m,  speed=%f km/h\n", i, m_tracked[i].idVeh(), m_tracked[i].distFromStart(), UnitConv().mps2kmph(m_tracked[i].currentSpeed()) );
	}

	bool isCarFollowing(int idVeh);
	CF_Info vehicleInfo(int idVeh);
	VelProfile &vehicleProfile(int idVeh);	// distFromStart must be provided for new vehicles being added and if provided for existing vehicles, it will update the distance in m_tracked

private:
	IVehComplianceProfileXForm *_ncvXform;	// profile transform applied to non-compliant vehicles (def. NULL)
	VehID m_tracked;			// vehicles we are tracking in order of ascending order of distance from start of section
	VelProfMap m_vmap;			// vehicle velocity profiles 
	EEIObject *m_pParent;
};

} // namespace

#endif
