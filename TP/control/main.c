#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE semaforo_execucao;
	HKEY chave = NULL;
	DWORD result = 0, cbdata = 0;
	DWORD MAXAE = 0, MAXAV = 0;
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

	semaforo_execucao = CreateSemaphore(NULL, 0, 1, TEXT("Semáforo Execução"));
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("Já existe um controlador em execução.\nPor favor termine-o para iniciar um novo.\n"));
		return -1;
	}

	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\temp\\SO2\\TP"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chave);
	result = GetLastError();
	_tprintf(TEXT("%ld\n"), result);
	if (chave == NULL) {
		MAXAE = 3;
		MAXAV = 7;
	}
	else {
		//_tprintf(TEXT("NAO ABRI A CHAVE!!\n"));
	}

	RegQueryValueEx(chave, TEXT("MAXAE"), NULL, NULL, (LPBYTE)&MAXAE, (LPDWORD)&cbdata);
	RegQueryValueEx(chave, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&MAXAV, (LPDWORD)&cbdata);

	result = GetLastError();
	_tprintf(TEXT("%ld\n"), ERROR_SUCCESS);
	_tprintf(TEXT("%ld\n"), result);
	_tprintf(TEXT("NUMERO MAXIMO DE AEROPORTOS: %ld\n"), MAXAE);
	_tprintf(TEXT("NUMERO MAXIMO DE AVIOES: %ld\n"), MAXAV);


	return 0;
	
}