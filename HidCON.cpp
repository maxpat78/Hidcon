// Execute console apps (with CreateProcess) hiding the console window and optionally redirecting STDOUT/STDERR
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdlib.h>

#pragma comment(linker,"/ENTRY:_Go_") // custom entry point
#pragma comment(linker,"/SUBSYSTEM:Windows")
#pragma comment(linker,"/DEFAULTLIB:SHELL32.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")

LPWSTR hidcon = L"HidCON";

void _Go_ (void) {
    int Argc = 0;
    LPWSTR *Argv = CommandLineToArgvW(GetCommandLineW(), &Argc);
    if (!Argv || Argc < 2) ExitProcess(0);

    int i = 1;
    HANDLE hFile = NULL;
    BOOL append = 0;
    LPWSTR outfile = 0;

    if (! lstrcmpW(Argv[i], L"-a")) {
        i++;
        append = 1;
    }
    
    if (! lstrcmpW(Argv[i], L"-o")) {
        i++;
        if (i < Argc)
            outfile = Argv[i++];
        else {
            MessageBoxW(NULL, L"Redirection file not specified", hidcon, MB_OK);
            ExitProcess(1);
        }
    }
    
    // Redirect STDOUT and STDERR to file
    if (outfile) {
        hFile = CreateFileW(
            outfile,
            FILE_APPEND_DATA | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            append ? OPEN_ALWAYS : CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBoxW(NULL, L"Couldn't open redirection file", hidcon, MB_OK);
            ExitProcess(1);
        }
        if (append) SetFilePointer(hFile, 0, 0, FILE_END);
        SetHandleInformation(hFile, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
    }

    if (i >= Argc) ExitProcess(0);
    
    LPCWSTR exe = Argv[i++];
    WCHAR FullExe[MAX_PATH];
    DWORD n = SearchPathW(NULL, exe, L".exe", MAX_PATH, FullExe, NULL);

    if (n == 0) {
        wchar_t msg[MAX_PATH+64];
        lstrcpyW(msg, L"Couldn't find program ");
        lstrcatW(msg, exe);
        MessageBoxW(NULL, msg, hidcon, MB_OK);
        ExitProcess(2);
    }

    WCHAR CmdLine[2048];

    lstrcpyW(CmdLine, FullExe);
    lstrcatW(CmdLine, L" ");

    for (; i < Argc; i++) {
        lstrcatW(CmdLine, Argv[i]);
        lstrcatW(CmdLine, L" ");
    }

    STARTUPINFOW si = {0};
    PROCESS_INFORMATION pi = {0};
    
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    if (hFile) {
        si.dwFlags |= STARTF_USESTDHANDLES;
        si.hStdOutput = hFile;
        si.hStdError  = hFile;
        si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
    }

    BOOL ok = CreateProcessW(
        FullExe,
        CmdLine,
        0, 0,
        hFile ? 1 : 0,
        CREATE_NO_WINDOW,
        0,
        0,
        &si,
        &pi
    );

    if (!ok) {
        MessageBoxW(NULL, L"Couldn't start hidden console process", hidcon, MB_OK);
        if (hFile) CloseHandle(hFile);
        ExitProcess(2);
    }

    // Wait for process end, like ShellExecute
    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD ec = 0;
    GetExitCodeProcess(pi.hProcess, &ec);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (hFile) CloseHandle(hFile);

    ExitProcess(ec);
}
