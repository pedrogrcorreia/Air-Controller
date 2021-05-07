#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include "util.h"

#define BUFFER 200

int _tmain(int argc, TCHAR* argv[]) {

	HANDLE objMap;
	Memoria* ptr_memoria;

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
	
	_tprintf(TEXT("%d\n"), ptr_memoria->naeroportos);
	_tprintf(TEXT("%d\n"), ptr_memoria->navioes);
	_tprintf(TEXT("%s\n"), ptr_memoria->aeroportos[0].nome);
	//CopyMemory(&novo, &ptr_memoria->aeroportos[ptr_memoria->naeroportos], sizeof(Aeroporto));
	//_tprintf(TEXT("%d\n"), ptr_memoria->aeroportos->x);
	//_tprintf(TEXT("%d\n"), novo.x);
	/*for (int i = 0; i < 1; i++) {
		_tprintf(TEXT("Aeroporto %d: %s, localizado em %d, %d.\n"), i + 1, aps->nome, aps->x, aps->y);
	}*/
	_tprintf(TEXT("FIM\n"));

}