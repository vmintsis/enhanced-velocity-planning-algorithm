#include "Precompiled.h"

#include "EEIManager.h"
#include "EnergyEfficientIntersection.h"

namespace eei
{

//-----------------------------------------
//
//-----------------------------------------
EEIManager::EEIManager()
	: m_enabled(true)
{
	_scenarioId = SP.f_ANGConnGetScenarioId();
	_scenarioName = SP.f_ANGConnGetObjectNameA(_scenarioId);

	char bf[1024];
	sprintf(bf, "%s\\EEI_SCParams_%d.eei", logs_location().c_str(), _scenarioId);
	m_settingsPath = bf;

	LOG("EEIManager CTOR\n");
}

//-----------------------------------------
//
//-----------------------------------------
EEIManager::~EEIManager()
{
	// cleanup here
	LOG("EEIManager DTOR\n");
}

//-----------------------------------------
//
//-----------------------------------------
EEIManager &EEIManager::instance()
{
	static EEIManager _eeim;
	return _eeim;
	
};

//-----------------------------------------
//
//-----------------------------------------
int EEIManager::LoadScenario()
{
	CLog::log()->show("EEIManager Load\n" );

	return 0;
}

//-----------------------------------------
//
//-----------------------------------------
EEI_ptr EEIManager::add(const char *key)
{
	REQUIRE( key != 0);
	eeiMap::iterator it = _map.find(key);
	if (it == _map.end())
	{
		EEI_ptr eei = EEI_ptr().newObj();
		eei->setTitle(key);
		_map[key] = eei;
		return eei;
	}

	return EEI_ptr();
}

//-----------------------------------------
//
//-----------------------------------------
EEI_ptr EEIManager::get(const char *key)
{
	REQUIRE(key != 0);

	eeiMap::iterator it = _map.find(key);
	if (it != _map.end())
		return it->second;

	return EEI_ptr();
}

//-----------------------------------------
//
//-----------------------------------------
size_t EEIManager::size()
{
	return _map.size();
}

} //  namespace


