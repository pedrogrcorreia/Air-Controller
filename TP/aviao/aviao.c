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
	HKEY chaveAeroportos;
	HANDLE objMap;
	Memoria* ptr_memoria;
	DWORD result, cbdata = sizeof(int);
	int x=0, y=0; // mapa

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

	if (argc < 2) {
		_tprintf(TEXT("Número incorreto de argumentos, inicie novamente.\n"));
		return -1;
	}

	TCHAR aeroportoLocal[BUFFER];
	_tcscpy_s(aeroportoLocal, BUFFER, argv[1]);

	objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMap == NULL) {
		_tprintf(TEXT("DEBUG\n"));
		return;
	}
	ptr_memoria = (Memoria*)MapViewOfFile(objMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (ptr_memoria == NULL) {
		_tprintf(TEXT("DEBUG\n"));
		return;
	}

	//Aeroporto novo;

	// Abrir chave
	
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveAeroportos, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Nenhum controlador aberto.\n"));
		return -1;
	}

	HKEY chave;
	RegCreateKeyEx(chaveAeroportos, aeroportoLocal, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chave, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Não existe nenhum aeroporto com esse nome.\n"));
		return -1;
	}

	result = RegQueryValueEx(chave, TEXT("x"), NULL, NULL, (LPBYTE)&x, (LPDWORD)&cbdata);
	//result = RegGetValue(chaveAeroportos, TEXT("Lisboa"), TEXT("x"), RRF_RT_REG_SZ, NULL, (PVOID)&x, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de x.\n"));
		return -1;
	}
	result = RegQueryValueEx(chave, TEXT("y"), NULL, NULL, (LPBYTE)&y, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de y.\n"));
		return -1;
	}
	_tprintf(TEXT("%d\n"), x);
	_tprintf(TEXT("%d\n"), y);
	_tprintf(TEXT("%d\n"), ptr_memoria->naeroportos);
	_tprintf(TEXT("%d\n"), ptr_memoria->navioes);
	//CopyMemory(&novo, &ptr_memoria->aeroportos[ptr_memoria->naeroportos], sizeof(Aeroporto));
	//_tprintf(TEXT("%d\n"), ptr_memoria->aeroportos->x);
	//_tprintf(TEXT("%d\n"), novo.x);
	/*for (int i = 0; i < 1; i++) {
		_tprintf(TEXT("Aeroporto %d: %s, localizado em %d, %d.\n"), i + 1, aps->nome, aps->x, aps->y);
	}*/
	_tprintf(TEXT("FIM\n"));

}