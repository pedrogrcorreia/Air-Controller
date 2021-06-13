#include <windows.h>
#include <tchar.h>
#include <math.h>
#include <stdio.h>
#include <fcntl.h> 
#include <io.h>
#include <stdbool.h>
#include <strsafe.h>
#include "resource.h"
#include "../util.h"

#define BUFSIZE 2048
#define Ps_Sz sizeof(Passageiro)

void iniciaPassageiros(TDados* dados) {
	int i;
	for (i = 0; i < MAXPASSAG; i++) {
		dados->p[i].hPipe = NULL;
	}
}

void adicionaPassageiro(TDados* dados, HANDLE pass) {
	int i;
	for (i = 0; i < MAXPASSAG; i++) {
		if (dados->p[i].hPipe == NULL) {
			dados->p[i].hPipe = pass;
			dados->numpassag++;
			return;
		}
	}
}

void removePassageiro(TDados* dados, HANDLE pass) {
	int i;
	for (i = 0; i < MAXPASSAG; i++) {
		if (dados->p[i].hPipe == pass) {
			dados->p[i].hPipe = NULL;
			dados->numpassag--;
			return;
		}
	}
}

HANDLE WriteReady;

int writePassageiroASINC(HANDLE hPipe, Passageiro p) {
	DWORD cbWritten = 0;
	BOOL fSuccess = FALSE;

	OVERLAPPED OverlWr = { 0 };

	ZeroMemory(&OverlWr, sizeof(OverlWr));
	ResetEvent(WriteReady);
	OverlWr.hEvent = WriteReady;

	fSuccess = WriteFile(hPipe, &p, Ps_Sz, &cbWritten, &OverlWr);

	WaitForSingleObject(WriteReady, INFINITE);
	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
	return 1;
}

int broadcastPassageiros(TDados* dados, Passageiro p) {
	int i, numwrites = 0;
	for (i = 0; i < MAXPASSAG; i++) {
		if (dados->p[i].hPipe != 0) {
			numwrites += writePassageiroASINC(dados->p[i].hPipe, p);
		}
	}
	return numwrites;
}

// FEITA AQUI

void registaPassageiro(TDados* dados, Passageiro p) {
	for (int i = 0; i < MAXPASSAG; i++) {
		if (dados->p[i].hPipe == p.hPipe) {
			_tcscpy_s(dados->p[i].inicial, BUFFER, p.inicial);
			_tcscpy_s(dados->p[i].destino, BUFFER, p.destino);
			_tcscpy_s(dados->p[i].nome, BUFFER, p.nome);
			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Registo"));
			dados->p[i].espera = p.espera;
			dados->p[i].termina = p.termina;
			dados->p[i].voo = p.voo;
			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
			return;
		}
	}
}

void embarcaPassageiro(TDados* dados, Aviao a) {
	for (int i = 0; i < dados->numpassag; i++) {
		if (_tcsicmp(dados->p[i].inicial, a.inicial.nome) == 0 && _tcsicmp(dados->p[i].destino, a.destino.nome) == 0) {
			//_tprintf(TEXT("%s %s"), dados->p[i].inicial, a.inicial.nome);
			//_tprintf(TEXT("%s %s"), dados->p[i].destino, a.destino.nome);
			dados->p[i].voo = a.id;
			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Embarcar"));
			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
			return; // falta meter a lotacao
		}
	}
}

void movePassageiro(TDados* dados, Aviao a) {
	for (int i = 0; i < dados->numpassag; i++) {
		if (dados->p[i].voo == a.id) {
			dados->p[i].x = a.x;
			dados->p[i].y = a.y;
			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Viajar"));
			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
			return;
		}
	}
}

void terminaPassageiro(TDados* dados, Aviao a) {
	for (int i = 0; i < dados->numpassag; i++) {
		if (dados->p[i].voo == a.id) {
			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Termina"));
			dados->p[i].termina = true;
			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
		}
	}
}

void avisaChegada(TDados* dados, Aviao a) {
	for (int i = 0; i < dados->numpassag; i++) {
		if (dados->p[i].voo == a.id) {
			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Chegada"));
			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
			terminaPassageiro(dados, a);
			return;
		}
	}
}



DWORD WINAPI PassagThread(LPVOID param) {
	Passageiro Recebido, Enviado;
	DWORD cbBytesRead = 0, cbReplyBytes = 0;
	BOOL fSuccess = FALSE;
	TDados* dados = (TDados*)param;
	HANDLE hPipe = dados->cPipe;
	HANDLE ReadReady;
	OVERLAPPED OverlRd = { 0 };
	ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	adicionaPassageiro(dados, hPipe);

	while (1) {
		bool terminaI = false, terminaD = false;
		ZeroMemory(&OverlRd, sizeof(OverlRd));
		ResetEvent(ReadReady);
		OverlRd.hEvent = ReadReady;

		fSuccess = ReadFile(hPipe, &Recebido, Ps_Sz, &cbBytesRead, &OverlRd);
		WaitForSingleObject(ReadReady, INFINITE);

		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);
		//_tprintf(TEXT("Passageiro definiu aeroportos %s %s %d\n"), Recebido.inicial, Recebido.destino, Recebido.termina);
		if (Recebido.termina) {
			break;
		}
		_tcscpy_s(Enviado.inicial, BUFFER, Recebido.inicial);
		_tcscpy_s(Enviado.destino, BUFFER, Recebido.destino);
		for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
			if (_tcsicmp(Recebido.inicial, dados->aeroportos[i].nome) == 0) {
				_tcscpy_s(Enviado.inicial, BUFFER, Recebido.inicial);
				terminaI = false;
				break;
			}
			else {
				_tcscpy_s(Enviado.mensagem, BUFFER, TEXT("O aeroporto inicial não existe. O programa vai terminar."));
				terminaI = true;
			}
		}
		if (!terminaI) {
			for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
				if (_tcsicmp(Recebido.destino, dados->aeroportos[i].nome) == 0) {
					_tcscpy_s(Enviado.destino, BUFFER, Recebido.destino);
					terminaD = false;
					break;
				}
				else {
					_tcscpy_s(Enviado.mensagem, BUFFER, TEXT("O aeroporto de destino não existe. O programa vai terminar."));
					terminaD = true;
				}
			}
		}

		if (!terminaI && !terminaD) {
			Enviado.hPipe = hPipe;
			_tcscpy_s(Enviado.nome, BUFFER, Recebido.nome);
			Enviado.termina = false;
			Enviado.voo = Recebido.voo;
			Enviado.espera = Recebido.espera;
			registaPassageiro(dados, Enviado);
		}
		else {
			Enviado.hPipe = hPipe;
			_tcscpy_s(Enviado.nome, BUFFER, Recebido.nome);
			Enviado.termina = true;
			Enviado.voo = Recebido.voo;
			Enviado.espera = Recebido.espera;
			writePassageiroASINC(hPipe, Enviado);
			break;
		}
	}

	removePassageiro(dados, hPipe);
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);

	return 1;
}

DWORD WINAPI RecebePassageiros(LPVOID param) {
	TDados* dados = (TDados*)param;
	BOOL fConnected = FALSE;
	DWORD dwThreadId = 0;
	HANDLE hThread = NULL;
	HANDLE hPipe;
	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);

	iniciaPassageiros(dados);
	while (1) {
		hPipe = CreateNamedPipe(PIPE_CONTROL, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 5000, NULL);
		//_tprintf(TEXT("\nServidor a aguardar por clientes\n"));
		
		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
		dados->cPipe = hPipe;
		if (fConnected) {
			hThread = CreateThread(NULL, 0, PassagThread, (LPVOID)dados, 0, &dwThreadId);
			if (hThread == NULL) {
				return -1;
			}
			else {
				CloseHandle(hThread);
			}
		}
		else {
			CloseHandle(hPipe);
		}
	}
	return 0;
}

DWORD WINAPI RecebeAvioes(LPVOID param) {
	TDados* dados = (TDados*)param;
	Aviao aviao;

	while (!dados->ptr_memoria->terminar) {
		bool novoAviao = true;
		int pos = -1;
		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR inicio

		// esperar semáforo dos aviões
		WaitForSingleObject(dados->sem_itens, INFINITE);

		// copiar o aviao recebido
		CopyMemory(&aviao, &dados->ptr_modelo->avioesBuffer[dados->ptr_modelo->entAviao], sizeof(Aviao));

		for (int i = 0; i <= dados->ptr_memoria->navioes; i++) {
			if (aviao.id == dados->ptr_memoria->avioes[i].id) {
				novoAviao = false;
				pos = i;
				//CopyMemory(&dados->ptr_memoria->avioes[i], &aviao, sizeof(Aviao));
			}
		}

		HANDLE eventoComandos; // evento para lidar com a definição de um destino
		eventoComandos = CreateEvent(NULL, FALSE, FALSE, EVENTO_COMANDOS);

		if (novoAviao) { // é um novo aviao

			// criar o evento de alerta para cada avião
			TCHAR alerta[BUFFER];
			_stprintf_s(alerta, BUFFER, TEXT("alerta %d"), aviao.id);
			dados->eventos[dados->ptr_memoria->navioes] = CreateEvent(NULL, FALSE, FALSE, alerta);

			if (checkNome(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos)) {
				aviao.x = -1;
				aviao.y = -1;
				aviao.terminar = true;
				CopyMemory(&dados->ptr_memoria->avioes[dados->ptr_memoria->navioes], &aviao, sizeof(Aviao)); // acrescenta o novo avião 
				dados->ptr_memoria->navioes++; // incrementa o numero de aviões
			}
			else {
				aviao.inicial = getAeroporto(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
				aviao.x = getX(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
				aviao.y = getY(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
				CopyMemory(&dados->ptr_memoria->avioes[dados->ptr_memoria->navioes], &aviao, sizeof(Aviao)); // acrescenta o avião
				dados->ptr_memoria->navioes++; // incrementa o número de aviões
			}
		}
		else {
			if (aviao.setDestino) { // esta a definir um novo destino
				if (checkNome(aviao.destino.nome, dados->aeroportos, dados->ptr_memoria->naeroportos)) {
					aviao.destino.x = -1;
					aviao.destino.y = -1;
					aviao.setDestino = false;
					_tcscpy_s(aviao.destino.nome, BUFFER, TEXT(""));
					CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
				}
				else {
					aviao.destino = getAeroporto(aviao.destino.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
					aviao.setDestino = true;
					CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
				}
				SetEvent(eventoComandos);
			}
			if (aviao.embarcar) { // embarcar passageiros
				//Passageiro p;
				//_tcscpy_s(p.mensagem, BUFFER, TEXT("Embarcar"));
				//_tcscpy_s(p.inicial, BUFFER, aviao.inicial.nome);
				//_tcscpy_s(p.destino, BUFFER, aviao.destino.nome);
				//p.voo = aviao.id;
				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
				embarcaPassageiro(dados, aviao);
				SetEvent(eventoComandos);
			}
			if (aviao.viajar) { // aviao está em movimento
				//Passageiro p;
				//_tcscpy_s(p.mensagem, BUFFER, TEXT("Viajar"));
				//_tcscpy_s(p.inicial, BUFFER, aviao.inicial.nome);
				//_tcscpy_s(p.destino, BUFFER, aviao.destino.nome);
				//p.x = aviao.x;
				//p.y = aviao.y;
				//p.voo = aviao.id;
				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
				movePassageiro(dados, aviao);
				//broadcastPassageiros(dados, p);
			}
			if (aviao.terminarViagem) { // aviao chegou
				Passageiro p;
				_tcscpy_s(p.mensagem, BUFFER, TEXT("Chegada"));
				_tcscpy_s(p.destino, BUFFER, aviao.destino.nome);
				//_tprintf(TEXT("O avião %d chegou ao Aeroporto de destino.\n"), aviao.id);
				aviao.inicial = aviao.destino;
				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
				avisaChegada(dados, aviao);
			}
		}
		InvalidateRect(dados->hWnd, NULL, TRUE);
		dados->ptr_modelo->entAviao = (dados->ptr_modelo->entAviao + 1) % TAM; // incrementar a posicao de leitura
		// assinala semáforo
		ReleaseSemaphore(dados->sem_vazios, 1, NULL);
		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR fim

		//_tprintf(TEXT("Aviao %d na posicao %d, %d.\n"), aviao.id, aviao.x, aviao.y);
	}
	return 0;
}

DWORD WINAPI RecebeAlerta(LPVOID param) {
	TDados* dados = (TDados*)param;
	DWORD result;
	//TCHAR alerta[BUFFER];
	while (!dados->ptr_memoria->terminar) {
		if (dados->ptr_memoria->navioes > 0) {
			for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
				result = WaitForSingleObject(dados->eventos[i], 1);
				if (result == WAIT_TIMEOUT) {
					_tprintf(TEXT("Avião %d desligou-se.\n"), dados->ptr_memoria->avioes[i].id); // se não chegar é porque o avião se desligou
					terminaPassageiro(dados, dados->ptr_memoria->avioes[i]); // terminar os passageiros que iam no voo
					dados->ptr_memoria->avioes[i] = dados->ptr_memoria->avioes[i + 1]; // retira o avião
					dados->ptr_memoria->navioes--; // decrementa o numero de aviões
					ReleaseSemaphore(dados->sem_avioes, 1, NULL); // retira o aviao do semáforo
					InvalidateRect(dados->hWnd, NULL, TRUE);
				}
			}
		}
		Sleep(3000);
	}
	return 0;
}

DWORD WINAPI suspend(LPVOID param) {
	TDados** dados = (TDados**)param;
	for (int i = (*dados)->ptr_memoria->navioes; i < (*dados)->ptr_memoria->maxavioes; i++) {
		WaitForSingleObject((*dados)->sem_avioes, INFINITE);
	}
	return 0;
}
TCHAR szProgName[] = TEXT("Base");

LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

ATOM RegistaClasse(HINSTANCE hInst, TCHAR* szWinName) {
	WNDCLASSEX wcl;

	wcl.cbSize = sizeof(WNDCLASSEX);
	wcl.hInstance = hInst;
	wcl.lpszClassName = szWinName;
	wcl.lpfnWndProc = TrataEventos;
	wcl.style = CS_HREDRAW;
	wcl.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
	wcl.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(IDI_ICON1));
	wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcl.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcl.cbClsExtra = sizeof(TDados);
	wcl.cbWndExtra = sizeof(TDados);// sizeof(TDados*);
	wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

	return RegisterClassEx(&wcl);
}

HWND CriarJanela(HINSTANCE hInst, TCHAR* szWinName) {
	return CreateWindow(
		szWinName,
		TEXT("Controlador Aéreo"),
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		1000,
		1000,
		HWND_DESKTOP,
		NULL,
		hInst,
		NULL
	);
}

HWND CriarJanelaPopUp(HINSTANCE hInst, TCHAR* szWinName, HWND hWnd, int x, int y) {
	return CreateWindowEx(
		0,
		TEXT("STATIC"),
		szWinName,
		WS_POPUP,
		x,
		y,
		100,
		100,
		hWnd,
		(HMENU)0,
		hInst,
		NULL);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HANDLE semaforo_execucao, objMapMem, objMapMod, hThread;
	HKEY chaveMAX = NULL;
	DWORD result = 0, cbdata = sizeof(DWORD);
	TDados dados;


#ifdef UNICODE 
	if (_setmode(_fileno(stdin), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
	if (_setmode(_fileno(stdout), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
	if (_setmode(_fileno(stderr), _O_WTEXT) == -1) {
		perror("Impossivel user _setmode()");
	}
#endif

	/* CRIACAO DE JANELA */
	TCHAR janelaPrinc[] = TEXT("Controlador Aéreo");
	HWND hWnd;
	MSG msg;
	if (!RegistaClasse(hInst, janelaPrinc)) {
		return 0;
	}

	hWnd = CriarJanela(hInst, janelaPrinc);

	/* FIM DA CRIACAO DE JANELA */

	// verifica se já está alguma instância em execução
	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		MessageBox(hWnd, TEXT("Já existe um controlador em execução.\nPor favor termine - o para iniciar um novo.\n"), TEXT("ERRO"), MB_ICONERROR);
		return -1;
	}

	// inicializar a memoria partilhada
	objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMapMem == NULL) {
		MessageBox(hWnd, TEXT("Impossível criar objecto de mapping.\n"), TEXT("ERRO"), MB_ICONERROR);
		return -1;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_memoria == NULL) {
		MessageBox(hWnd, TEXT("Impossível criar objecto de mapping.\n"), TEXT("ERRO"), MB_ICONERROR);
		return -1;
	}

	// inicilizar a memoria partilhada do modelo produtor consumidor
	objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	if (objMapMod == NULL) {
		MessageBox(hWnd, TEXT("Impossível criar objecto de mapping.\n"), TEXT("ERRO"), MB_ICONERROR);
		return -1;
	}
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_modelo == NULL) {
		MessageBox(hWnd, TEXT("Impossível criar objecto de mapping.\n"), TEXT("ERRO"), MB_ICONERROR);
		return -1;
	}

	// verifica se a chave abre
	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\temp\\SO2\\TP"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chaveMAX);

	// obter o numero maximo de aeroportos e avioes
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAE"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxaeroportos, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		MessageBox(hWnd, TEXT("\nNão foi possível ler do registo o número máximo de aeroportos.\nVai ser definido como 10.\n"), TEXT("AVISO"), MB_ICONINFORMATION);
		dados.ptr_memoria->maxaeroportos = 10;
	}
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxavioes, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		MessageBox(hWnd, TEXT("\nNão foi possível ler do registo o número máximo de aviões.\nVai ser definido como 10.\n"), TEXT("AVISO"), MB_ICONINFORMATION);
		dados.ptr_memoria->maxavioes = 10;
	}

	RegCloseKey(chaveMAX);

	// inicializa semáforo para controlar instâncias de aviões
	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);

	// inicializa os semáforos, mutex para o modelo produtor - consumidor
	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);

	// inicializar a condição de paragem a null
	dados.ptr_memoria->terminar = false;
	dados.ptr_memoria->navioes = 0;
	dados.ptr_memoria->naeroportos = 0;

	// inicializa array de aeroportos
	dados.aeroportos = malloc(sizeof(Aeroporto) * dados.ptr_memoria->maxaeroportos);
	memset(dados.aeroportos, 0, (size_t)dados.ptr_memoria->maxaeroportos * sizeof(Aeroporto));

	// DEBUG PARA SER MAIS FACIL UTILIZAR 
	_tcscpy_s(dados.aeroportos[0].nome, BUFFER, TEXT("Lisboa"));
	dados.aeroportos[0].x = 30;
	dados.aeroportos[0].y = 40;
	_tcscpy_s(dados.aeroportos[1].nome, BUFFER, TEXT("Porto"));
	dados.aeroportos[1].x = 500;
	dados.aeroportos[1].y = 500;
	dados.ptr_memoria->naeroportos = 2;

	// lança thread para controlar a entrada de aviões
	hThread = CreateThread(NULL, 0, RecebeAvioes, &dados, 0, NULL);

	// inicializa a thread para receber alertas
	HANDLE hAlerta;
	hAlerta = CreateThread(NULL, 0, RecebeAlerta, &dados, 0, NULL);

	dados.numpassag = 0;
	//inicializa a thread para receber passageiros
	HANDLE hPassag;
	hPassag = CreateThread(NULL, 0, RecebePassageiros, (LPVOID)&dados, 0, NULL);

	// inicializa suspensao de dados como falso
	dados.suspend = false;

	dados.hWnd = hWnd;
	dados.hPop = NULL;

	LONG_PTR oldWnd;
	oldWnd = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&dados);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}


	free(dados.aeroportos);
	dados.ptr_memoria->terminar = true;
	return msg.wParam;
}

BOOL CALLBACK dListarAeroportos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	TCHAR texto[BUFFER], titulo[BUFFER];
	int indice = 0;
	TDados* dados;
	HWND h;
	h = GetParent(hWnd);
	dados = (TDados*)GetWindowLongPtr(h, GWLP_USERDATA);

	switch (messg) {
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			EndDialog(hWnd, 0);
		}
		if (LOWORD(wParam) == IDC_LIST1 && HIWORD(wParam) == LBN_DBLCLK) {
			int i = (int)SendDlgItemMessage(hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0);
			indice = (int)SendDlgItemMessage(hWnd, IDC_LIST1, LB_GETITEMDATA, i, 0);
			_stprintf_s(texto, BUFFER, TEXT("Aeroporto %s nas coordenadas %d, %d."), dados->aeroportos[indice].nome, dados->aeroportos[indice].x, dados->aeroportos[indice].y);
			_stprintf_s(titulo, BUFFER, TEXT("Aeroporto %s"), dados->aeroportos[indice].nome);
			MessageBox(hWnd, texto, titulo, MB_OK);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
			int pos;
			pos = (int)SendDlgItemMessage(hWnd, IDC_LIST1, LB_ADDSTRING, 0, (LPARAM)dados->aeroportos[i].nome);
			SendDlgItemMessage(hWnd, IDC_LIST1, LB_SETITEMDATA, pos, (LPARAM)i);
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK dCriarAeroportos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	TCHAR texto[BUFFER], nome[BUFFER], xT[BUFFER], yT[BUFFER];
	TDados* dados;
	HWND h;
	int x = 0, y = 0, result;
	h = GetParent(hWnd);
	dados = (TDados*)GetWindowLongPtr(h, GWLP_USERDATA);
	switch (messg) { //Eventos da caixa de diálogo
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BSAIR) {
			EndDialog(hWnd, 0);
		}
		if (LOWORD(wParam) == IDC_BADD) {
			//Buscar texto à caixa de texto
			result = GetDlgItemText(hWnd, IDC_EDITNOME, nome, BUFFER);
			if (result == 0) {
				_stprintf_s(texto, BUFFER, TEXT("Não foi possível adicionar o aeroporto %s nas coordenadas %d, %d."), nome, x, y);
				MessageBox(hWnd, texto, TEXT("Erro ao adicionar aeroporto"), MB_ICONERROR);
				return FALSE;
			}
			result = GetDlgItemText(hWnd, IDC_EDITX, xT, BUFFER);
			if (result == 0) {
				_stprintf_s(texto, BUFFER, TEXT("Não foi possível adicionar o aeroporto %s nas coordenadas %d, %d."), nome, x, y);
				MessageBox(hWnd, texto, TEXT("Erro ao adicionar aeroporto"), MB_ICONERROR);
				return FALSE;
			}
			result = GetDlgItemText(hWnd, IDC_EDITY, yT, BUFFER);
			if (result == 0) {
				_stprintf_s(texto, BUFFER, TEXT("Não foi possível adicionar o aeroporto %s nas coordenadas %d, %d."), nome, x, y);
				MessageBox(hWnd, texto, TEXT("Erro ao adicionar aeroporto"), MB_ICONERROR);
				return FALSE;
			}
			x = _tstoi(xT);
			y = _tstoi(yT);
			if (criaAeroportoGUI(dados->aeroportos, &dados->ptr_memoria->naeroportos, dados->ptr_memoria->maxaeroportos, nome, x, y)) {
				_stprintf_s(texto, BUFFER, TEXT("Adicionado aeroporto %s nas coordenadas %d, %d."), nome, x, y);
				MessageBox(hWnd, texto, TEXT("Aeroporto adicionado"), MB_OK);
				InvalidateRect(h, NULL, TRUE);
				//SendMessage(hWnd, WM_INITDIALOG, lParam, wParam);
				return TRUE;
			}
			else {
				_stprintf_s(texto, BUFFER, TEXT("Não foi possível adicionar o aeroporto %s nas coordenadas %d, %d."), nome, x, y);
				MessageBox(hWnd, texto, TEXT("Erro ao adicionar aeroporto"), MB_ICONERROR);
				return TRUE;
			}
		}
		break;
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}

LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {

	static HBITMAP bmpAviao, bmpAeroporto;
	HBITMAP foto;
	HDC hdc;
	static int maxX, maxY;
	static HDC double_dc;
	HDC auxdc;
	static HANDLE thread;
	PAINTSTRUCT paint;
	int x = 0, y = 0;

	TDados* dados;
	dados = (TDados*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	bool g_fMouseTracking = FALSE;
	HWND hPop = NULL;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	switch (messg) {
		case WM_CREATE:
			// criar double buffer
			bmpAviao = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_AVIAO));
			bmpAeroporto = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_AEROPORTO));
			maxX = GetSystemMetrics(SM_CXSCREEN);
			maxY = GetSystemMetrics(SM_CYSCREEN);
			hdc = GetDC(hWnd);
			double_dc = CreateCompatibleDC(hdc);
			foto = CreateCompatibleBitmap(hdc, maxX, maxY);
			SelectObject(double_dc, foto);
			SelectObject(double_dc, GetStockObject(GRAY_BRUSH));
			PatBlt(double_dc, 0, 0, maxX, maxY, PATCOPY);
			DeleteObject(foto);
			ReleaseDC(hWnd, hdc);
			break;
		case WM_LBUTTONDOWN:
			hdc = GetDC(hWnd);
			auxdc = CreateCompatibleDC(hdc);
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if (dados->hPop != NULL) {
				ShowWindow(dados->hPop, SW_HIDE);
				DestroyWindow(dados->hPop);
				dados->hPop = NULL;
			}
			for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
				if(dados->aeroportos[i].x <= x && (dados->aeroportos[i].x+30) >= x && dados->aeroportos[i].y <= y && (dados->aeroportos[i].y+30) >= y){
					int np = 0, na = 0;
					TCHAR text[BUFFER];
					for (int j = 0; j < dados->numpassag; j++) {
						if (_tcsicmp(dados->aeroportos[i].nome, dados->p[j].inicial) == 0 && dados->p[j].voo == -1) {
							np++;
						}
					}
					for (int j = 0; j < dados->ptr_memoria->navioes; j++) {
						if (_tcsicmp(dados->aeroportos[i].nome, dados->ptr_memoria->avioes[j].inicial.nome) == 0 && dados->ptr_memoria->avioes[j].viajar == false) {
							na++;
						}
					}
					_stprintf_s(text, BUFFER, TEXT("Aeroporto %s.\nNº avioes: %d\nNº passageiros: %d\n"), dados->aeroportos[i].nome, na, np);
					dados->hPop = CriarJanelaPopUp(hInstance, text, hWnd, x, y);
					ShowWindow(dados->hPop, SW_SHOW);
				}
			}
			SelectObject(auxdc, dados->hPop);
			BitBlt(hdc, x, y, 1000, 1000, auxdc, 0, 0, SRCCOPY);
			BitBlt(double_dc, x, y, 1000, 1000, auxdc, 0, 0, SRCCOPY);
			DeleteDC(auxdc);
			ReleaseDC(hWnd, hdc);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
		case WM_DESTROY:
			DeleteObject(bmpAviao);
			DeleteObject(bmpAeroporto);
			PostQuitMessage(0);
			break;
		case WM_SIZE:
			GetWindowRect(hWnd, &paint.rcPaint);
			//_stprintf_s(texto, 100, TEXT("Dimensões da janela: largura-> %d altura -> %d"), paint.rcPaint.right, paint.rcPaint.bottom);
			//altura = paint.rcPaint.bottom / 2 - 134;
			x = paint.rcPaint.right / 2 - 30;
			InvalidateRect(hWnd, NULL, TRUE);
			//Calcular nova altura e largura
			//Gerar um evento WM_PAINT
			break;
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &paint);//GetDC(hWnd);

			//EndPaint(hWnd, &paint);

			auxdc = CreateCompatibleDC(hdc);


			x = LOWORD(lParam);
			y = HIWORD(lParam);

			SelectObject(auxdc, bmpAviao);
			if (dados->ptr_memoria->navioes > 0) {
				for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
					BitBlt(hdc, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
					BitBlt(double_dc, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
				}
			}

			SelectObject(auxdc, bmpAeroporto);
			if (dados->ptr_memoria->naeroportos > 0) {
				for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
					BitBlt(hdc, dados->aeroportos[i].x, dados->aeroportos[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
					BitBlt(double_dc, dados->aeroportos[i].x, dados->aeroportos[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
				}
			}
			BitBlt(double_dc, x, y, 134, 134, auxdc, 0, 0, SRCCOPY);
			DeleteDC(auxdc);
			ReleaseDC(hWnd, hdc);

			//hdc = BeginPaint(hWnd, &paint);
			//auxdc = CreateCompatibleDC(hdc);
			//SelectObject(auxdc, bmp);
			//BitBlt(hdc, 0, 0, maxX, maxY, double_dc, 0, 0, SRCCOPY);
			//DeleteDC(auxdc);
			//if (dados->ptr_memoria->navioes > 0) {
			//	for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
			//		TextOut(hdc, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y, texto, _tcslen(texto));
			//		//BitBlt(hdc, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y, 30, 30, bmp, 0, 0, SRCCOPY);
			//	}
			//} //FUNCIONA




			//if (dados->ptr_memoria->naeroportos > 0) {
			//	for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
			//		TextOut(hdc, x, y, dados->aeroportos[i].nome, _tcslen(dados->aeroportos[i].nome));
			//		x += 100;
			//	}
			//}
			EndPaint(hWnd, &paint);
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_AEROPORTOS_CRIARAEROPORTOS) {
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGCRIAR), hWnd, dCriarAeroportos);
			}
			if (LOWORD(wParam) == ID_AEROPORTOS_LISTARAEROPORTOS) {
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGLISTAR), hWnd, dListarAeroportos);
			}
			if (LOWORD(wParam) == ID_AVSUS) {
				HANDLE susThread;
				if (!dados->suspend) {
					susThread = CreateThread(NULL, 0, suspend, &dados, 0, NULL); // thread para ocupar todas as posições do semáforo
					dados->suspend = true;
					MessageBox(hWnd, TEXT("Registo dos aviões suspenso."), TEXT("Suspender registos."), MB_OK);
				}
				else {
					MessageBox(hWnd, TEXT("Registo de aviões já se encontra suspenso."), TEXT("Suspender registos"), MB_ICONERROR);
				}
			}
			if (LOWORD(wParam) == ID_AVREG) {
				if (dados->suspend) {
					int release = 0;
					if (dados->ptr_memoria->maxavioes - dados->ptr_memoria->navioes < 0) {
						release = 0; // se todas as posições tiverem ocupadas antes da suspensão 
					}
					else {
						release = dados->ptr_memoria->maxavioes - dados->ptr_memoria->navioes; // numero de posições a libertar
					}
					ReleaseSemaphore(dados->sem_avioes, release, NULL);
					dados->suspend = false;
					MessageBox(hWnd, TEXT("Registo dos aviões ativo."), TEXT("Ativar registos"), MB_OK);
				}
				else {
					MessageBox(hWnd, TEXT("Registo de aviões já se encontra ativo."), TEXT("Ativar registos"), MB_ICONERROR);
				}
			}
			break;
		/*case WM_ERASEBKGND:
			return (LRESULT)1;
			break;
			*/
		case  WM_MOUSEMOVE:
			if (!g_fMouseTracking && !dados->hPop)
			{
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_HOVER | TME_LEAVE;
				tme.hwndTrack = hWnd;
				tme.dwHoverTime = 0;
				g_fMouseTracking = TrackMouseEvent(&tme);
			}
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			break;
		case  WM_MOUSEHOVER:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			g_fMouseTracking = FALSE;
			for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
				if (!dados->hPop) {
					if (dados->ptr_memoria->avioes[i].x <= x && (dados->ptr_memoria->avioes[i].x + 30) >= x && dados->ptr_memoria->avioes[i].y <= y && (dados->ptr_memoria->avioes[i].y + 30) >= y) {
						TCHAR text[BUFFER];
						int np = 0;
						for (int j = 0; j < dados->numpassag; j++) {
							if (dados->p[j].voo == dados->ptr_memoria->avioes[i].id) {
								np++;
							}
						}
						_stprintf_s(text, BUFFER, TEXT("Avião %d.\nPartida: %s\nDestino: %s\nNº Passageiros: %d"), dados->ptr_memoria->avioes[i].id, dados->ptr_memoria->avioes[i].inicial.nome, dados->ptr_memoria->avioes[i].destino.nome, np);
						dados->hPop = CriarJanelaPopUp(hInstance, text, hWnd, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y);
						ShowWindow(dados->hPop, SW_SHOW);

						TRACKMOUSEEVENT tme;
						tme.cbSize = sizeof(TRACKMOUSEEVENT);
						tme.dwFlags = TME_LEAVE;
						tme.hwndTrack = hWnd;
						g_fMouseTracking = TrackMouseEvent(&tme);
					}
				}
			}
			return 0;
			break;
		case  WM_MOUSELEAVE:
			g_fMouseTracking = FALSE; 
			if (dados->hPop)
			{
				ShowWindow(dados->hPop, SW_HIDE);
				DestroyWindow(dados->hPop);
				dados->hPop = NULL;
			}
			return 0;
			break;
		default:
			return(DefWindowProc(hWnd, messg, wParam, lParam));
			break;
		}
	return(0);
}