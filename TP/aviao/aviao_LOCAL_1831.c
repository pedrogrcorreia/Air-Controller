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

Aviao getAviao(Aviao a, TDados dados) {
	WaitForSingleObject(dados.mutex, INFINITE);
	for (int i = 0; i <= dados.ptr_memoria->navioes; i++) {
		if (a.id == dados.ptr_memoria->avioes[i].id) {
			ReleaseMutex(dados.mutex);
			return dados.ptr_memoria->avioes[i];
		}
	}
	ReleaseMutex(dados.mutex);
	return;
}

DWORD WINAPI stop(LPVOID param) {
	TDados** dados = (TDados**)param;
	TCHAR buffer[BUFFER];
	while (!(*dados)->self.terminarViagem) {
		_fgetts(buffer, BUFFER, stdin);
		if (_tcsicmp(TEXT("fim\n"), buffer) == 0) {
			(*dados)->self.terminarViagem = true; // parar uma viagem
		}
	}
	return 0;
}

DWORD WINAPI DeslocaAviao(LPVOID param) {
	TDados** dados = (TDados**)param;
	
	while (!(*dados)->ptr_memoria->terminar && !(*dados)->self.terminar) {
		(*dados)->self = getAviao((*dados)->self, **dados);
		(*dados)->self.setDestino = false;
		(*dados)->self.viajar = true;
		//SetEvent((*dados)->self.eventos[0]);
		Sleep(1000 / (*dados)->self.velocidade);

		move((*dados)->self.x, (*dados)->self.y, (*dados)->self.destino.x, (*dados)->self.destino.y, &(*dados)->self.x, &(*dados)->self.y);

		// desviar
		for (int i = 0; i < (*dados)->ptr_memoria->navioes; i++) {
			if ((*dados)->self.x == (*dados)->ptr_memoria->avioes[i].x && (*dados)->self.y == (*dados)->ptr_memoria->avioes[i].y && (*dados)->self.x != (*dados)->self.destino.x ) {
				(*dados)->self.x--;
				(*dados)->self.y--;
			}
		}

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR inicio

		
		// esperar semáforo vazio e o mutex
		WaitForSingleObject((*dados)->sem_vazios, INFINITE);
		WaitForSingleObject((*dados)->mutex, INFINITE);

		// copiar o avião
		CopyMemory(&(*dados)->ptr_modelo->avioesBuffer[(*dados)->ptr_modelo->saiAviao], &(*dados)->self, sizeof(Aviao));
		(*dados)->ptr_modelo->saiAviao = ((*dados)->ptr_modelo->saiAviao + 1) % TAM; // incrementar a posicao de escrita

		// assinalar mutex
		ReleaseMutex((*dados)->mutex);

		// assinala semáforo
		ReleaseSemaphore((*dados)->sem_itens, 1, NULL);

		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR fim
		

		// imprimir a posicao atual do aviao
		_tprintf(TEXT("Aviao na posicao %d %d\n"), (*dados)->self.x, (*dados)->self.y);

		// parar quando chega ao aeroporto
		if ((*dados)->self.x == (*dados)->self.destino.x && ((*dados)->self.y == (*dados)->self.destino.y) ) {
			(*dados)->self.terminarViagem = true;
			(*dados)->self.viajar = false;

			// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR inicio

			WaitForSingleObject((*dados)->sem_vazios, INFINITE);
			WaitForSingleObject((*dados)->mutex, INFINITE);

			// copiar o avião
			CopyMemory(&(*dados)->ptr_modelo->avioesBuffer[(*dados)->ptr_modelo->saiAviao], &(*dados)->self, sizeof(Aviao));
			(*dados)->ptr_modelo->saiAviao = ((*dados)->ptr_modelo->saiAviao + 1) % TAM; // incrementar a posicao de escrita

			// assinalar mutex
			ReleaseMutex((*dados)->mutex);

			// assinala semáforo
			ReleaseSemaphore((*dados)->sem_itens, 1, NULL);

			// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR fim 

			_tprintf(TEXT("O avião chegou ao aeroporto de destino.\n"));
			_tprintf(TEXT("Enter para continuar.\n")); // sair da thread stop
			break;
		}
	}
	return 0;
}
//
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
		dados->self = getAviao(dados->self, *dados);
		dados->self.terminarViagem = false;
		dados->self.setDestino = false;
		dados->self.embarcar = false;
		dados->self.viajar = false;
		_tprintf(TEXT("\nO avião %d encontra-se no aeroporto %s localizado em %d, %d.\n"), dados->self.id, dados->self.inicial.nome, dados->self.x, dados->self.y);
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
					dados->self.setDestino = true;
					// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR inicio

					// esperar semáforo vazio e o mutex
					WaitForSingleObject(dados->sem_vazios, INFINITE);
					WaitForSingleObject(dados->mutex, INFINITE);

					// copiar o avião
					CopyMemory(&dados->ptr_modelo->avioesBuffer[dados->ptr_modelo->saiAviao], &dados->self, sizeof(Aviao));
					dados->ptr_modelo->saiAviao = (dados->ptr_modelo->saiAviao + 1) % TAM; // incrementar a posicao de escrita

					// assinalar mutex
					ReleaseMutex(dados->mutex);

					// assinala semáforo
					ReleaseSemaphore(dados->sem_itens, 1, NULL);

					// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR final

					HANDLE setDestino;
					setDestino = CreateEvent(NULL, FALSE, FALSE, EVENTO_COMANDOS); 
					if (setDestino == NULL) {
						return -1;
					}
					WaitForSingleObject(setDestino, INFINITE); // sincronizar a receção do aeroporto por parte do controlador

					dados->self = getAviao(dados->self, *dados); // obter o novo avião com o destino definido
					if (!dados->self.setDestino) {
						_tprintf(TEXT("Erro ao definir aeroporto de destino.\n"));
					}
					else {
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
						//dados->ptr_memoria->avioes[dados->self.pos].dest = false;
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
		if (_tcsicmp(comando, TEXT("debug")) == 0) {
			_tprintf(TEXT("%d %d"), dados->ptr_memoria->avioes[0].x, dados->ptr_memoria->avioes[0].y);
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

DWORD WINAPI terminarAviao(LPVOID param) {
	TDados* dados = (TDados*)param;
	while (!dados->self.terminar) {
		dados->self = getAviao(dados->self, *dados);
		continue; // se o controlador terminar o aviao
	}
	return 0;
}

DWORD WINAPI AlertaControl(LPVOID param) {
	HANDLE alerta = (HANDLE)param;
	while (1) {
		SetEvent(alerta); // Assinalar evento para o controlador
	}
}

int _tmain(int argc, TCHAR* argv[]) {
	HANDLE objMapMem, objMapMod, sem_control;
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
	dados.self.terminarViagem = false;
	dados.self.setDestino = false;
	dados.self.embarcar = false;
	dados.self.viajar = false;

	// obtem os dados passados por argumento da linha de comando -> aeroporto inicial e velocidade 
	_tcscpy_s(dados.self.inicial.nome, BUFFER, argv[1]);
	dados.self.velocidade = _tstoi(argv[2]);

	// inicializar a memoria partilhada
	objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMapMem == NULL) {
		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
		return;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_memoria == NULL) {
		_tprintf(TEXT("Impossível criar vista de memória partilhada.\n"));
		return;
	}

	// inicilizar a memoria partilhada do modelo produtor consumidor
	objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	if (objMapMod == NULL) {
		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
		return;
	}
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_modelo == NULL) {
		_tprintf(TEXT("Impossivel criar vista de memória partilhada.\n"));
		return;
	}

	// inicializa semáforo para controlar instâncias de aviões
	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);

	// inicializa os semáforos, mutex para o modelo produtor - consumidor
	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);
	if (dados.mutex == NULL) {
		return -1;
	}

	TCHAR alerta[BUFFER];
	HANDLE alertaEvento;

	// criar evento para alerta
	_stprintf_s(alerta, BUFFER, TEXT("alerta %d"), dados.self.id);
	alertaEvento = CreateEvent(NULL, FALSE, FALSE, alerta);

	// criar thread para enviar alertas enquanto o programa nao terminar
	HANDLE hAlerta;
	hAlerta = CreateThread(NULL, 0, AlertaControl, alertaEvento, 0, NULL);

	// esperar pelo semáforo das instâncias do avião
	WaitForSingleObject(dados.sem_avioes, INFINITE);

	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR inicio

	// esperar pelo semáforo das posições vazias e do mutex
	WaitForSingleObject(dados.sem_vazios, INFINITE);
	WaitForSingleObject(dados.mutex, INFINITE);

	// copiar para a memória o avião criado
	CopyMemory(&dados.ptr_modelo->avioesBuffer[dados.ptr_modelo->saiAviao], &dados.self, sizeof(Aviao));

	dados.ptr_modelo->saiAviao = (dados.ptr_modelo->saiAviao + 1) % TAM; // incrementar a posicao de escrita

	// assinalar mutex
	ReleaseMutex(dados.mutex);

	// assinalar semáforo dos itens
	ReleaseSemaphore(dados.sem_itens, 1, NULL);

	// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR final

	// lança a thread para ler os comandos e a thread para terminar
	HANDLE hThread[3];
	hThread[0] = CreateThread(NULL, 0, leComandos, &dados, 0, NULL); 
	hThread[1] = CreateThread(NULL, 0, terminar, &dados, 0, NULL);
	hThread[2] = CreateThread(NULL, 0, terminarAviao, &dados, 0, NULL);
	result = WaitForMultipleObjects(3, hThread, FALSE, INFINITE); // apenas 1 necessita de acabar
	if (result == WAIT_OBJECT_0 + 1) {
		_tprintf(TEXT("O controlador terminou.\n"));
	}
	if (result == WAIT_OBJECT_0 + 2) {
		_tprintf(TEXT("O controlador terminou este aviao.\n"));
	}

	UnmapViewOfFile(dados.ptr_memoria);
	UnmapViewOfFile(dados.ptr_modelo);
	
	return 0;
}