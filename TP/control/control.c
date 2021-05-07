#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include "../util.h"

#define BUFFER 200

int _tmain(int argc, TCHAR* argv[]) {
	Aeroporto* aeroportos;
	Memoria* ptr_memoria;
	HANDLE semaforo_execucao, objMap;
	HKEY chaveMAX = NULL, chaveAeroportos = NULL;
	DWORD result = 0, cbdata = sizeof(DWORD);
	int maxae = 0, maxav = 0, numae = 0, numav = 0;
	TCHAR cmd[BUFFER] = TEXT("");

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

	// Verifica se já está alguma instância em execução

	semaforo_execucao = CreateSemaphore(NULL, 0, 1, TEXT("Semáforo Execução"));
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("Já existe um controlador em execução.\nPor favor termine-o para iniciar um novo.\n"));
		return -1;
	}

	// Verifica se a chave abre

	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\temp\\SO2\\TP"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chaveMAX);

	// Obter o numero maximo de aeroportos e avioes

	result = RegQueryValueEx(chaveMAX, TEXT("MAXAE"), NULL, NULL, (LPBYTE)&maxae, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Não foi possível ler do registo o número máximo de aeroportos.\nVai ser definido como 10.\n"));
		maxae = 10;
	}
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&maxav, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Não foi possível ler do registo o número máximo de aviões.\nVai ser definido como 20.\n"));
		maxav = 20;
	}

	// Debug tirar depois
	_tprintf(TEXT("NUMERO MAXIMO DE AEROPORTOS: %ld\n"), maxae);
	_tprintf(TEXT("NUMERO MAXIMO DE AVIOES: %ld\n"), maxav);

	// Registar ou abrir chave para registo de aeroportos
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveAeroportos, NULL);

	objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMap == NULL) {
		return -1;
	}
	ptr_memoria = (Memoria*)MapViewOfFile(objMap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	if (ptr_memoria == NULL) {
		return -1;
	}

	aeroportos = malloc(sizeof(Aeroporto) * maxae);
	memset(aeroportos, 0, (size_t)maxae * sizeof(Aeroporto));


	// Imprimir menu

	do {
		_tprintf(TEXT("Introduza a opção do comando que pretende executar: \n"));
		_tprintf(TEXT("1. Criar aeroporto\n2. Suspender/Ativar registo de aviões\n3. Listar tudo\n"));
		_tprintf(TEXT("Opção: "));
		_fgetts(cmd, BUFFER, stdin);
		int cmdOpt = _tstoi(cmd);
		switch (cmdOpt) {
			case 1:
				if (criaAeroporto(aeroportos, &ptr_memoria->naeroportos, maxae)) {
					RegistaAeroporto(aeroportos[ptr_memoria->naeroportos-1], chaveAeroportos);
				}
				break;
			case 3:
				for (int i = 0; i < ptr_memoria->naeroportos; i++) {
					_tprintf(TEXT("Aeroporto %d: %s, localizado em %d, %d.\n"), i+1, aeroportos[i].nome, aeroportos[i].x, aeroportos[i].y);
				}
				break;
		}

	} while (_tcsicmp(cmd, TEXT("fim\n")) != 0);

	//hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LeCmd, NULL, NULL, NULL);
	//if (hThread == NULL) {
	//	_tprintf(TEXT("Impossível lançar a thread para ler os comandos do utilizador.\n"));
	//}

	//WaitForSingleObject(hThread, INFINITE);



	return 0;
	
}