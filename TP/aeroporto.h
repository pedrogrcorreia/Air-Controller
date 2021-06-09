#include <tchar.h>
#include <stdbool.h>
#define BUFFER 200

typedef struct {
	TCHAR nome[BUFFER];
	int x, y;
} Aeroporto;

// Cria aeroportos
bool criaAeroporto(Aeroporto* aeroportos, int* numae, int maxae);

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