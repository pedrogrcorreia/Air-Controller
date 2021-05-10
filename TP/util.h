#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria")
#define SEMAFORO_CONTROLADOR TEXT("Sem�foro de execu��o do controlador")
#define SEMAFORO_INSTANCIAS TEXT("Sem�foro de execu��o de avi�es")
#define CHAVE_AEROPORTOS TEXT("SOFTWARE\\temp\\SO2\\Aeroportos")
#define SEMAFORO_ITENS TEXT("Sem�foro dos itens")
#define SEMAFORO_VAZIOS TEXT("Sem�foro das posi��es vazias")
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
