#ifndef _EEIManager_H_
#define _EEIManager_H_

#include "EnergyEfficientIntersection.h"
#include "TSptr.h"

namespace eei
{
	typedef TSptr<EnergyEfficientIntersection> EEI_ptr;



	class EEIManager
	{
		// prevent default construction and copying
		EEIManager();
		EEIManager(const EEIManager &st);
		EEIManager &operator=(const EEIManager &st);
	public:
		~EEIManager();

		typedef std::map<std::string, EEI_ptr> eeiMap;
		typedef eeiMap::iterator iterator;

		static EEIManager &instance();

		std::string scenarioName() { return _scenarioName; }
		int scenarioId() { return _scenarioId; }
		int LoadScenario();

		EEI_ptr add(const char *key);
		EEI_ptr get(const char *key);
		size_t size();

		iterator begin() { return _map.begin(); }
		iterator end() { return _map.end(); }

		void enable() { m_enabled = true; }
		void disable() { m_enabled = false; }
		bool isEnabled() { return m_enabled; }
		void clear() { _map.clear(); }

		std::string getSettingsPath() { return m_settingsPath; }
		void setSettingsPath(const char *p) { REQUIRE(p != 0); m_settingsPath = p; }

		VehicleTypes &vehTypes() { return m_vehTypes; }

	private:
		int _scenarioId;
		std::string _scenarioName;
		eeiMap _map;
		bool m_enabled;
		std::string m_settingsPath;
		VehicleTypes m_vehTypes;
	};

} // namespace

#endif

