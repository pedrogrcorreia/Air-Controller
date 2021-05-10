#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria")
#define SEMAFORO_CONTROLADOR TEXT("Semáforo de execução do controlador")
#define SEMAFORO_INSTANCIAS TEXT("Semáforo de execução de aviões")
#define CHAVE_AEROPORTOS TEXT("SOFTWARE\\temp\\SO2\\Aeroportos")
#define SEMAFORO_ITENS TEXT("Semáforo dos itens")
#define SEMAFORO_VAZIOS TEXT("Semáforo das posições vazias")
#define MUTEX_CONTROL TEXT("Mutex do controlador")

typedef struct {
	DWORD id;
	int x;
	int y;
} Aviao;

typedef struct {
	int mapa[50][50];
	int maxaeroportos;
	int maxavioes;
	int naeroportos;
	int navioes;
	int entAviao;
	int saiAviao;
	bool terminar;
	Aviao avioes[];
} Memoria;

typedef struct {
	Aviao self;
	Memoria* ptr_memoria;
	HANDLE sem_avioes;
	HANDLE sem_vazios;
	HANDLE sem_itens;
	HANDLE mutex;
	bool suspend;
	int id;
} TDados;
