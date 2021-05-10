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

DWORD WINAPI leCmd(LPVOID param) {
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

		//Sleep(1000);

		move(dados->self.x, dados->self.y, dados->self.destino.x, dados->self.destino.y, &dados->self.x, &dados->self.y);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		// esperar sem�foro vazio e o mutex
		WaitForSingleObject(dados->sem_vazios, INFINITE);
		WaitForSingleObject(dados->mutex, INFINITE);
		
		// copiar o avi�o
		CopyMemory(&dados->ptr_memoria->avioes[dados->ptr_memoria->saiAviao], &dados->self, sizeof(Aviao));
		dados->ptr_memoria->saiAviao++; 

		// reset buffer
		if (dados->ptr_memoria->saiAviao == dados->ptr_memoria->maxavioes) {
			dados->ptr_memoria->saiAviao = 0;
		}

		// assinalar mutex
		ReleaseMutex(dados->mutex);

		// assinala sem�foro
		ReleaseSemaphore(dados->sem_itens, 1, NULL);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		if ((dados->self.x == dados->self.destino.x) && (dados->self.y == dados->self.destino.y)) {
			break;
		}

		_tprintf(TEXT("Aviao na posicao %d %d\n"), dados->self.x, dados->self.y);
	}
	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {
	HKEY chaveAeroportos, chaveLocal;
	HANDLE objMap, sem_control;
	DWORD result, cbdata = sizeof(int);
	TDados dados;
	TCHAR cmd[BUFFER];
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

	// termina se n�o tiver argumentos suficientes
	if (argc < 2) {
		_tprintf(TEXT("N�mero incorreto de argumentos, inicie novamente.\n"));
		return -1;
	}

	// verifica se existe algum controlador ativo
	sem_control = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
	result = GetLastError();
	if (result != ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("N�o existe nenhum controlador ativo.\n"));
		return -1;
	}

	// marca o aeroporto de origem
	TCHAR aeroportoLocal[BUFFER];
	_tcscpy_s(aeroportoLocal, BUFFER, argv[1]);

	// inicializar a memoria partilhada
	objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMap == NULL) {
		_tprintf(TEXT("Imposs�vel criar objecto de mapping.\n"));
		return;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_memoria == NULL) {
		_tprintf(TEXT("Imposs�vel criar vista de mem�ria partilhada.\n"));
		return;
	}

	// inicializa sem�foro para controlar inst�ncias de avi�es
	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);

	// inicializa os sem�foros, mutex para o modelo produtor - consumidor
	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);

	// abrir chave dos aeroportos
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveAeroportos, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Nenhum controlador aberto.\n"));
		return -1;
	}

	// abrir chave do aeroporto inicial, caso n�o exista encerrar
	RegCreateKeyEx(chaveAeroportos, aeroportoLocal, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveLocal, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("N�o existe nenhum aeroporto com esse nome.\n"));
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

	// esperar pelo sem�foro das inst�ncias do avi�o
	WaitForSingleObject(dados.sem_avioes, INFINITE);
	
	
	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

	// esperar pelo sem�foro das posi��es vazias e do mutex
	WaitForSingleObject(dados.sem_vazios, INFINITE);
	WaitForSingleObject(dados.mutex, INFINITE);

	// incrementar n�mero de avi�es
	dados.ptr_memoria->navioes++;

	// copiar para a mem�ria o avi�o criado
	CopyMemory(&dados.ptr_memoria->avioes[dados.ptr_memoria->saiAviao], &dados.self, sizeof(Aviao));
	dados.ptr_memoria->saiAviao++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte

	// reset do buffer
	if (dados.ptr_memoria->saiAviao == dados.ptr_memoria->maxavioes)
		dados.ptr_memoria->saiAviao = 0;

	// assinalar mutex
	ReleaseMutex(dados.mutex);

	// assinalar sem�foro dos itens
	ReleaseSemaphore(dados.sem_itens, 1, NULL);

	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

	// DEBUG
	_tprintf(TEXT("Aviao %d\n"), dados.self.id);

	// comandos

	//do {
	//	_tprintf(TEXT("Introduza o comando que pretende.\n"));
	//	_fgetts(cmd, BUFFER, stdin);
	//} while(_tcsicmp(cmd, TEXT("fim\n")) != 0);


	// Aeroporto de destino
	TCHAR aeroportoDestino[BUFFER] = TEXT("Porto");
	HKEY chaveDestino;
	RegCreateKeyEx(chaveAeroportos, aeroportoDestino, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveDestino, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("N�o existe nenhum aeroporto com esse nome.\n"));
		return -1;
	}

	result = RegQueryValueEx(chaveDestino, TEXT("x"), NULL, NULL, (LPBYTE)&dados.self.destino.x, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de x.\n"));
		return -1;
	}
	result = RegQueryValueEx(chaveDestino, TEXT("y"), NULL, NULL, (LPBYTE)&dados.self.destino.y, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de y.\n"));
		return -1;
	}

	// lan�a thread para deslocar o aviao e para terminar o programa
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, DeslocaAviao, &dados, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, leCmd, NULL, 0, NULL);
	result = WaitForMultipleObjects(2, hThread, FALSE, INFINITE);

	// retira o avi�o
	WaitForSingleObject(dados.mutex, INFINITE);
	dados.ptr_memoria->navioes--;
	ReleaseMutex(dados.mutex);

	// DEBUG
	_tprintf(TEXT("%d\n"), dados.ptr_memoria->navioes);
	_tprintf(TEXT("Aviao %d, %d, %d.\n"), dados.self.id, dados.self.x, dados.self.y);

	// assinala sem�foro quando termina para dar lugar a outro avi�o
	ReleaseSemaphore(dados.sem_avioes, 1, &dados.ptr_memoria->navioes);
	Sleep(5000);
	_tprintf(TEXT("FIM\n"));
}