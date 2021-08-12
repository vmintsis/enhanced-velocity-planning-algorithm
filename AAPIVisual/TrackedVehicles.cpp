#include "Precompiled.h"
#include "TrackedVehicles.h"

namespace eei
{

//--------------------------------------------------
//
//--------------------------------------------------
TrackedVehicles::TrackedVehicles(EEIObject *parent/*=0*/)
	: m_pParent(parent), _ncvXform(NULL)
{
}

//--------------------------------------------------
//
//--------------------------------------------------
TrackedVehicles::~TrackedVehicles()
{
	if (_ncvXform) delete _ncvXform;
}

//--------------------------------------------------
//
//--------------------------------------------------
bool TrackedVehicles::advisedVehicle(int idVeh)
{
	VelProfMap::iterator it = m_vmap.find(idVeh);
	return (it != m_vmap.end());
}

//--------------------------------------------------
//
//--------------------------------------------------
bool TrackedVehicles::hasProfile(int idVeh)
{
	return advisedVehicle(idVeh) && m_vmap[idVeh].isValid();
}

//--------------------------------------------------
//
//--------------------------------------------------
TrackedVehicles::CF_Info TrackedVehicles::vehicleInfo(int idVeh) 
{
	for (VehID::iterator iv = m_tracked.begin(); iv!= m_tracked.end(); iv++)
		if (iv->idVeh() == idVeh)
			return *iv;
	return CF_Info();
}

//--------------------------------------------------
//
//--------------------------------------------------
VelProfile &TrackedVehicles::vehicleProfile(int idVeh)
{
	if (!advisedVehicle(idVeh))
	{
		trackVehicle(idVeh);
		SP.f_SetAsAdvised(idVeh);
		SP.f_SetAsCompliant(idVeh);
		m_vmap[idVeh] = VelProfile().setParent(m_pParent);
	}
	return m_vmap[idVeh];
}

//--------------------------------------------------
//
//--------------------------------------------------
int TrackedVehicles::trackVehicle(int idVeh)
{
	int r = 0;
	if (!isTracked(idVeh))
	{
		r = SP.f_AKIVehSetAsTracked(idVeh);
		if (r>= 0)
			m_tracked.push_back( CF_Info(idVeh) );
	}
	return r;
}
//--------------------------------------------------
//
//--------------------------------------------------
void TrackedVehicles::xformNonCompliantVehicleProfile(int idVeh, bool c)
{
	if (_ncvXform)
	{
		VelProfMap::iterator it = m_vmap.find(idVeh);
		if (it != m_vmap.end())
		{
			if (c)
				SP.f_SetAsCompliant(idVeh);
			else
			{
				SP.f_SetAsNotCompliant(idVeh);
		
				it->second.transForm( *_ncvXform );
			}
		}
	}
}

//--------------------------------------------------
//
//--------------------------------------------------
bool TrackedVehicles::removeVehicle(int idVeh, int remove/*=REMOVE_ALL*/)
{
	bool found = false;
	if ( (remove & REMOVE_ADVISE) == REMOVE_ADVISE)
	{
		VelProfMap::iterator it = m_vmap.find(idVeh);
		found = it != m_vmap.end();
		if (found)
		{
			m_vmap.erase(it);
			SP.f_SetAsNotAdvised(idVeh);
			SP.f_SetAsNotCompliant(idVeh);
		}
	}

	if ( (remove & REMOVE_REASONS_MASK) != 0)
	{
		for (VehID::iterator iv = m_tracked.begin(); iv!= m_tracked.end(); iv++)
			if (iv->idVeh() == idVeh)
			{
				iv->addflag(remove & REMOVE_REASONS_MASK);
			}
	}

//	bool tracked = false;
	if ( (remove & REMOVE_TRACKING) == REMOVE_TRACKING)
	{
		for (VehID::iterator iv = m_tracked.begin(); iv!= m_tracked.end(); iv++)
			if (iv->idVeh() == idVeh)
			{
//				tracked = true;
				m_tracked.erase(iv);
				SP.f_AKIVehSetAsNoTracked(idVeh);
				break;
			}
	}

	return found;
}

//--------------------------------------------------
//
//  [1] : Operational Analysis of Traffic Dynamics, Louis A Pipes, Journal of Applied Physics, March 1953
//
//--------------------------------------------------
bool TrackedVehicles::isCarFollowing(int idVeh)
{
	bool r = false;

	const double a = 1.02;  // sec  : time constant prescribed by the postulated "traffic law" suggested by the California Vehicle Code Summary [1]
	const double b = 1.5;	// meters : distance between vehicles at standstill [1]

	enum CF_Rules
	{
		CFR_First = -1,

		CFR_empiricalRule,
		CFR_practicalRule,

		CFR_Last
	};

	// TODO: add this to GUI
	CF_Rules rule = CFR_empiricalRule;
	// CF_Rules rule = CFR_practicalRule;

	// locate the vehicle requested
	long iCV = -1;
	for (size_t i=0; i<m_tracked.size(); i++)
		if (m_tracked[i].idVeh() == idVeh)
		{
			iCV = (long)i;
			break;
		}

	if (iCV > 0)
	{
		CF_Info &cur = m_tracked[iCV];
		long i;
		for (i=iCV-1; i >= 0 && m_tracked[i].lane() != cur.lane(); i--)
			;
		if (i >= 0) // at least one vehicle in front of 'cur' in the same lane: 'cur' is behind this vehicle
		{
			CF_Info &front = m_tracked[i];

			double vdist = front.distFromStart() - cur.distFromStart() - front.vehicleLength();
			
			double distLimit =	-1.0;
			switch(rule)
			{
				case CFR_empiricalRule: 
						distLimit = a * cur.currentSpeed() + b; 
						break;
				case CFR_practicalRule: 
						distLimit = pow(cur.currentSpeed(), 2) / (2 * fabs(cur.normalDeceleration())) + cur.reactionTime() * cur.currentSpeed(); 
						break;

				default:
					// TODO: return an error value
					break;
			}

			r = (vdist < distLimit);
			pflog("[%d] Car-following (%s) influence length: vid=%d, lane=%d, actual dist=%f, min dist=%f\n", iCV, (r ? "Yes" : "No"), cur.idVeh(), cur.lane(), vdist, distLimit);
		}
	}

	return r;
}

}// namespace
