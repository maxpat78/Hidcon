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

void _Go_ (void) {
    int Argc = 0;
    LPWSTR *Argv = CommandLineToArgvW(GetCommandLineW(), &Argc);
    if (!Argv || Argc < 2) ExitProcess(0);

    int i = 1;

    HANDLE hFile = NULL;

    // Redirect STDOUT and STDERR to file
    if (i + 2 <= Argc && lstrcmpiW(Argv[i], L"-o") == 0) {
        hFile = CreateFileW(
            Argv[i + 1],
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (hFile == INVALID_HANDLE_VALUE) {
            MessageBoxW(NULL, L"Couldn't open redirection file", L"HidCON", MB_OK);
            ExitProcess(1);
        }
        SetHandleInformation(hFile, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
        i += 2;
    }

    if (i >= Argc) ExitProcess(0);
    
    LPCWSTR exe = Argv[i++];
    WCHAR FullExe[MAX_PATH];
    DWORD n = SearchPathW(NULL, exe, L".exe", MAX_PATH, FullExe, NULL);

    if (n == 0) {
        MessageBoxW(NULL, L"Couldn't find executable!", L"HidCON", MB_OK);
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
        MessageBoxW(NULL, L"Could not start hidden console process!", L"HidCON", MB_OK);
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
