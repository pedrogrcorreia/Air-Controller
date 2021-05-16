#pragma once
#include "aeroporto.h"
#define MEMORIA TEXT("Memoria partilhada")
#define SEMAFORO_CONTROLADOR TEXT("Sem�foro de execu��o do controlador")
#define SEMAFORO_INSTANCIAS TEXT("Sem�foro de execu��o de avi�es")
#define CHAVE_AEROPORTOS TEXT("SOFTWARE\\temp\\SO2\\Aeroportos")
#define SEMAFORO_ITENS TEXT("Sem�foro dos itens")
#define SEMAFORO_VAZIOS TEXT("Sem�foro das posi��es vazias")
#define MUTEX_CONTROL TEXT("Mutex do controlador")
#define TAM 100

typedef struct {
	DWORD id;
	int x;
	int y;
	int velocidade;
	bool terminar;
	Aeroporto inicial;
	Aeroporto destino;
	HANDLE eventos[2]; // 0 -> alerta // 1 -> chegada aeroporto
} Aviao;

typedef struct {
	int maxaeroportos;
	int maxavioes;
	int naeroportos;
	int navioes;
	int entAviao; // controlo do modelo produtor - consumidor
	int saiAviao; // controlo do modelo produtor - consumidor
	bool terminar; // condi��o de paragem do programa
	Aviao avioes[TAM]; // array de avioes, onde vao ser colocados os "itens" do modelo produtor - consumidor
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
} TDados;
