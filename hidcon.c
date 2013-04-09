// Executes from command line hiding the console window
#define UNICODE

#include <windows.h>

#pragma comment(linker,"/ENTRY:_Go_")

#pragma comment(linker,"/SUBSYSTEM:Windows")

#pragma comment(linker,"/DEFAULTLIB:SHELL32.lib")
#pragma comment(linker,"/DEFAULTLIB:USER32.lib")
#pragma comment(linker,"/DEFAULTLIB:KERNEL32.lib")

void _Go_()
{
    LPWSTR* Argv;
    LPWSTR Args;
    int Argc, i;

    Argv = CommandLineToArgvW(GetCommandLineW(), &Argc);

    if (Argc < 2)
        goto Exit;

    Args = LocalAlloc(LMEM_FIXED|LMEM_ZEROINIT, Argc + 2*lstrlen(GetCommandLineW()));
    
    if (Argc > 2)
    {
        for (i=2; i < Argc; i++)
        {
             lstrcat(Args, Argv[i]);
             lstrcat(Args, L" ");
        }
    }
    //MessageBox(0, Args, L"Debug", MB_OK);
    ShellExecute(0, L"open", Argv[1], Args, 0, 0);
Exit:
    ExitProcess(0);
}
