#ifndef _EEI_GUI_H_
#define _EEI_GUI_H_


#include "ParameterGroup.h"
class Settings;
struct Settings::paramGroup;

struct EEI_GUI : public ParameterGroup
{
	EEI_GUI() { }
	EEI_GUI(const ParameterGroup &c) {	*this = c;}
	EEI_GUI(const EEI_GUI &c) {	*this = c;}

	EEI_GUI &operator=(const ParameterGroup &pg) 
	{
		ParameterGroup::operator=(pg);
		return *this;
	}

	EEI_GUI &operator=(const EEI_GUI &c)
	{
		if (this != &c)
		{
			ParameterGroup::operator=(c);
			m_vehTypesSelected = c.m_vehTypesSelected;
		}
		return *this;
	}
	std::vector<int> m_vehTypesSelected;
};


//EEI_GUI &operator<< (EEI_GUI &gui, const Settings::paramGroup &sg);
//const EEI_GUI &operator>> (const EEI_GUI &gui, Settings::paramGroup &sg);

inline EEI_GUI &operator<< (EEI_GUI &gui, const Settings::paramGroup &sg)
{
	gui = sg;
	return gui;
}

inline const EEI_GUI &operator>> (const EEI_GUI &gui, Settings::paramGroup &sg)
{
	sg = gui;
	return gui;
}

#endif
