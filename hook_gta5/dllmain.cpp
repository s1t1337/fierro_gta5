
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
//#include <wininet.h>
#include <winhttp.h>
#include "detours.h"
#include "detver.h"
#include <psapi.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <handleapi.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment(lib,"detours.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib,"Winhttp.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "urlmon.lib")


HINTERNET(WINAPI* winhttp_connect_orig)(IN HINTERNET     hSession,
    IN LPCWSTR       pswzServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD         dwReserved) = WinHttpConnect;



HINTERNET(WINAPI* winhttp_open_request_orig)(IN HINTERNET hConnect,
    IN LPCWSTR   pwszVerb,
    IN LPCWSTR   pwszObjectName,
    IN LPCWSTR   pwszVersion,
    IN LPCWSTR   pwszReferrer,
    IN LPCWSTR* ppwszAcceptTypes,
    IN DWORD     dwFlags) = WinHttpOpenRequest;


HRESULT(WINAPI* url_download_file_a)(LPUNKNOWN            pCaller,
    LPCTSTR              szURL,
    LPCTSTR              szFileName,
    _Reserved_ DWORD                dwReserved,
    LPBINDSTATUSCALLBACK lpfnCB) = URLDownloadToFileA;




int cout_hooks = 0;

__declspec(dllexport) HRESULT WINAPI my_url_download_file_a(LPUNKNOWN            pCaller,
    LPCTSTR              szURL,
    LPCTSTR              szFileName,
    _Reserved_ DWORD                dwReserved,
    LPBINDSTATUSCALLBACK lpfnCB)
{


    if (cout_hooks == 0)
        szURL = "https://enigma.thelegion.info/enigma_resource/Translation/Chinese.json";
    else if(cout_hooks == 1)
        szURL = "https://enigma.thelegion.info/enigma_resource/Graphics/graphics.ytd";

    cout_hooks++;

    return url_download_file_a(pCaller, szURL, szFileName, dwReserved, lpfnCB);
}


__declspec(dllexport) HINTERNET WINAPI my_http_connect(IN HINTERNET     hSession,
    IN LPCWSTR       pswzServerName,
    IN INTERNET_PORT nServerPort,
    IN DWORD         dwReserved)
{

        pswzServerName = L"enigma.thelegion.info";


    return winhttp_connect_orig(hSession, pswzServerName, nServerPort, dwReserved);
}

int count_https = 0;

__declspec(dllexport) HINTERNET WINAPI my_http_open_request(IN HINTERNET hConnect,
    IN LPCWSTR   pwszVerb,
    IN LPCWSTR   pwszObjectName,
    IN LPCWSTR   pwszVersion,
    IN LPCWSTR   pwszReferrer,
    IN LPCWSTR* ppwszAcceptTypes,
    IN DWORD     dwFlags)
{


    if (count_https == 0)
        pwszObjectName = L"/enigma_resource/web/hash.php";
    else if(count_https == 1)
        pwszObjectName = L"/enigma_resource/web/time.php";
    else if (count_https == 2)
        pwszObjectName = L"/enigma_resource/web/vehicle_list.ini";

    count_https++;

    return winhttp_open_request_orig(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);
}



BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:

                DetourRestoreAfterWith();
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());
                DetourAttach(&(PVOID&)winhttp_connect_orig, my_http_connect);
                DetourAttach(&(PVOID&)winhttp_open_request_orig, my_http_open_request);
                DetourAttach(&(PVOID&)url_download_file_a, my_url_download_file_a);
                DetourTransactionCommit();

                break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

