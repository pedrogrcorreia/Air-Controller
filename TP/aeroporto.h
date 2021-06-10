#include <tchar.h>
#include <stdbool.h>
#define BUFFER 200

typedef struct {
	TCHAR nome[BUFFER];
	int x, y;
} Aeroporto;

// Cria aeroportos
bool criaAeroporto(Aeroporto* aeroportos, int* numae, int maxae);

// Cria aeroportos com suporte a interface gráfica
bool criaAeroportoGUI(Aeroporto* aeroportos, int* numae, int maxae, TCHAR nome[], int x, int y);

// Verifica se o nome do novo aeroporto ainda não existe
bool checkNome(TCHAR nome[], Aeroporto* aeroportos, int numae);

// Verifica se as coordenadas inseridas estão dísponiveis
bool checkCoordenadas(int x, int y, Aeroporto* aeroportos, int numae);

// Devolve o aeroporto pretendido
Aeroporto getAeroporto(TCHAR nome[], Aeroporto* aeroportos, int numae);

// Devolve a coordenada X do aeroporto
int getX(TCHAR nome[], Aeroporto* aeroportos, int numae);

// Devolve a coordenada X do aeroporto
int getY(TCHAR nome[], Aeroporto* aeroportos, int numae);