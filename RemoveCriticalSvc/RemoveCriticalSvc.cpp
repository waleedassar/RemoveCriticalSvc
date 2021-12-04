// RemoveCriticalSvc.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "windows.h"
#include "stdio.h"
#include "Header.h"


#pragma comment(lib,"ntdll.lib")






//global
SERVICE_STATUS_HANDLE hSta;
SERVICE_STATUS S_Sta ={0};
HANDLE hThread = 0;
ulong ThreadId = 0;


DWORD MyHandler(
  DWORD dwControl,
  DWORD dwEventType,
  LPVOID lpEventData,
  LPVOID lpContext
)
{
	if(dwControl == SERVICE_CONTROL_STOP || dwControl == SERVICE_CONTROL_SHUTDOWN)
	{
		S_Sta.dwCurrentState=SERVICE_STOP_PENDING;
		SetServiceStatus(hSta,&S_Sta);

		if(hThread)
		{
			TerminateThread(hThread,0);
			CloseHandle(hThread);
			hThread = 0;
			ThreadId = 0;
		}

		Sleep(50);
		S_Sta.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hSta,&S_Sta);
		return NO_ERROR;
	}
	else if(dwControl == SERVICE_START)
	{
		S_Sta.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(hSta,&S_Sta);
		return NO_ERROR;
	}
	else if(dwControl == SERVICE_CONTROL_INTERROGATE )
	{
		return NO_ERROR;
	}
	return NO_ERROR;//ERROR_CALL_NOT_IMPLEMENTED;
};







int ThreadCallback(ulonglong ThreadId)
{
	HANDLE hThread = OpenThread(THREAD_SET_INFORMATION,FALSE,ThreadId);
	if(hThread)
	{
		ulong Set_Unset = 0;
		int ret = ZwSetInformationThread(hThread,ThreadBreakOnTermination,&Set_Unset,0x4);
		printf("(TID: %I64X) ZwSetInformationThread, ret: %X\r\n",ThreadId,ret);
		//getchar();
		CloseHandle(hThread);
		if(ret < 0) return ret;
		return 0;
	}
	return -1;
}


int ProcessCallback(ulonglong ProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION ,FALSE,ProcessId);
	if(hProcess)
	{
		ulong Set_Unset = 0;
		int ret = ZwSetInformationProcess(hProcess,ProcessBreakOnTermination,&Set_Unset,0x4);
		printf("(PID: %I64X) ZwSetInformationProcess, ret: %X\r\n",ProcessId,ret);
		//getchar();
		CloseHandle(hProcess);
		if(ret < 0) return ret;
		return 0;
	}
	return -1;
}

void ThreadProc()
{
	while(1)
	{
		EnumThreads(ProcessCallback,ThreadCallback);
		Sleep(100);
	}
}


extern "C" __declspec(dllexport) void  ServiceMain(int argc, void* argv[])//Args not used
{
	hSta = RegisterServiceCtrlHandlerExW(L"RemoveCriticalSvc",(LPHANDLER_FUNCTION_EX)MyHandler,0);

	S_Sta.dwServiceType = SERVICE_WIN32_SHARE_PROCESS; 
    S_Sta.dwServiceSpecificExitCode = 0; 
	S_Sta.dwCurrentState = SERVICE_START_PENDING;
	S_Sta.dwWin32ExitCode=0;
	S_Sta.dwControlsAccepted = 0;


	S_Sta.dwCurrentState= SERVICE_RUNNING;
	S_Sta.dwControlsAccepted=SERVICE_ACCEPT_STOP;
	SetServiceStatus(hSta,&S_Sta);



	//Here

	bool bRet = Acquire_Debug();
	if(!bRet)
	{
		printf("Can't acquire SeDebugPrvilege\r\n");
		return;
	}
	else
	{
		bRet = Acquire_Tcb();
		if(!bRet)
		{
			printf("Can't acquire SeTcbPrvilege\r\n");
			return;
		}
		else
		{
			hThread = CreateThread(0,0x1000,(LPTHREAD_START_ROUTINE)ThreadProc,0,0,&ThreadId);
		}
	}


	
	//Sleep(-1);
	return;
}


