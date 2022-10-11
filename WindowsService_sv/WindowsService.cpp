// WindowsService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define UNICODE
#include <iostream>
#include "stdio.h"
#include "Windows.h"

#pragma warning(disable : 4996)

#define SVCNAME TEXT	("123")
#define BUFFERSIZE 512

using namespace std;

SERVICE_STATUS          gSvcStatus;
SERVICE_STATUS_HANDLE   gSvcStatusHandle;
HANDLE                  ghSvcStopEvent;
LPCTSTR PipeName = TEXT("\\\\.\\pipe\\ServicePipe"); 

char szErrorLog[MAX_PATH + 1] = { 0 };
char Temp[BUFFERSIZE] = { 0 };

int ReceivePiped(HANDLE hPiped);
int WriteToLog(char* str);
int WriteToLogUnicode(wchar_t* str);

#define LOGFILE "C:\\Users\\ADMIN\\Desktop\\WindowsForm\\WINDOWS MFC\\WindowsService\\output.log"

void SvcInstall();
void WINAPI SvcMain();
void WINAPI SvcCtrlHandler(DWORD dwSvcCtrlHandler);

int main()
{

	SvcInstall();
	
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ (LPWSTR)SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
		{ NULL, NULL }
	};
	
	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
		sprintf(szErrorLog, "StartServiceCrtlDispatcher error GLE=%d\n", GetLastError());
		WriteToLog(szErrorLog);
	}
	else {
		sprintf(szErrorLog, "StartServiceCtrlDispatcher() already\n");
		WriteToLog(szErrorLog);
	}
	
	
}

void SvcInstall()
{

	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	TCHAR szPath[MAX_PATH];

	if (GetModuleFileName(NULL, szPath, MAX_PATH) == FALSE)
	{
		cout<< "Cannot install service\n" << GetLastError() <<endl;
		return;
	}

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	
	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		//viết vào WrieToLog((char*)"failed to open")
		return;
	}

	schService = CreateService(
		schSCManager,					// SCM database 
		SVCNAME,						// name of service 
		SVCNAME,						// service name to display 
		SERVICE_ALL_ACCESS,				// desired access 
		SERVICE_WIN32_OWN_PROCESS,		// service type 
		SERVICE_DEMAND_START,			// start type 
		SERVICE_ERROR_NORMAL,			// error control type 
		szPath,							// path to service's binary 
		NULL,							// no load ordering group 
		NULL,							// no tag identifier 
		NULL,							// no dependencies 
		NULL,							// LocalSystem account 
		NULL);							// no password 

	if (schService == NULL)
	{
		cout<< "CreateService failed (%d)\n" << GetLastError() <<endl;
		CloseServiceHandle(schSCManager);
		return;
	}
	else cout<< "Service installed successfully\n";
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return;

}

void WINAPI SvcMain()
{
	
	BOOL fConnected = FALSE;
	ghSvcStopEvent = CreateEvent(
		NULL,    // default security attributes
		TRUE,    // manual reset event
		FALSE,   // not signaled
		NULL);   // no name

	gSvcStatusHandle = RegisterServiceCtrlHandler(

		SVCNAME,
		SvcCtrlHandler);

	//Hàm ServiceMain gọi hàm RegisterServiceCtrlHandler để đăng kí 
	//hàm SvcCtrlHandler xử lý  các request do “control dispatcher” gửi về từ SCM. 
	//Hàm RegisterServiceCtrlHandler trả về status_handle.

	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	gSvcStatus.dwCurrentState = SERVICE_START_PENDING;
	gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	gSvcStatus.dwWin32ExitCode = 0;
	gSvcStatus.dwServiceSpecificExitCode = 0;
	gSvcStatus.dwCheckPoint = 0;
	gSvcStatus.dwWaitHint = 0;

	// Chuyển thành Running
	gSvcStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
	
	while (gSvcStatus.dwCurrentState == SERVICE_RUNNING)
	{
		HANDLE hPipe = CreateNamedPipe(
			PipeName,												// pipe name 
			PIPE_ACCESS_DUPLEX,										// read/write access 
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,  // message type pipe | message-read mode | blocking mode 
			PIPE_UNLIMITED_INSTANCES,								// max. instances  
			BUFFERSIZE,												// output buffer size 
			BUFFERSIZE,												// input buffer size 
			0,														// client time-out 
			NULL);													// default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE)
		{
			sprintf(szErrorLog, "CreateNamedPipe failed GLE=%d\n", GetLastError());
			WriteToLog(szErrorLog);
			return;
		}
		WriteToLog((char*)"wait for ConnectNamedPipe\n");

		//Wait for the client to connect 
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		if (fConnected)
		{			
			ReceivePiped(hPipe);
			WriteToLog((char*)"Connect Successed\n");
		}
	}
	return;
}

void WINAPI SvcCtrlHandler(DWORD dwSvcCtrlHandler)
{
	switch (dwSvcCtrlHandler)
	{
	case SERVICE_CONTROL_STOP:

		gSvcStatus.dwWin32ExitCode = 0;
		gSvcStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
		WriteToLog((char*)"Service Stopped\n");
		return;

	case SERVICE_CONTROL_SHUTDOWN:

		//SetEvent(ghSvcStopEvent);
		gSvcStatus.dwWin32ExitCode = 0;
		gSvcStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
		WriteToLog((char*)"Service Shutdown\n");
		return;

	default:
		break;
	}
}

int WriteToLog(char* str)
{
	FILE* fpLog;
	fpLog = fopen(LOGFILE, "a+");

	if (fpLog == NULL)
		return -1;

	if (fprintf(fpLog, "%s\n", str) < strlen(str))
	{
		fclose(fpLog);
		return -1;
	}

	fclose(fpLog);
	return 0;
}

int WriteToLogUnicode(wchar_t* str)
{
	FILE* fpLog;
	fpLog = fopen(LOGFILE, "a+");

	if (fpLog == NULL)
		return -1;
	char buffer[500] = { 0 };

	int nchar = WideCharToMultiByte(CP_UTF8, 0, str, -1,
		buffer, 500, NULL, NULL);

	if (fprintf(fpLog, "%s\n", buffer) < nchar)
	{
		fclose(fpLog);
		return -1;
	}

	fclose(fpLog);
	return 0;
}

int ReceivePiped(HANDLE hPipe)
{
	char WriteBuff[100] = { 0 }; //ghi log

	wchar_t DataBuff[BUFFERSIZE] = { 0 };
	wchar_t* chReadBuffer = (wchar_t*)malloc(BUFFERSIZE);	//nhận dữ liệu

	HANDLE hFile;
	BOOL fSuccess = FALSE;
	DWORD cbBytesRead, cbReplyBytes, cbWritten, cbToWrite;

	// nhận thông tin từ giao diện
	while (1)
	{
		// Read from the pipe if there is more data in the message.
		fSuccess = ReadFile(
					hPipe,									// pipe handle 
					chReadBuffer,							// buffer to receive reply 
					BUFFERSIZE * sizeof(TCHAR),				// size of buffer 
					&cbBytesRead,							// number of bytes read 
					NULL);									// not overlapped 
		
		//Đọc không thành công hoặc không có byte nào từ message
		if (fSuccess == FALSE || cbBytesRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE) {
				sprintf(WriteBuff, "Client Disconnected Error: %d\n", GetLastError());
				WriteToLog(WriteBuff);
			}
			else {
				ZeroMemory(WriteBuff, sizeof(WriteBuff));
				sprintf(WriteBuff, "ReadFile from service failed Error: %d\n", GetLastError());
				WriteToLog(WriteBuff);
			}
			break;
		}
		WriteToLogUnicode(chReadBuffer);

		//Tạo file
		lstrcpynW(DataBuff, chReadBuffer, lstrlenW(L"CreateFile:") + 1);
		if (lstrcmpW(DataBuff, L"CreateFile:") == 0) 
		{
			//copy pathlink vào databuff
			lstrcpynW(DataBuff, &chReadBuffer[11], lstrlenW(chReadBuffer) + 1);
			WriteToLogUnicode(DataBuff);

			if ((hFile = CreateFile(DataBuff, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
									CREATE_ALWAYS, NULL, NULL)) != INVALID_HANDLE_VALUE)
			{
				sprintf(WriteBuff, "CreateFile Successed: %d\n", GetLastError());
				WriteToLog(WriteBuff);
			}
			else
			{
				sprintf(WriteBuff, "CreateFile failed-Error: %d\n", GetLastError());
				WriteToLog(WriteBuff);
			}
			CloseHandle(hFile);
			break;
		}
		
		//Xoá file
		lstrcpynW(DataBuff, chReadBuffer, lstrlenW(L"Delete:") + 1);
		if (lstrcmpW(DataBuff, L"Delete:") == 0) 
		{
			//copy pathlink vào databuff
			lstrcpynW(DataBuff, &chReadBuffer[7], lstrlenW(chReadBuffer) + 1);
			WriteToLogUnicode(DataBuff);
			if ((hFile = CreateFile(DataBuff, GENERIC_READ,FILE_SHARE_READ | FILE_SHARE_DELETE, 
									NULL, OPEN_EXISTING, NULL, NULL)) != INVALID_HANDLE_VALUE)
			{
				if (DeleteFileW(DataBuff))
				{
					sprintf(WriteBuff, "DeleteFile Successed= %d\n", GetLastError());
					WriteToLog(WriteBuff);
				}
				else
				{
					sprintf(WriteBuff, "DeleteFile Failed-Error= %d\n", GetLastError());
					WriteToLog(WriteBuff);
				}
			}
			else 
			{
				sprintf(WriteBuff, "DeleteFile Error= %d\n", GetLastError());
				WriteToLog(WriteBuff);
			}
			CloseHandle(hFile);
		}
	}
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	return 0;
}

