#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include "../util.h"

#define BUFFER 200

DWORD WINAPI suspend(LPVOID param) {
	TDados* dados = (TDados*)param;
	for (int i = dados->ptr_memoria->navioes; i < dados->ptr_memoria->maxavioes; i++) {
		WaitForSingleObject(dados->sem_avioes, INFINITE);
	}
	return 0;
}

DWORD WINAPI RecebeAvioes(LPVOID param) {
	TDados* dados = (TDados*)param;
	Aviao aviao;

	while (!dados->ptr_memoria->terminar) {

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		// esperar sem�foro dos avi�es
		WaitForSingleObject(dados->sem_itens, INFINITE);

		// copiar o aviao recebido
		CopyMemory(&aviao, &dados->ptr_memoria->avioes[dados->ptr_memoria->entAviao], sizeof(Aviao));
		dados->ptr_memoria->entAviao++; 

		// reset do buffer
		if (dados->ptr_memoria->entAviao == dados->ptr_memoria->maxavioes) {
			dados->ptr_memoria->entAviao = 0;
		}

		// assinala sem�foro
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		//_tprintf(TEXT("Aviao %d na posicao %d, %d.\n"), aviao.id, aviao.x, aviao.y);
	}
	return 0;
}


int _tmain(int argc, TCHAR* argv[]) {
	Aeroporto* aeroportos;
	HANDLE semaforo_execucao, objMap, hThread;
	HKEY chaveMAX = NULL, chaveAeroportos = NULL;
	DWORD result = 0, cbdata = sizeof(DWORD);
	TCHAR cmd[BUFFER] = TEXT("");
	TDados dados;
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

	// verifica se j� est� alguma inst�ncia em execu��o
	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("J� existe um controlador em execu��o.\nPor favor termine-o para iniciar um novo.\n"));
		return -1;
	}

	// inicializar a mem�ria
	objMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMap == NULL) {
		return -1;
	}

	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	if (dados.ptr_memoria == NULL) {
		return -1;
	}

	// verifica se a chave abre
	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\temp\\SO2\\TP"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chaveMAX);

	// obter o numero maximo de aeroportos e avioes
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAE"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxaeroportos, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("N�o foi poss�vel ler do registo o n�mero m�ximo de aeroportos.\nVai ser definido como 10.\n"));
		dados.ptr_memoria->maxaeroportos = 10;
	}
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxavioes, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("N�o foi poss�vel ler do registo o n�mero m�ximo de avi�es.\nVai ser definido como 20.\n"));
		dados.ptr_memoria->maxavioes = 10;
	}

	// inicializa sem�foro para controlar inst�ncias de avi�es
	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);

	// inicializa os sem�foros, mutex para o modelo produtor - consumidor
	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);

	// inicializar a condi��o de paragem a null
	dados.ptr_memoria->terminar = false;
	dados.ptr_memoria->navioes = 0;
	dados.ptr_memoria->naeroportos = 0;

	// Debug tirar depois
	_tprintf(TEXT("NUMERO MAXIMO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->maxaeroportos);
	_tprintf(TEXT("NUMERO MAXIMO DE AVIOES: %ld\n"), dados.ptr_memoria->maxavioes);
	_tprintf(TEXT("NUMERO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->naeroportos);
	_tprintf(TEXT("NUMERO DE AVIOES: %ld\n"), dados.ptr_memoria->navioes);

	// registar ou abrir chave para registo de aeroportos
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveAeroportos, NULL);

	// inicializa array de aeroportos
	aeroportos = malloc(sizeof(Aeroporto) * dados.ptr_memoria->maxaeroportos);
	memset(aeroportos, 0, (size_t)dados.ptr_memoria->maxaeroportos * sizeof(Aeroporto));

	// lan�a thread para controlar a entrada de avi�es
	hThread = CreateThread(NULL, 0, RecebeAvioes, &dados, 0, NULL);

	// imprimir menu

	dados.suspend = false;

	do {
		_tprintf(TEXT("Introduza a op��o do comando que pretende executar: \n"));
		_tprintf(TEXT("1. Criar aeroporto\n2. Suspender/Ativar registo de avi�es\n3. Listar tudo\n"));
		_tprintf(TEXT("Op��o: "));
		_fgetts(cmd, BUFFER, stdin);
		int cmdOpt = _tstoi(cmd);
		switch (cmdOpt) {
		case 1:
			if (criaAeroporto(aeroportos, &dados.ptr_memoria->naeroportos, dados.ptr_memoria->maxavioes)) {
				RegistaAeroporto(aeroportos[dados.ptr_memoria->naeroportos - 1], chaveAeroportos);
			}
			break;
		case 2:
			HANDLE susThread;
			if (!dados.suspend) {
				susThread = CreateThread(NULL, 0, suspend, &dados, 0, NULL);
				dados.suspend = true;
				_tprintf(TEXT("Registo de avi�es suspensos.\n"));
			}
			else {
				int release = 0;
				if (dados.ptr_memoria->maxavioes - dados.ptr_memoria->navioes < 0) {
					release = 0;
				}
				else {
					release = dados.ptr_memoria->maxavioes - dados.ptr_memoria->navioes;
				}
				ReleaseSemaphore(dados.sem_avioes, release, NULL);
				dados.suspend = false;
				_tprintf(TEXT("Registo de avi�es ativo.\n"));
			}
			break;
		case 3:
			for (int i = 0; i < dados.ptr_memoria->naeroportos; i++) {
				_tprintf(TEXT("Aeroporto %d: %s, localizado em %d, %d.\n"), i + 1, aeroportos[i].nome, aeroportos[i].x, aeroportos[i].y);
			}
			break;
		case 4:
			_tprintf(TEXT("N avioes: %d\n"), dados.ptr_memoria->navioes);
		}
	} while (_tcsicmp(cmd, TEXT("fim\n")) != 0);

	// accionar condi��o de paragem
	dados.ptr_memoria->terminar = true;
	WaitForSingleObject(hThread, 0);

	// apagar as chaves dos Aeroportos antes de encerrar.
	RegDeleteTree(chaveAeroportos, NULL);
	result = RegDeleteKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, KEY_WOW64_64KEY, 0);
	if (result == ERROR_SUCCESS) {
		_tprintf(TEXT("Apaguei a chave dos aeroportos.\n"));
	}

	return 0;
	
}