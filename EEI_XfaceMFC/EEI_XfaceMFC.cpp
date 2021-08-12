// EEI_XfaceMFC.cpp : Defines the initialization routines for the DLL.
//

#include "Precompiled.h"
#include "EEI_XfaceMFC.h"

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// CEEI_XfaceMFCApp

BEGIN_MESSAGE_MAP(CEEI_XfaceMFCApp, CWinApp)
END_MESSAGE_MAP()


// CEEI_XfaceMFCApp construction

CEEI_XfaceMFCApp::CEEI_XfaceMFCApp()
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	LOG("==== APP CTOR =====\n");
}

CEEI_XfaceMFCApp::~CEEI_XfaceMFCApp()
{
	LOG("==== APP DTOR =====\n");
	_CrtDumpMemoryLeaks();
}


// The one and only CEEI_XfaceMFCApp object

CEEI_XfaceMFCApp theApp;


// CEEI_XfaceMFCApp initialization

BOOL CEEI_XfaceMFCApp::InitInstance()
{
	CWinApp::InitInstance();


	return TRUE;
}

int CEEI_XfaceMFCApp::ExitInstance()
{
	int r = CWinApp::ExitInstance();
	return r;
}