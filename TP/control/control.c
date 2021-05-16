#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include "../util.h"

#define BUFFER 200

void listaAeroportos(Aeroporto aeroportos[], int nae) {
	for (int i = 0; i < nae; i++) {
		_tprintf(TEXT("\nAeroporto %s, localizado em %d, %d.\n"), aeroportos[i].nome, aeroportos[i].x, aeroportos[i].y);
	}
}

void listaAvioes(Aviao avioes[], int nav) {
	for (int i = 0; i < nav; i++) {
		_tprintf(TEXT("\t %d"), nav);
		_tprintf(TEXT("\nAvião %d na posição %d, %d.\n"), avioes[i].id, avioes[i].x, avioes[i].y);
	}
}

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

		// esperar semáforo dos aviões
		WaitForSingleObject(dados->sem_itens, INFINITE);
		// copiar o aviao recebido
		CopyMemory(&aviao, &dados->ptr_memoria->avioes[dados->ptr_memoria->entAviao], sizeof(Aviao));
		dados->ptr_memoria->entAviao++; 
		 
		// reset do buffer não é necessário
		if (dados->ptr_memoria->entAviao == dados->ptr_memoria->maxavioes) {
			dados->ptr_memoria->entAviao = dados->ptr_memoria->maxavioes;
		}

		// assinala semáforo
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		//_tprintf(TEXT("Aviao %d na posicao %d, %d.\n"), aviao.id, aviao.x, aviao.y);
	}
	return 0;
}

DWORD WINAPI RecebeAlerta(LPVOID param) {
	TDados* dados = (TDados*)param;
	DWORD result;
	TCHAR alerta[BUFFER];
	while (!dados->ptr_memoria->terminar) {
		if (dados->ptr_memoria->entAviao > 0) {
			for (int i = 0; i < dados->ptr_memoria->entAviao; i++) {
				_stprintf_s(alerta, BUFFER, TEXT("alerta %d"), dados->ptr_memoria->avioes[i].id); // criar evento com o id do aviao
				dados->ptr_memoria->avioes[i].eventos[0] = OpenEvent(EVENT_ALL_ACCESS, TRUE, alerta); // abrir o evento
				if (dados->ptr_memoria->avioes[i].eventos[0] == NULL) {
					_tprintf(TEXT("Erro não abri o evento.\n"));
					_tprintf(TEXT("O controlador não vai funcionar normalmente, por favor reinicie.\n"));
					return -1;
				}
			}
			for (int i = 0; i < dados->ptr_memoria->entAviao; i++) {
				result = WaitForSingleObject(dados->ptr_memoria->avioes[i].eventos[0], 100); // esperar pelos eventos de cada avião
				if (result == WAIT_TIMEOUT) {
					_tprintf(TEXT("Avião %d desligou-se.\n"), dados->ptr_memoria->avioes[i].id); // se não chegar é porque o avião se desligou
					WaitForSingleObject(dados->mutex, INFINITE);
					dados->ptr_memoria->avioes[i] = dados->ptr_memoria->avioes[i+1]; // retira o avião
					dados->ptr_memoria->entAviao--;
					ReleaseMutex(dados->mutex);
				}
			}
		}
		Sleep(3000);
	}
	return 0;
}

DWORD WINAPI RecebeChegada(LPVOID param) {
	TDados* dados = (TDados*)param;
	DWORD result;
	TCHAR chegada[BUFFER];
	while (!dados->ptr_memoria->terminar) {
		if (dados->ptr_memoria->entAviao > 0) {
			for (int i = 0; i < dados->ptr_memoria->entAviao; i++) {
				_stprintf_s(chegada, BUFFER, TEXT("chegada %d"), dados->ptr_memoria->avioes[i].id); // criar evento com o id do avião
				dados->ptr_memoria->avioes[i].eventos[1] = OpenEvent(EVENT_ALL_ACCESS, TRUE, chegada); // abrir o evento
				if (dados->ptr_memoria->avioes[i].eventos[1] == NULL) {
					_tprintf(TEXT("Erro não abri o evento.\n"));
					_tprintf(TEXT("O controlador não vai funcionar normalmente, por favor reinicie.\n"));
					return -1;
				}
			}
			for (int i = 0; i < dados->ptr_memoria->entAviao; i++) {
				result = WaitForSingleObject(dados->ptr_memoria->avioes[i].eventos[1], 100); // esperar pelas chegadas de cada avião
				if (result == WAIT_OBJECT_0) {
					_tprintf(TEXT("Avião %d chegou ao destino.\n"), dados->ptr_memoria->avioes[i].id); // mostrar a mensagem
				}
			}
		}
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

	// verifica se já está alguma instância em execução
	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("Já existe um controlador em execução.\nPor favor termine-o para iniciar um novo.\n"));
		return -1;
	}

	// inicializar a memória
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
		_tprintf(TEXT("\nNão foi possível ler do registo o número máximo de aeroportos.\nVai ser definido como 10.\n"));
		dados.ptr_memoria->maxaeroportos = 10;
	}
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxavioes, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("\nNão foi possível ler do registo o número máximo de aviões.\nVai ser definido como 20.\n"));
		dados.ptr_memoria->maxavioes = 10;
	}

	// inicializa semáforo para controlar instâncias de aviões
	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);

	// inicializa os semáforos, mutex para o modelo produtor - consumidor
	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);

	// inicializar a condição de paragem a null
	dados.ptr_memoria->terminar = false;
	dados.ptr_memoria->navioes = 0;
	dados.ptr_memoria->naeroportos = 0;

	// DEBUG
	//_tprintf(TEXT("NUMERO MAXIMO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->maxaeroportos);
	//_tprintf(TEXT("NUMERO MAXIMO DE AVIOES: %ld\n"), dados.ptr_memoria->maxavioes);
	//_tprintf(TEXT("NUMERO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->naeroportos);
	//_tprintf(TEXT("NUMERO DE AVIOES: %ld\n"), dados.ptr_memoria->navioes);

	// registar ou abrir chave para registo de aeroportos
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveAeroportos, NULL);

	// inicializa array de aeroportos
	aeroportos = malloc(sizeof(Aeroporto) * dados.ptr_memoria->maxaeroportos);
	memset(aeroportos, 0, (size_t)dados.ptr_memoria->maxaeroportos * sizeof(Aeroporto));

	// lança thread para controlar a entrada de aviões
	hThread = CreateThread(NULL, 0, RecebeAvioes, &dados, 0, NULL);

	// inicializa a thread para receber alertas
	HANDLE hAlerta;
	hAlerta = CreateThread(NULL, 0, RecebeAlerta, &dados, 0, NULL);

	// inicializa a thread para receber alertas de chegadas
	HANDLE hChegada;
	hChegada = CreateThread(NULL, 0, RecebeChegada, &dados, 0, NULL);

	// inicializa suspensao de dados como falso
	dados.suspend = false;

	// imprimir menu
	do {
		_tprintf(TEXT("\nIntroduza a opção do comando que pretende executar: \n"));
		_tprintf(TEXT("1. Criar aeroporto\n2. Suspender/Ativar registo de aviões\n3. Listar tudo\n"));
		_tprintf(TEXT("\nOpção: "));
		_fgetts(cmd, BUFFER, stdin);
		if (_tcsicmp(cmd, TEXT("\n")) != 0) {
			cmd[_tcslen(cmd) - 1] = '\0'; // retirar \n
		}
		else {
			continue;
		}
		int cmdOpt = _tstoi(cmd);
		switch (cmdOpt) {
		case 1:
			if (criaAeroporto(aeroportos, &dados.ptr_memoria->naeroportos, dados.ptr_memoria->maxaeroportos)) {
				RegistaAeroporto(aeroportos[dados.ptr_memoria->naeroportos - 1], chaveAeroportos);
			}
			break;
		case 2:
			HANDLE susThread; 
			if (!dados.suspend) {
				susThread = CreateThread(NULL, 0, suspend, &dados, 0, NULL); // thread para ocupar todas as posições do semáforo
				dados.suspend = true;
				_tprintf(TEXT("\nRegisto de aviões suspensos.\n"));
			}
			else {
				int release = 0;
				if (dados.ptr_memoria->maxavioes - dados.ptr_memoria->navioes < 0) {
					release = 0; // se todas as posições tiverem ocupadas antes da suspensão 
				}
				else {
					release = dados.ptr_memoria->maxavioes - dados.ptr_memoria->navioes; // numero de posições a libertar
				}
				ReleaseSemaphore(dados.sem_avioes, release, NULL);
				dados.suspend = false;
				_tprintf(TEXT("\nRegisto de aviões ativo.\n"));
			}
			break;
		case 3:
			listaAeroportos(aeroportos, dados.ptr_memoria->naeroportos);
			listaAvioes(dados.ptr_memoria->avioes, dados.ptr_memoria->entAviao);
			break;
		}
	} while (_tcsicmp(cmd, TEXT("fim")) != 0);

	// accionar condição de paragem
	dados.ptr_memoria->terminar = true;
	WaitForSingleObject(hThread, 0);

	// apagar as chaves dos Aeroportos antes de encerrar.
	RegDeleteTree(chaveAeroportos, NULL);
	result = RegDeleteKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, KEY_WOW64_64KEY, 0);

	// DEBUG
	//if (result == ERROR_SUCCESS) {
	//	_tprintf(TEXT("Apaguei a chave dos aeroportos.\n"));
	//}

	UnmapViewOfFile(dados.ptr_memoria);

	return 0;
	
}