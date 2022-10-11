// DisplayService.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "DisplayService.h"
#include "Windows.h"
#include "stdio.h"
#include "tchar.h"
#include <iostream>
#include <commdlg.h>
#include <fstream>

using namespace std;

#define MAX_LOADSTRING 100
#define CREATE_BUTTON 1
#define DELETE_BUTTON 2
#define OPEN_BUTTON 3
#define BUFFERSIZE 500
#pragma warning(disable : 4996)

LPCTSTR PipeName = TEXT("\\\\.\\pipe\\ServicePipe");
#define LOGFILE "C:\\Users\\ADMIN\\Desktop\\WindowsForm\\WINDOWS MFC\\WindowsService\\output.log"

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

void OpenFileDlg(HWND hWnd);
void CreateFileDlg(HWND hWnd);
void DeleteFileDlg(HWND hWnd);
int PipeClientConnect(wchar_t* argv);

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND hWnd, hBtnCreate, hBtnDelete, hStatic, hInput, hBtnOpenfile;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DISPLAYSERVICE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DISPLAYSERVICE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DISPLAYSERVICE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_BTNFACE +1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DISPLAYSERVICE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{

   hInst = hInstance; // Store instance handle in our global variable
   hWnd = CreateWindowW(szWindowClass, L"WindowService", WS_OVERLAPPEDWINDOW
                       ,520, 70, 450, 650, nullptr, nullptr, hInstance, nullptr);

   //Nút tạo / xoá file
   hBtnCreate = CreateWindowW(L"Button", L"Create File", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER
                       , 120, 280, 200, 40, hWnd, (HMENU)CREATE_BUTTON, NULL, NULL);
   hBtnDelete = CreateWindowW(L"Button", L"Delete File", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER
                       , 120, 215, 200, 40, hWnd, (HMENU)DELETE_BUTTON, NULL, NULL);
   hBtnOpenfile = CreateWindowW(L"Button", L"Open Browser", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER
                       , 120, 160, 200, 40, hWnd, (HMENU)OPEN_BUTTON, NULL, NULL);
   
   hStatic = CreateWindowW(L"Static", L"Path Link", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER
                       , 120, 80, 200, 40, hWnd, NULL, NULL, NULL);
   hInput = CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTER | ES_MULTILINE | ES_AUTOVSCROLL
                       , 20, 120, 400, 40, hWnd, NULL, NULL, NULL);
   
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;

}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

//Tạo-File
void CreateFileDlg(HWND hWnd)
{
    wchar_t Temp[512] = { 0 };
    wchar_t Tittle[256] = { 0 };
    lstrcatW(Temp,L"CreateFile:");
    GetWindowText(hInput, Tittle, 256);
    lstrcatW(Temp, Tittle);
    
    PipeClientConnect(Temp);
}

//Xoá-File
void DeleteFileDlg(HWND hWnd)
{
    wchar_t Temp[512] = { 0 };
    wchar_t Tittle[256] = { 0 };
    lstrcatW(Temp, L"Delete:");
    GetWindowText(hInput, Tittle, 256);
    lstrcatW(Temp, Tittle);

    PipeClientConnect(Temp);
}

void OpenFileDlg(HWND hWnd)
{
    OPENFILENAME Ffile;
    TCHAR hFileIn[MAX_PATH];
    ZeroMemory(&Ffile, sizeof(Ffile));
    Ffile.lStructSize = sizeof(Ffile);
    Ffile.lpstrFile = hFileIn;
    Ffile.lpstrFile[0] = '\0';
    Ffile.hwndOwner = hWnd;
    Ffile.nMaxFile = sizeof(hFileIn);
    string Temdir = "C:txt Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    Ffile.nFilterIndex = 1;
    Ffile.lpstrInitialDir = NULL;
    Ffile.lpstrFileTitle = NULL;
    Ffile.lpstrDefExt = TEXT("txt");
    Ffile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&Ffile))
    {
        GetWindowText(hInput, Ffile.lpstrFile, sizeof(hFileIn));
        SetWindowText(hInput, Ffile.lpstrFile);
    }
}

int PipeClientConnect(wchar_t* argv)
{
    HANDLE hPipe;
 
    char Error[312]= {0};
    BOOL fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;

    while (1)
    {
        hPipe = CreateFile(
            PipeName,      // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE,
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file

        // Break if the pipe handle is valid. 

        if (hPipe != INVALID_HANDLE_VALUE) 
        {
            sprintf(Error, "Create Pipe Successed. GLE= %d", GetLastError());
            MessageBoxA(NULL, Error, "Successed Open Pipe", MB_OK);
            break;
        }

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            sprintf(Error, "ERROR_PIPE_BUSY. GLE= %d", GetLastError());
            MessageBoxA(NULL, Error, "ERROR_PIPE_BUSY", MB_OK);
            return -1;
        }

        // All pipe instances are busy, so wait for 20 seconds. 

        if (!WaitNamedPipe(PipeName, 20000))
        {
            //printf("Could not open pipe: 20 second wait timed out.");
            MessageBoxA(NULL, "Could not open pipe: 20 second wait timed out.","Wait To Open Pipe",MB_OK);
            return -1;
        }
    }
    // The pipe connected; change to message-read mode. 

    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(
        hPipe,                              // pipe handle 
        &dwMode,                            // new pipe mode 
        NULL,                               // don't set maximum bytes 
        NULL);                              // don't set maximum time 
    if (!fSuccess)
    {
        sprintf(Error,"SetNamedPipeHandleState failed. GLE= %d", GetLastError());
        MessageBoxA(NULL, Error, "FAILED SetNamedPipe", MB_OK);
        return -1;
    }

    // Send a message to the pipe server. 
    cbToWrite = (lstrlenW(argv)+1) * sizeof(TCHAR);
    while (1)
    {
        fSuccess = WriteFile(
            hPipe,                             // pipe handle 
            argv,                              // message 
            cbToWrite,                         // message length 
            &cbWritten,                        // bytes written 
            NULL);                             // not overlapped 

        if (!fSuccess)
        {
            sprintf(Error, "WriteFile to pipe failed. GLE= %d\n", GetLastError());
            MessageBoxA(NULL, Error, "FAILED WriteFile", MB_OK);
            return -1;
        }
        break;
    }
    //char chBuf[BUFFERSIZE] = { 0 };
    //do
    //{
    //    // Read from the pipe. 

    //    fSuccess = ReadFile(
    //        hPipe,    // pipe handle 
    //        chBuf,    // buffer to receive reply 
    //        BUFFERSIZE * sizeof(char),  // size of buffer 
    //        &cbRead,  // number of bytes read 
    //        NULL);    // not overlapped 

    //    if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
    //        break;

    //    MessageBoxA(0, chBuf, "Tittle", MB_OK);

    //} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA

    CloseHandle(hPipe);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
    {
        if (MessageBox(hWnd, L"Thoát?", L"Dừng Service?", MB_YESNO) == IDYES)
            DestroyWindow(hWnd);
        break;
    }
    case WM_COMMAND:
        {
            switch (wParam)
            {
                
            case OPEN_BUTTON:
            {
                OpenFileDlg(hWnd);
                break;
            }
            case CREATE_BUTTON:
            {
                CreateFileDlg(hWnd);
                break;
            }
            case DELETE_BUTTON:
            {
                DeleteFileDlg(hWnd);
                break;
            }

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }   
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
