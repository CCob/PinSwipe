#include "pch.h"

#include "minhook/include/MinHook.h"
#include "PinSwipe.h"
#include <stdio.h>

static HMODULE hWinSCard = NULL;

DWORD WINAPI InitHooksThread(LPVOID param) {

    if (MH_Initialize() != MH_OK) {
        return -1;
    }

    if ( (hWinSCard = GetModuleHandle(TEXT("WinSCard.dll"))) == NULL) {
        //If WinSCard is not loaded we need to pre-load it 
        //so that we can hook SCardTransmit 
        LoadLibrary(TEXT("WinSCard.dll"));
    }

    MH_STATUS status = MH_CreateHookApi(TEXT("winscard"), "SCardTransmit", SCardTransmit_Hooked, reinterpret_cast<LPVOID*>(&pOriginalpSCardTransmit));

    if (status == MH_OK) {
        status = MH_EnableHook(MH_ALL_HOOKS);
    }
    else {
        OutputDebugString(TEXT("Unable to create hooks"));
    }

    return status;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        //We are not interested in callbacks when a thread is created
        DisableThreadLibraryCalls(hModule);

        //We need to create a thread when initialising our hooks since
        //DllMain is prone to lockups if executing code inline.
        HANDLE hThread = CreateThread(nullptr, 0, InitHooksThread, nullptr, 0, nullptr);
        if (hThread != nullptr) {
            CloseHandle(hThread);
        }
        break;
    }
    case DLL_PROCESS_DETACH:

        //Remove all our hooks
        MH_DisableHook(MH_ALL_HOOKS);

        //If we pre-loaded winscard we need to 
        //free our reference to it
        if (hWinSCard != NULL) {
            FreeLibrary(hWinSCard);
        }
        
        break;
    }
    return TRUE;
}
