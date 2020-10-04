
#ifndef PINSWIPE_HEADER
#define PINSWIPE_HEADER

#ifdef PINSWIPE_EXPORTS
#define PINSWIPE_API __declspec(dllexport)
#else
#define PINSWIPE_API __declspec(dllimport)
#endif

#include <winscard.h>
#include <wincred.h>

DWORD WINAPI SCardTransmit_Hooked(SCARDHANDLE hCard, LPCSCARD_IO_REQUEST pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength, LPSCARD_IO_REQUEST pioRecvPci, 
	LPBYTE pbRecvBuffer, LPDWORD pcbRecvLength);

typedef DWORD (WINAPI *pSCardTransmit)(SCARDHANDLE hCard, LPCSCARD_IO_REQUEST pioSendPci, LPCBYTE pbSendBuffer, DWORD cbSendLength,
	LPSCARD_IO_REQUEST  pioRecvPci, LPBYTE pbRecvBuffer,LPDWORD pcbRecvLength);

extern pSCardTransmit pOriginalpSCardTransmit;

#endif
