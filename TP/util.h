#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria partilhada")
#define MODELO TEXT("Modelo produtor consumidor")
#define SEMAFORO_CONTROLADOR TEXT("Semáforo de execução do controlador")
#define SEMAFORO_INSTANCIAS TEXT("Semáforo de execução de aviões")
#define CHAVE_AEROPORTOS TEXT("SOFTWARE\\temp\\SO2\\Aeroportos")
#define SEMAFORO_ITENS TEXT("Semáforo dos itens")
#define SEMAFORO_VAZIOS TEXT("Semáforo das posições vazias")
#define MUTEX_CONTROL TEXT("Mutex do controlador")
#define EVENTO_COMANDOS TEXT("Evento para controlo de comandos")
#define TAM 100 // TAMANHO DO BUFFER CIRCULAR

typedef struct {
	DWORD id;
	int x;
	int y;
	int pos;
	int velocidade;
	bool terminar;
	bool terminarViagem;
	bool setDestino;
	bool embarcar;
	bool viajar;
	Aeroporto inicial;
	Aeroporto destino;
} Aviao;

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
	HANDLE eventos[];
} TDados;
