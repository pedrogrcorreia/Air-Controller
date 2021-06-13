#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <io.h>
#include <fcntl.h>
#include "..\util.h"

#define BUFSIZE 2048

#define Ps_Sz sizeof(Passageiro)

void readTChars(TCHAR* p, int maxchars) {
	int len;
	_fgetts(p, maxchars, stdin);
	len = _tcslen(p);
	if (p[len - 1] == TEXT('\n'))
		p[len - 1] = TEXT('\0');
}

DWORD WINAPI ThreadPassageiroReader(LPVOID lpvParam) {
	Passageiro FromServer;
	DWORD cbBytesRead = 0;
	BOOL fSuccess = FALSE;
	Passageiro* eu = (Passageiro*)lpvParam;
	HANDLE hPipe = eu->hPipe;

	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };

	ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	while (!eu->termina) {
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		OverlRd.hEvent = ReadReady;
		ResetEvent(ReadReady);

		fSuccess = ReadFile(hPipe, &FromServer, Ps_Sz, &cbBytesRead, &OverlRd);

		WaitForSingleObject(ReadReady, INFINITE);

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);

		if (_tcsicmp(FromServer.mensagem, TEXT("Registo")) == 0) {
			_tprintf(TEXT("Passageiro registado no aeroporto %s com destino a %s\n"), FromServer.inicial, FromServer.destino);
		}
		else if(_tcsicmp(FromServer.mensagem, TEXT("Embarcar")) == 0) {
			_tprintf(TEXT("Passageiro no voo %d com origem em %s e destino a %s\n"), FromServer.voo, FromServer.inicial, FromServer.destino);
		}
		else if(_tcsicmp(FromServer.mensagem, TEXT("Viajar")) == 0) {
			_tprintf(TEXT("Passageiro em voo. Coordenadas %d, %d.\n"), FromServer.x, FromServer.y);
		}
		else if(_tcsicmp(FromServer.mensagem, TEXT("Chegada")) == 0) {
			_tprintf(TEXT("Passageiro chegou ao destino %s.\n"), FromServer.destino);
		}
		else if (_tcsicmp(FromServer.mensagem, TEXT("Termina")) == 0) {
			_tprintf(TEXT("Passageiro terminou.\n"));
		}
		else {
			_tprintf(TEXT("%s\n"), FromServer.mensagem);
		}
		
		if (FromServer.termina) {
			eu->termina = true;
			break;
		}

	}
	CloseHandle(ReadReady);
	return 1;
}


DWORD WINAPI ThreadPassageiroWrite(LPVOID lparam) {
	Passageiro* eu = (Passageiro*)lparam;
	BOOL fSuccess = FALSE;
	DWORD cbWritten, dwMode;
	HANDLE WriteReady;
	OVERLAPPED OverlWr = { 0 };
	HANDLE hPipe = eu->hPipe;
	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	bool firstRun = true;
	_tprintf(TEXT("Ligacao feita ao Controlador Aéreo.\n"));
	//eu->termina = false;
	TCHAR buf[BUFFER];
	while (!eu->termina) {
		if (!firstRun) {
			_tprintf(TEXT("Introduza fim para sair.\n"));
			readTChars(buf, BUFFER);
			if (_tcscmp(TEXT("fim"), buf) == 0) {
				eu->termina = true;
			}
		}

		ZeroMemory(&OverlWr, sizeof(OverlWr));
		ResetEvent(WriteReady);
		OverlWr.hEvent = WriteReady;

		fSuccess = WriteFile(hPipe, eu, Ps_Sz, &cbWritten, &OverlWr);

		WaitForSingleObject(WriteReady, INFINITE);
		_tprintf(TEXT("\nWrite concluido\n"));

		GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);

		if (eu->termina) {
			break;
		}
		firstRun = false;
	}

	CloseHandle(WriteReady);
	//pressEnter();
	return 0;

}

int _tmain(int argc, TCHAR* argv[]) {
	HANDLE hPipe;
	BOOL fSuccess = FALSE;
	DWORD cbWritten, dwMode;
	HANDLE hThread[2];
	DWORD dwThreadId = 0;

#ifdef UNICODE 
	if (_setmode(_fileno(stdin), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
	if (_setmode(_fileno(stdout), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
	if (_setmode(_fileno(stderr), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
#endif
	Passageiro eu;
	_tcscpy_s(eu.inicial, BUFFER, argv[1]);
	_tcscpy_s(eu.destino, BUFFER, argv[2]);
	_tcscpy_s(eu.nome, BUFFER, argv[3]);
	if (argc > 4) {
		eu.espera = _tstoi(argv[4]) * 1000;
	}
	eu.termina = false;
	eu.voo = -1;

	while (1) {
		hPipe = CreateFile(PIPE_CONTROL, GENERIC_READ | GENERIC_WRITE, 0 | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0 | FILE_FLAG_OVERLAPPED, NULL);


		if (hPipe != INVALID_HANDLE_VALUE) {
			eu.hPipe = hPipe;
			break;
		}
	}

	dwMode = PIPE_READMODE_MESSAGE;
	fSuccess = SetNamedPipeHandleState(hPipe, &dwMode, NULL, NULL);
	eu.hPipe = hPipe;
	hThread[0] = CreateThread(NULL, 0, ThreadPassageiroReader, (LPVOID)&eu, 0, 0);
	hThread[1] = CreateThread(NULL, 0, ThreadPassageiroWrite, (LPVOID)&eu, 0, 0);
	DWORD result;
	result = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	HANDLE WriteReady;
	OVERLAPPED OverlWr = { 0 };
	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	eu.termina = true;
	
	fSuccess = WriteFile(hPipe, &eu, Ps_Sz, &cbWritten, &OverlWr);

	//WaitForSingleObject(WriteReady, INFINITE);

	//GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);

	CloseHandle(WriteReady);
	CloseHandle(hPipe);
	
	return 0;
}

