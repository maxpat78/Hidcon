// Execute console apps (with ShellExecute "open") hiding the console window
#define UNICODE

#include <windows.h>

#pragma comment(linker,"/ENTRY:_Go_") // custom entry point

#pragma comment(linker,"/SUBSYSTEM:Windows")

#pragma comment(linker,"/DEFAULTLIB:SHELL32.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")

void _Go_() {
    LPWSTR* Argv;
    LPWSTR Args;
    int Argc, i;
    //wchar_t s[100];

    Argv = CommandLineToArgvW(GetCommandLineW(), &Argc);

    if (Argc < 2) goto Exit;

    Args = LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, Argc + 2*lstrlen(GetCommandLineW()));
    
    if (Argc > 1) {
        for (i=1; i < Argc; i++) {
             lstrcat(Args, Argv[i]);
             lstrcat(Args, L" ");
        }
    }
    //MessageBox(0, Args, L"Debug", MB_OK);
    i = ShellExecute(0, L"open", Argv[1], Args, 0, 0);
    if (i < 33)
        MessageBox(0, L"Hidcon failed in launching hidden console process!", L"Hidcon", MB_OK);
    //wsprintf(s, L"ERR=0x%08X", i);
    //MessageBox(0, s, L"Debug", MB_OK);
Exit:
    ExitProcess(i);
}
