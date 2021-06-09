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

bool getAeroportoInicial(TDados* dados, HKEY chaveAeroportos) {
	HKEY chaveLocal;
	DWORD result, cbdata = sizeof(int);

	// abrir chave do aeroporto inicial, caso não exista encerrar
	RegCreateKeyEx(chaveAeroportos, dados->self.inicial.nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveLocal, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Não existe nenhum aeroporto com esse nome.\n"));
		RegDeleteKey(chaveAeroportos, dados->self.inicial.nome);
		return false;
	}

	// obter valor de x e y
	result = RegQueryValueEx(chaveLocal, TEXT("x"), NULL, NULL, (LPBYTE)&dados->self.x, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de x.\n"));
		return false;
	}
	dados->self.inicial.x = dados->self.x;
	result = RegQueryValueEx(chaveLocal, TEXT("y"), NULL, NULL, (LPBYTE)&dados->self.y, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de y.\n"));
		return false;
	}
	dados->self.inicial.y = dados->self.y;
	return true;
}

bool getAeroportoDestino(TDados** dados, HKEY chaveAeroportos) {
	HKEY chaveLocal;
	DWORD result, cbdata = sizeof(int);

	// abrir chave do aeroporto de destino, caso não exista encerrar
	RegCreateKeyEx((*dados)->chaveAeroportos, (*dados)->self.destino.nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &chaveLocal, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Não existe nenhum aeroporto com esse nome.\n"));
		RegDeleteKey(chaveAeroportos, (*dados)->self.destino.nome);
		return false;
	}

	// obter valor de x e y
	result = RegQueryValueEx(chaveLocal, TEXT("x"), NULL, NULL, (LPBYTE)&(*dados)->self.destino.x, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de x.\n"));
		return false;
	}
	result = RegQueryValueEx(chaveLocal, TEXT("y"), NULL, NULL, (LPBYTE)&(*dados)->self.destino.y, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("Impossivel ler o valor de y.\n"));
		return false;
	}
	return true;
}

DWORD WINAPI stop(LPVOID param) {
	TDados** dados = (TDados**)param;
	TCHAR buffer[BUFFER];
	while (!(*dados)->self.terminar) {
		_fgetts(buffer, BUFFER, stdin);
		if (_tcsicmp(TEXT("fim\n"), buffer) == 0) {
			(*dados)->self.terminar = true; // parar uma viagem
		}
	}
	return 0;
}

DWORD WINAPI DeslocaAviao(LPVOID param) {
	TDados** dados = (TDados**)param;

	while (!(*dados)->ptr_memoria->terminar && !(*dados)->self.terminar) {

		Sleep(1000 / (*dados)->self.velocidade);

		move((*dados)->self.x, (*dados)->self.y, (*dados)->self.destino.x, (*dados)->self.destino.y, &(*dados)->self.x, &(*dados)->self.y);

		// desviar avião -> não funciona sem a implementação abaixo
		for (int i = 0; i < (*dados)->ptr_memoria->navioes; i++) {
			if ((*dados)->self.x == (*dados)->ptr_memoria->avioes[i].x && (*dados)->self.y == (*dados)->ptr_memoria->avioes[i].y) {
				(*dados)->self.x--;
			}
		}

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

		/*
		// esperar semáforo vazio e o mutex
		WaitForSingleObject((*dados)->sem_vazios, INFINITE);
		WaitForSingleObject((*dados)->mutex, INFINITE);

		// copiar o avião
		CopyMemory(&(*dados)->ptr_memoria->avioes[(*dados)->ptr_memoria->saiAviao], &(*dados)->self, sizeof(Aviao));
		(*dados)->ptr_memoria->saiAviao++;

		// reset buffer
		if ((*dados)->ptr_memoria->saiAviao == (*dados)->ptr_memoria->maxavioes) {
			(*dados)->ptr_memoria->saiAviao = (*dados)->ptr_memoria->maxavioes;
		}

		// assinalar mutex
		ReleaseMutex((*dados)->mutex);

		// assinala semáforo
		ReleaseSemaphore((*dados)->sem_itens, 1, NULL);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR
		*/

		// imprimir a posicao atual do aviao
		_tprintf(TEXT("Aviao na posicao %d %d\n"), (*dados)->self.x, (*dados)->self.y);

		// parar quando chega ao aeroporto
		if ( (*dados)->self.x == (*dados)->self.destino.x && ( (*dados)->self.y == (*dados)->self.destino.y ) ) {
			(*dados)->self.inicial = (*dados)->self.destino;
			(*dados)->self.terminar = true;
			SetEvent((*dados)->self.eventos[1]);
			_tprintf(TEXT("O avião chegou ao aeroporto de destino.\n"));
			_tprintf(TEXT("Enter para continuar.\n")); // sair da thread stop
			break;
		}
	}
	return 0;
}

DWORD WINAPI leComandos(LPVOID param) {
	TCHAR cmd[BUFFER];
	TDados* dados = (TDados*)param;
	TCHAR* delim = TEXT(" ");
	TCHAR* token = NULL;
	TCHAR* nextToken = NULL;
	TCHAR* comando = NULL;
	DWORD result;
	bool destino = false; // controlo da existência de um destino definido

	do {
		_tprintf(TEXT("\nO avião encontra-se no aeroporto %s localizado em %d, %d.\n"), dados->self.inicial.nome, dados->self.inicial.x, dados->self.inicial.y);
		_tprintf(TEXT("\nIntroduza o comando que pretende:\n"));
		_fgetts(cmd, BUFFER, stdin);
		if (_tcsicmp(cmd, TEXT("\n")) != 0) { // se o comando nao for apenas enter
			cmd[_tcslen(cmd) - 1] = '\0'; // retirar \n
			token = _tcstok_s(cmd, delim, &nextToken); // obtem o comando
			comando = token;
		}
		else {
			continue;
		}
		if (_tcsicmp(comando, TEXT("proximo")) == 0) {
			if (!destino) {
				token = _tcstok_s(NULL, delim, &nextToken); // obtem o nome do aeroporto
				if (token != 0) { // se o token for != 0 copia para o aeroporto de destino
					_tcscpy_s(dados->self.destino.nome, BUFFER, token);
					if (getAeroportoDestino(&dados, dados->chaveAeroportos)) {
						destino = true; // impedir outro destino
						_tprintf(TEXT("\nAeroporto destino definido como %s localizado em %d, %d.\n"), dados->self.destino.nome, dados->self.destino.x, dados->self.destino.y);
					}
				}
				else {
					_tprintf(TEXT("\nIntroduza o nome do aeroporto.\n"));
				}
			}
			else {
				_tprintf(TEXT("\nJá introduziu o destino.\n"));
			}
		}
		if (_tcsicmp(comando, TEXT("inicia")) == 0) {
			if (destino) { // apenas inicia se já tiver sido dado um destino
				token = _tcstok_s(NULL, delim, &nextToken);
				if (token == 0) {
					dados->self.terminar = false; // reset da condição de paragem
					_tprintf(TEXT("\nIntroduza 'fim' para terminar a viagem.\n"));
					HANDLE hThread[2];
					hThread[0] = CreateThread(NULL, 0, DeslocaAviao, &dados, 0, NULL); // cria thread para movimentar os avioes
					hThread[1] = CreateThread(NULL, 0, stop, &dados, 0, NULL);  // cria thread para parar o programa
					if (hThread[1] == NULL) {
						return -1;
					}
					result = WaitForMultipleObjects(2, hThread, FALSE, INFINITE); // espera por ambos
					if (result == WAIT_OBJECT_0) {								// caso acabe a viagem espera pelo enter do utilizador
						WaitForSingleObject(hThread[1], INFINITE);
						_tprintf(TEXT("\nViagem concluída com sucesso.\n"));
					}
					if (result == WAIT_OBJECT_0 + 1) {							// caso contrário sai termina
						_tprintf(TEXT("\nViagem interrompida em voo, o avião despenhou-se.\n"));
						break;
					}
					destino = false;
				}
				else {
					_tprintf(TEXT("\nIntroduza apenas 'inicia' para iniciar a viagem.\n"));
				}
			}
			else {
				_tprintf(TEXT("\nNecessita de introduzir um destino primeiro.\n"));
			}
		}
	} while (_tcsicmp(cmd, TEXT("fim")) != 0);

	return 0;
}

DWORD WINAPI terminar(LPVOID param) {
	TDados* dados = (TDados*)param;
	while (!dados->ptr_memoria->terminar) {
		continue; // enquanto o controlador nao terminar
	}
	return 0;
}

<<<<<<< HEAD
DWORD WINAPI terminarAviao(LPVOID param) {
	TDados* dados = (TDados*)param;
	while (!dados->self.terminar) {
		dados->self = getAviao(dados->self, *dados);
		continue; // se o controlador terminar o aviao
	}
	return 0;
}

=======
>>>>>>> parent of bf23a58 (final console version)
DWORD WINAPI AlertaControl(LPVOID param) {
	TDados* dados = (TDados*)param;
	while (1) {
		SetEvent(dados->self.eventos[0]); // Assinalar evento para o controlador
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	HANDLE objMap, sem_control;
	DWORD result, cbdata = sizeof(int);
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

	// termina se não tiver argumentos suficientes
	if (argc < 3) {
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

	// obtem o id do aviao
	dados.self.id = GetCurrentProcessId();
	dados.self.terminar = false;

	// obtem os dados passados por argumento da linha de comando -> aeroporto inicial e velocidade 
	_tcscpy_s(dados.self.inicial.nome, BUFFER, argv[1]);
	dados.self.velocidade = _tstoi(argv[2]);

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

	// abrir chave dos aeroportos para verificar se existe algum controlador aberto
	RegCreateKeyEx(HKEY_CURRENT_USER, CHAVE_AEROPORTOS, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &dados.chaveAeroportos, &result);
	if (result == REG_CREATED_NEW_KEY) {
		_tprintf(TEXT("Nenhum controlador aberto.\n"));
		return -1;
	}


	// obter aeroporto inicial
	if (getAeroportoInicial(&dados, dados.chaveAeroportos)) {
		_tprintf(TEXT("Aeroporto inicial definido em %s localizado em %d, %d.\n"), dados.self.inicial.nome, dados.self.inicial.x, dados.self.inicial.y);
	}
	else {
		return -1;
	}


	TCHAR alerta[BUFFER];
	TCHAR chegada[BUFFER];

	// criar evento para alerta
	_stprintf_s(alerta, BUFFER, TEXT("alerta %d"), dados.self.id);
	dados.self.eventos[0] = CreateEvent(NULL, FALSE, FALSE, alerta);


	// criar evento para chegadas
	_stprintf_s(chegada, BUFFER, TEXT("chegada %d"), dados.self.id);
	dados.self.eventos[1] = CreateEvent(NULL, FALSE, FALSE, chegada);


	// criar thread para enviar alertas enquanto o programa nao terminar
	HANDLE hAlerta;
	hAlerta = CreateThread(NULL, 0, AlertaControl, &dados, 0, NULL);

	// esperar pelo semáforo das instâncias do avião
	WaitForSingleObject(dados.sem_avioes, INFINITE);


	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

	// esperar pelo semáforo das posições vazias e do mutex
	WaitForSingleObject(dados.sem_vazios, INFINITE);
	WaitForSingleObject(dados.mutex, INFINITE);

	// incrementar número de aviões
	dados.ptr_memoria->navioes++;

	// copiar para a memória o avião criado
	CopyMemory(&dados.ptr_memoria->avioes[dados.ptr_memoria->saiAviao], &dados.self, sizeof(Aviao));
	dados.ptr_memoria->saiAviao++; //incrementamos a posicao de escrita para o proximo produtor escrever na posicao seguinte

	// reset do buffer
	if (dados.ptr_memoria->saiAviao == dados.ptr_memoria->maxavioes)
		dados.ptr_memoria->saiAviao = dados.ptr_memoria->maxavioes;

	// assinalar mutex
	ReleaseMutex(dados.mutex);

	// assinalar semáforo dos itens
	ReleaseSemaphore(dados.sem_itens, 1, NULL);

	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR

	// DEBUG
	_tprintf(TEXT("Aviao id %d: %d, %d\n"), dados.self.id, dados.self.x, dados.self.y);

	// lança a thread para ler os comandos e a thread para terminar
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, leComandos, &dados, 0, NULL); 
	hThread[1] = CreateThread(NULL, 0, terminar, &dados, 0, NULL);
	result = WaitForMultipleObjects(2, hThread, FALSE, INFINITE); // apenas 1 necessita de acabar
	if (result == WAIT_OBJECT_0 + 1) {
		_tprintf(TEXT("O controlador terminou.\n"));
	}

	// retira o avião
	WaitForSingleObject(dados.mutex, INFINITE);
	dados.ptr_memoria->saiAviao--;
	dados.ptr_memoria->navioes--;
	ReleaseMutex(dados.mutex);

	// assinala semáforo quando termina para dar lugar a outro avião
	ReleaseSemaphore(dados.sem_avioes, 1, NULL);

	UnmapViewOfFile(dados.ptr_memoria);
	
	return 0;
}