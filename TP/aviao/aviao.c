#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include "../util.h"
#include "SO2_TP_DLL_2021.h"

#define BUFFER 200

DWORD WINAPI termina(LPVOID param) {
	TCHAR debug[100];
	while (1) {
		_getts_s(debug, 100);
		break;
	}
	return 0;
}

DWORD WINAPI DeslocaAviao(LPVOID param) {
	TDados* dados = (TDados*)param;

	while (!dados->ptr_memoria->terminar) {

		Sleep(1000);

		move(dados->self.x, dados->self.y, 50, 50, &dados->self.x, &dados->self.y);
		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		// esperar semáforo vazio e o mutex
		WaitForSingleObject(dados->sem_vazios, INFINITE);
		WaitForSingleObject(dados->mutex, INFINITE);
		
		// copiar o avião
		CopyMemory(&dados->ptr_memoria->avioes[dados->ptr_memoria->saiAviao], &dados->self, sizeof(Aviao));
		dados->ptr_memoria->saiAviao++; 

		// reset buffer
		if (dados->ptr_memoria->saiAviao == dados->ptr_memoria->maxavioes) {
			dados->ptr_memoria->saiAviao = 0;
		}

		// assinalar mutex
		ReleaseMutex(dados->mutex);

		// assinala semáforo
		ReleaseSemaphore(dados->sem_itens, 1, NULL);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		_tprintf(TEXT("Aviao na posicao %d %d\n"), dados->self.x, dados->self.y);
	}
	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {
	HKEY chaveAeroportos, chaveLocal;
	HANDLE objMap, sem_control;
	DWORD result, cbdata = sizeof(int);
	TDados dados;
	HMODULE hDLL;
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
	int x = 0; int y = 0;
	// termina se não tiver argumentos suficientes
	if (argc < 2) {
		_tprintf(TEXT("Número incorreto de argumentos, inicie novamente.\n"));
		return -1;
	}

	// verifica se existe algum controlador ativo
	sem_control = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
	result = GetLastError();
	if (result != ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("Não existe nenhum controlador ativo.\n"));
		return -1;
	}

	// marca o aeroporto de origem
	TCHAR aeroportoLocal[BUFFER];
	_tcscpy_s(aeroportoLocal, BUFFER, argv[1]);

	// inicializar a memoria partilhada
	objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMap == NULL) {
		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
		return;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_memoria == NULL) {
		_tprintf(TEXT("Impossível criar vista de memória partilhada.\n"));
		return;
	}

	// inicializa semáforo para controlar instâncias de aviões
	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);

	// inicializa os semáforos, mutex para o modelo produtor - consumidor
	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);

	// abrir chave dos aeroportos
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveAeroportos, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Nenhum controlador aberto.\n"));
		return -1;
	}

	// abrir chave do aeroporto inicial, caso não exista encerrar
	RegCreateKeyEx(chaveAeroportos, aeroportoLocal, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveLocal, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Não existe nenhum aeroporto com esse nome.\n"));
		return -1;
	}

	// obter valor de x e y
	result = RegQueryValueEx(chaveLocal, TEXT("x"), NULL, NULL, (LPBYTE)&dados.self.x, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de x.\n"));
		return -1;
	}
	result = RegQueryValueEx(chaveLocal, TEXT("y"), NULL, NULL, (LPBYTE)&dados.self.y, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de y.\n"));
		return -1;
	}
	
	// obter o ID do processo
	dados.self.id = GetCurrentProcessId();

	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

	// esperar pelo semáforo das posições vazias e do mutex
	WaitForSingleObject(dados.sem_avioes, INFINITE);
	WaitForSingleObject(dados.sem_vazios, INFINITE);
	WaitForSingleObject(dados.mutex, INFINITE);

	// incrementar número de aviões
	dados.ptr_memoria->navioes += 1;

	// copiar para a memória o avião criado
	CopyMemory(&dados.ptr_memoria->avioes[dados.ptr_memoria->saiAviao], &dados.self, sizeof(Aviao));
	dados.ptr_memoria->saiAviao++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte

	// reset do buffer
	if (dados.ptr_memoria->saiAviao == dados.ptr_memoria->maxavioes)
		dados.ptr_memoria->saiAviao = 0;

	// assinalar mutex
	ReleaseMutex(dados.mutex);

	// assinalar semáforo dos itens
	ReleaseSemaphore(dados.sem_itens, 1, NULL);

	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

	_tprintf(TEXT("Aviao %d\n"), dados.self.id);

	// lança thread para deslocar o aviao e para terminar o programa
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, DeslocaAviao, &dados, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, termina, NULL, 0, NULL);
	result = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	// retira o avião
	dados.ptr_memoria->navioes -= 1;
	// assinala semáforo quando termina para dar lugar a outro avião
	ReleaseSemaphore(dados.sem_avioes, 1, &dados.ptr_memoria->navioes);
	_tprintf(TEXT("FIM\n"));
}