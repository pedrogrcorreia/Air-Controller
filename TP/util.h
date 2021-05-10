#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria")
#define CHAVE_AEROPORTOS TEXT("SOFTWARE\\temp\\SO2\\Aeroportos")
#define SEMAFORO_AVIOES TEXT("Semáforo dos aviões")
#define SEMAFORO_VAZIOS TEXT("Semáforo vazios")
#define MUTEX_CONTROL TEXT("Mutex do Controlador")

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
	HANDLE mutex;
	int id;
} TDados;
