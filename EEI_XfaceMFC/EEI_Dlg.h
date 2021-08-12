#pragma once
#include "afxwin.h"

#include "Settings.h"
#include "VehicleTypes.h"
//#include "ParameterGroup.h"
#include "EEI_GUI.h"
#include "UnorderedContainer.h"

// EEI_Dlg dialog

class EEI_Dlg : public CDialog
{
	DECLARE_DYNAMIC(EEI_Dlg)

public:
	EEI_Dlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~EEI_Dlg();

	virtual BOOL OnInitDialog();

	void updateXSectionLB();
	void updateVehTypesLB();

//	typedef std::pair<std::string, EEI_GUI> guiMapEntry;
//	typedef std::vector<guiMapEntry> guiMap;

	typedef UnorderedContainer<std::string, EEI_GUI> guiMap;
	typedef guiMap::iterator iterator;

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

	bool SetupParameters(const char *path, const eei::VehicleTypes &vt);

	void setLogLevels(const char *logLevels);
	std::string getLogLevels();

	void setReplications(int);
	int getReplications();

	EEI_Dlg &addXSection(const char *key, EEI_GUI &x);
	EEI_Dlg &removeXSection(const char *key);

	// TODO: consider moving selected into EEI and using it as a single param block
	//		 alternatively, extend VehicleTypes to include a 'selected' member with appropriate iterators
	//		 to return all or the selected only

	// vehTypes: used to initialize the dialog with ALL possible vehicle types
	eei::VehicleTypes &vehTypes() { return m_vehTypes; }

	// vehTypesSelected: vehicle types the user selected as 'equipped'
	eei::VehicleTypes &vehTypesSel() { return m_vehTypesSel; }

	EEI_GUI toStruct();
	void fromStruct(EEI_GUI &x);

	iterator begin() { return m_guiMap.begin(); }
	iterator end() { return m_guiMap.end(); }
	iterator find(const char *key){ return m_guiMap.find(key); }
//	iterator find(const char *key){ for (guiMap::iterator i = m_guiMap.begin(); i!= m_guiMap.end(); i++) if (i->first == key) return i; return m_guiMap.end(); }
	size_t size() { return m_guiMap.size(); }
//	EEI_GUI &add(const char *name, EEI_GUI &gui) { if (find(name) == m_guiMap.end()) m_guiMap.push_back(guiMapEntry(name, gui)); return m_guiMap[m_guiMap.size()-1].second; }
	EEI_GUI &add(const char *name, EEI_GUI &gui) { return (m_guiMap[name] = gui); }

	const char *getSettingsFileName() { return m_filename.IsEmpty() ? "" : m_filename; }
	bool isEmptySettingsFileName() { return m_filename.IsEmpty() == TRUE; }

	void CancelTimer()
	{
		if (m_timer) 
			KillTimer(m_timer);
		m_timer = 0;
	}
	void updateMap(bool fromTimer=false);

	void newMode(bool b);
	bool newMode() { return m_newMode; }

	HBRUSH m_redBR;
//	CFont m_fontCourier;
	bool m_newMode;
	guiMap m_guiMap;
	UINT_PTR m_timer;

	eei::VehicleTypes m_vehTypes;
	eei::VehicleTypes m_vehTypesSel;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedAddBtn();
	afx_msg void OnBnClickedRemoveBtn();
	afx_msg void OnLbnSelchangeList1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnEnChangeEdit6();
	afx_msg void OnEnChangeEdit8();
	afx_msg void OnEnChangeEdit10();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedNewBtn();
	afx_msg void OnLbnSelchangeVehicles();
	afx_msg void OnBnClickedButtonUse();
	afx_msg void OnBnClickedCheck2();

	int m_JunctionID;
	int m_SignalGroup;
	double m_StartPoint;
	double m_EndPoint;

	CEdit m_filenameCtl;
	CString m_filename;
	double m_accelFactor;
	BOOL m_enableCFR;
	CString m_sectionID;
	double m_infZoneCalibParam;
	CString m_XSectionName;
	CListBox m_XSectionLB;
	double m_minSpeed;
	CStatic m_newIndicator;
	CListBox m_VehicleTypes;
	BOOL m_logAll;
	BOOL m_logErr;
	BOOL m_logMsg;
	BOOL m_EnforceMinSpeed;
	int m_NumReplications;
};
