#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria")
#define SEMAFORO_CONTROLADOR TEXT("Sem�foro de execu��o do controlador")
#define SEMAFORO_INSTANCIAS TEXT("Sem�foro de execu��o de avi�es")
#define CHAVE_AEROPORTOS TEXT("SOFTWARE\\temp\\SO2\\Aeroportos")
#define SEMAFORO_ITENS TEXT("Sem�foro dos itens")
#define SEMAFORO_VAZIOS TEXT("Sem�foro das posi��es vazias")
#define MUTEX_CONTROL TEXT("Mutex do controlador")
#define EVENTO TEXT("Evento para chegada")

typedef struct {
	DWORD id;
	int x;
	int y;
	int velocidade;
	bool terminar;
	Aeroporto inicial;
	Aeroporto destino;
} Aviao;

typedef struct {
	int mapa[1000][1000];
	int maxaeroportos;
	int maxavioes;
	int naeroportos;
	int navioes;
	int entAviao; // controlo do modelo produtor - consumidor
	int saiAviao; // controlo do modelo produtor - consumidor
	bool terminar; // condi��o de paragem do programa
	Aviao avioes[]; // array de avioes, onde vao ser colocados os "itens" do modelo produtor - consumidor
} Memoria;

typedef struct {
	Aviao self;
	Memoria* ptr_memoria;
	HANDLE sem_avioes; // sem�foro de inst�ncias de avi�o a correr
	HANDLE sem_vazios; // posi��es vazios do modelo produtor - consumidor
	HANDLE sem_itens; // itens do modelo produtor - consumidor
	HANDLE mutex; // mutex para o modelo produtor - consumidor
	HKEY chaveAeroportos;
	bool suspend;
	int id;
} TDados;
