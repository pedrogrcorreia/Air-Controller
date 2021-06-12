#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria partilhada")
#define MODELO TEXT("Modelo produtor consumidor")
#define SEMAFORO_CONTROLADOR TEXT("Semáforo de execução do controlador")
#define SEMAFORO_INSTANCIAS TEXT("Semáforo de execução de aviões")
#define SEMAFORO_ITENS TEXT("Semáforo dos itens")
#define SEMAFORO_VAZIOS TEXT("Semáforo das posições vazias")
#define MUTEX_CONTROL TEXT("Mutex do controlador")
#define EVENTO_COMANDOS TEXT("Evento para controlo de comandos")
#define PIPE_CONTROL TEXT("\\\\.\\pipe\\control")
#define PIPE_TESTE TEXT("\\\\.\\pipe\\teste")
#define BUFFER 200
#define TAM 100 // TAMANHO DO BUFFER CIRCULAR
#define MAXPASSAG 25 // NUMERO MÁXIMO DE PASSAGEIROS

typedef struct {
	DWORD id;
	int x;
	int y;
	int pos;
	int velocidade;
	int lotacao;
	bool terminar;
	bool terminarViagem;
	bool setDestino;
	bool embarcar;
	bool viajar;
	Aeroporto inicial;
	Aeroporto destino;
} Aviao;

typedef struct {
	TCHAR inicial[BUFFER];
	TCHAR destino[BUFFER];
	TCHAR mensagem[BUFFER];
	bool termina;
	int voo;
	int x, y;
	HANDLE hPipe;
} Passageiro;

typedef struct {
	int entAviao; // controlo do modelo produtor - consumidor
	int saiAviao; // controlo do modelo produtor - consumidor
	Aviao avioesBuffer[TAM]; // array de avioes, onde vao ser colocados os "itens" do modelo produtor - consumidor
} Modelo;

typedef struct {
	int navioes;
	int maxavioes;
	int maxaeroportos;
	int naeroportos;
	bool terminar;
	Aviao avioes[];
} Memoria;

typedef struct {
	Modelo* ptr_modelo;
	Memoria* ptr_memoria;
	HANDLE sem_vazios;
	HANDLE sem_itens;
	HANDLE mutex;
	Aviao self;
	HANDLE sem_avioes;
	Aeroporto* aeroportos;
	bool suspend;
	HWND hWnd;
	HANDLE cPipe;
	int numpassag;
	Passageiro p[MAXPASSAG];
	HANDLE eventos[];
} TDados;