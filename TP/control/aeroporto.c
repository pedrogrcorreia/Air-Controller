#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include "../aeroporto.h"

bool criaAeroporto(Aeroporto* aeroportos, int* numae, int maxae) {
	TCHAR nome[BUFFER];
	int x = 0, y = 0;
	_tprintf(TEXT("Introduza o nome do aeroporto: "));
	_fgetts(nome, BUFFER, stdin);
	nome[_tcslen(nome) - 1] = '\0';
	if (!checkNome(nome, aeroportos, *numae)) {
		_tprintf(TEXT("Já existe um aeroporto com esse nome.\n"));
		return false;
	}
	_tprintf(TEXT("Introduza as coordenadas do novo aeroporto (x, y): "));

	if (_ftscanf_s(stdin, TEXT("%d, %d"), &x, &y) < 2) {
		_tprintf(TEXT("Coordenadas inválidas. Tente novamente.\n"));
		return false;
	}

	if (x < 0 || x>1000 || y < 0 || y>1000) {
		_tprintf(TEXT("Coordenadas fora do mapa. Tente novamente.\n"));
		return false;
	}

	if (!checkCoordenadas(x, y, aeroportos, *numae)) {
		_tprintf(TEXT("Não pode adicionar um aeroporto nessas coordenadas.\n"));
		return false;
	}

	if (*numae >= maxae) {
		_tprintf(TEXT("Não pode adicionar mais aeroportos.\n"));
		return false;
	}

	// adiciona o aeroporto ao array de aeroportos

	_tcscpy_s(aeroportos[*numae].nome, BUFFER, nome);
	aeroportos[*numae].x = x;
	aeroportos[*numae].y = y;
	*numae += 1;
	return true;
}

bool RegistaAeroporto(Aeroporto novo, HANDLE chave) {
	HKEY temp;
	TCHAR x[BUFFER], y[BUFFER];
	//TCHAR x_valor[BUFFER], y_valor[BUFFER];
	DWORD xvalor = novo.x, yvalor = novo.y;
	RegCreateKeyEx(chave, novo.nome, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &temp, NULL);
	_tcscpy_s(x, BUFFER, TEXT("x"));
	_tcscpy_s(y, BUFFER, TEXT("y"));

	RegSetValueEx(temp, (LPCWSTR)x, 0, REG_DWORD, (const BYTE*)&xvalor, sizeof(xvalor));
	RegSetValueEx(temp, (LPCWSTR)y, 0, REG_DWORD, (const BYTE*)&yvalor, sizeof(yvalor));
	RegCloseKey(temp);
	return true;
	
}

bool checkNome(TCHAR nome[], Aeroporto* aeroportos, int numae) {
	for (int i = 0; i < numae; i++) {
		if (_tcsicmp(aeroportos[i].nome, nome) == 0) {
			return false;
		}
	}
	return true;
}

bool checkCoordenadas(int x, int y, Aeroporto* aeroportos, int numae) {
	int xinf = x - 10;
	int xsup = x + 11;
	int yinf = y - 10;
	int ysup = y + 11;
	if (xinf < 0) {
		xinf = 0;
	}
	if (xsup > 1000) {
		xsup = 1000;
	}
	if (yinf < 0) {
		yinf = 0;
	}
	if (ysup > 1000) {
		ysup = 1000;
	}

	for (int i = 0; i < numae; i++) {
		for (int j = xinf; j < xsup; j++) {
			for (int k = yinf; k < ysup; k++) {
				if (aeroportos[i].x == j && aeroportos[i].y == k) {
					return false;
				}
			}
		}
	}
	return true;
<<<<<<< HEAD
}

Aeroporto getAeroporto(TCHAR nome[], Aeroporto* aeroportos, int numae) {
	for (int i = 0; i < numae; i++) {
		if (_tcsicmp(aeroportos[i].nome, nome) == 0) {
			return aeroportos[i];
		}
	}
	return;
}

int getX(TCHAR nome[], Aeroporto* aeroportos, int numae) {
	for (int i = 0; i < numae; i++) {
		if (_tcsicmp(aeroportos[i].nome, nome) == 0) {
			return aeroportos[i].x;
		}
	}
	return -1;
}

int getY(TCHAR nome[], Aeroporto* aeroportos, int numae) {
	for (int i = 0; i < numae; i++) {
		if (_tcsicmp(aeroportos[i].nome, nome) == 0) {
			return aeroportos[i].y;
		}
	}
	return -1;
=======
>>>>>>> parent of bf23a58 (final console version)
}