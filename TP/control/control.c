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

#define BUFFER 200

void listaAeroportos(Aeroporto aeroportos[], int nae) {
	_tprintf(TEXT("\nExistem %d aeroportos.\n"), nae);
	for (int i = 0; i < nae; i++) {
		_tprintf(TEXT("\nAeroporto %s, localizado em %d, %d.\n"), aeroportos[i].nome, aeroportos[i].x, aeroportos[i].y);
	}
}

void listaAvioes(Aviao avioes[], int nav) {
	_tprintf(TEXT("\nExistem %d aviões.\n"), nav);
	for (int i = 0; i < nav; i++) {
		_tprintf(TEXT("\nAvião %d na posição %d, %d.\n"), avioes[i].id, avioes[i].x, avioes[i].y);
		if (avioes[i].setDestino) {
			_tprintf(TEXT("\nAvião %d com destino a %s em %d %d.\n"), avioes[i].id, avioes[i].destino.nome, avioes[i].destino.x, avioes[i].destino.y);
		}
	}
}

DWORD WINAPI suspend(LPVOID param) {
	TDados** dados = (TDados**)param;
	for (int i = (*dados)->ptr_memoria->navioes; i < (*dados)->ptr_memoria->maxavioes; i++) {
		WaitForSingleObject((*dados)->sem_avioes, INFINITE);
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
				//fazer alguma coisa
			}
			if (aviao.viajar) { // aviao está em movimento
				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
			}
			if (aviao.terminarViagem) { // aviao chegou
				_tprintf(TEXT("O avião %d chegou ao Aeroporto de destino.\n"), aviao.id);
				aviao.inicial = aviao.destino;
				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
			}
		}
		InvalidateRect(dados->hWnd, NULL, TRUE);
		TextOut(dados->hWnd, 40, 40, TEXT("DEBUG"), _tcslen(TEXT("DEBUG")));
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
	TCHAR alerta[BUFFER];
	while (!dados->ptr_memoria->terminar) {
		if (dados->ptr_memoria->navioes > 0) {
			for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
				result = WaitForSingleObject(dados->eventos[i], 1);
				if (result == WAIT_TIMEOUT) {
					_tprintf(TEXT("Avião %d desligou-se.\n"), dados->ptr_memoria->avioes[i].id); // se não chegar é porque o avião se desligou
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
	wcl.cbWndExtra = sizeof(TDados*);
	wcl.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);

	return RegisterClassEx(&wcl);
}

HWND CriarJanela(HINSTANCE hInst, TCHAR* szWinName) {

	return CreateWindow(
		szWinName,
		TEXT("Controlador Aéreo"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1000,
		1000,
		HWND_DESKTOP,
		NULL,
		hInst,
		NULL
	);
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
	HANDLE semaforo_execucao, objMapMem, objMapMod, hThread;
	HKEY chaveMAX = NULL;
	DWORD result = 0, cbdata = sizeof(DWORD);
	TDados dados;
	
	TCHAR janelaPrinc[] = TEXT("Controlador Aéreo");
	HWND hWnd;
	MSG msg;
	if (!RegistaClasse(hInst, janelaPrinc)) {
		return 0;
	}

	hWnd = CriarJanela(hInst, janelaPrinc);

	// verifica se já está alguma instância em execução
	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
	result = GetLastError();
	if (result == ERROR_ALREADY_EXISTS) {
		_tprintf(TEXT("Já existe um controlador em execução.\nPor favor termine-o para iniciar um novo.\n"));
		return -1;
	}

	// inicializar a memoria partilhada
	objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
	if (objMapMem == NULL) {
		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
		return;
	}
	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_memoria == NULL) {
		_tprintf(TEXT("Impossível criar vista de memória partilhada.\n"));
		return;
	}

	// inicilizar a memoria partilhada do modelo produtor consumidor
	objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
	if (objMapMod == NULL) {
		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
		return;
	}
	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (dados.ptr_modelo == NULL) {
		_tprintf(TEXT("Impossivel criar vista de memória partilhada.\n"));
		return;
	}

	// verifica se a chave abre
	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\temp\\SO2\\TP"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chaveMAX);

	// obter o numero maximo de aeroportos e avioes
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAE"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxaeroportos, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("\nNão foi possível ler do registo o número máximo de aeroportos.\nVai ser definido como 10.\n"));
		dados.ptr_memoria->maxaeroportos = 10;
	}
	result = RegQueryValueEx(chaveMAX, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxavioes, (LPDWORD)&cbdata);
	if (result != ERROR_SUCCESS) {
		_tprintf(TEXT("\nNão foi possível ler do registo o número máximo de aviões.\nVai ser definido como 20.\n"));
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

	// DEBUG
	//_tprintf(TEXT("NUMERO MAXIMO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->maxaeroportos);
	//_tprintf(TEXT("NUMERO MAXIMO DE AVIOES: %ld\n"), dados.ptr_memoria->maxavioes);
	//_tprintf(TEXT("NUMERO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->naeroportos);
	//_tprintf(TEXT("NUMERO DE AVIOES: %ld\n"), dados.ptr_memoria->navioes);


	// inicializa array de aeroportos
	dados.aeroportos = malloc(sizeof(Aeroporto) * dados.ptr_memoria->maxaeroportos);
	memset(dados.aeroportos, 0, (size_t)dados.ptr_memoria->maxaeroportos * sizeof(Aeroporto));

	/* DEBUG PARA SER MAIS FACIL UTILIZAR */
	_tcscpy_s(dados.aeroportos[0].nome, BUFFER, TEXT("Lisboa"));
	dados.aeroportos[0].x = 30;
	dados.aeroportos[0].y = 40;
	_tcscpy_s(dados.aeroportos[1].nome, BUFFER, TEXT("Porto"));
	dados.aeroportos[1].x = 500;
	dados.aeroportos[1].y = 500;
	dados.ptr_memoria->naeroportos = 2;

	// inicializa suspensao de dados como falso
	dados.suspend = false;

	dados.hWnd = hWnd;

	// lança thread para controlar a entrada de aviões
	hThread = CreateThread(NULL, 0, RecebeAvioes, &dados, 0, NULL);

	// inicializa a thread para receber alertas
	HANDLE hAlerta;
	hAlerta = CreateThread(NULL, 0, RecebeAlerta, &dados, 0, NULL);
	LONG_PTR oldWnd;
	oldWnd = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)&dados);
	result = GetLastError();
	//if (teste == 0) {
	//	return result;
	//}

	ShowWindow(hWnd, nCmdShow);
	//UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//dados.ptr_memoria->terminar = true;
}

BOOL CALLBACK dListarAeroportos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	TCHAR texto[BUFFER], titulo[BUFFER];
	int indice;
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
			indice = SendDlgItemMessage(hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0);
			_stprintf_s(texto, BUFFER, TEXT("Aeroporto %s nas coordenadas %d, %d."), dados->aeroportos[indice].nome, dados->aeroportos[indice].x, dados->aeroportos[indice].y);
			_stprintf_s(titulo, BUFFER, TEXT("Aeroporto %s"), dados->aeroportos[indice].nome);
			MessageBox(hWnd, texto, titulo, MB_OK);
		}
		return TRUE;
		break;
	case WM_INITDIALOG:
		for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
			SendDlgItemMessage(hWnd, IDC_LIST1, LB_ADDSTRING, 0, dados->aeroportos[i].nome);
		}
		//SendDlgItemMessage(hWnd, IDC_LIST1, LB_ADDSTRING, 0, TEXT("Porto"));
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK dCriarAeroportos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	TCHAR texto[BUFFER], titulo[BUFFER], nome[BUFFER], xT[BUFFER], yT[BUFFER];
	int indice;
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
				return FALSE;
			}
			//Acrescentar à lista de strings (list box)
			//SendDlgItemMessage(hWnd, IDC_LIST1, LB_ADDSTRING, 0, texto);
			//return TRUE;
		}
		//if (LOWORD(wParam) == IDC_LIST1 && HIWORD(wParam) == LBN_DBLCLK) {
		//	indice = SendDlgItemMessage(hWnd, IDC_LIST1, LB_GETCURSEL, 0, 0);
		//	_stprintf_s(texto, BUFFER, TEXT("Aeroporto %s nas coordenadas %d, %d."), dados->aeroportos[indice].nome, dados->aeroportos[indice].x, dados->aeroportos[indice].y);
		//	_stprintf_s(titulo, BUFFER, TEXT("Aeroporto %s"), dados->aeroportos[indice].nome);
		//	MessageBox(hWnd, texto, titulo, MB_OK);
		//}
		//return TRUE;
	case WM_INITDIALOG:
		//for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
		//	SendDlgItemMessage(hWnd, IDC_LIST1, LB_ADDSTRING, 0, dados->aeroportos[i].nome);
		//}
		//SendDlgItemMessage(hWnd, IDC_LIST1, LB_ADDSTRING, 0, TEXT("Porto"));
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
	TCHAR texto[BUFFER] = TEXT("A");
	int x=0, y=0;

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
	//case WM_LBUTTONDOWN:
	//	hdc = GetDC(hWnd);
	//	auxdc = CreateCompatibleDC(hdc);
	//	x = LOWORD(lParam);
	//	y = HIWORD(lParam);
	//	SelectObject(auxdc, bmpAviao);
	//	BitBlt(hdc, x, y, 134, 134, auxdc, 0, 0, SRCCOPY);
	//	BitBlt(double_dc, x, y, 134, 134, auxdc, 0, 0, SRCCOPY);
	//	DeleteDC(auxdc);
	//	ReleaseDC(hWnd, hdc);
	//	InvalidateRect(hWnd, NULL, TRUE);
	//	break;
	case WM_DESTROY:
		DeleteObject(bmpAviao);
		DeleteObject(bmpAeroporto);
		PostQuitMessage(0);
		break;
	//case WM_SIZE:
	//	GetWindowRect(hWnd, &paint.rcPaint);
	//	//_stprintf_s(texto, 100, TEXT("Dimensões da janela: largura-> %d altura -> %d"), paint.rcPaint.right, paint.rcPaint.bottom);
	//	//altura = paint.rcPaint.bottom / 2 - 134;
	//	x = paint.rcPaint.right / 2 - 134;
	//	InvalidateRect(hWnd, NULL, TRUE);
	//	//Calcular nova altura e largura
	//	//Gerar um evento WM_PAINT
	//	break;
	case WM_CHAR:
		//+ -> aumentar a velocidade (ou dimunir o tempo a parar entre cada ciclo do for)
		//- -> dimunir a velocidade (ou aumentaro tempo a parar entre cada ciclo do for)
		// espaço -> suspender/resumir a thread
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &paint);//GetDC(hWnd);
		auxdc = CreateCompatibleDC(hdc);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
				SelectObject(auxdc, bmpAeroporto);
		if (dados->ptr_memoria->naeroportos > 0) {
			for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
				BitBlt(hdc, dados->aeroportos[i].x, dados->aeroportos[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
				BitBlt(double_dc, dados->aeroportos[i].x, dados->aeroportos[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
				//TextOut(hdc, dados->aeroportos[i].x, dados->aeroportos[i].y, dados->aeroportos[i].nome, _tcslen(dados->aeroportos[i].nome));
			}
		}
		SelectObject(auxdc, bmpAviao);
		//BitBlt(hdc, x, y, 134, 134, auxdc, 0, 0, SRCCOPY);
		if (dados->ptr_memoria->navioes > 0) {
			for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
				BitBlt(hdc, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
				BitBlt(double_dc, dados->ptr_memoria->avioes[i].x, dados->ptr_memoria->avioes[i].y, 30, 30, auxdc, 0, 0, SRCCOPY);
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
            //if (LOWORD(wParam) == ID_SOBRE)
            //    MessageBox(hWnd, TEXT("Opção Sobre"), TEXT("Título"), MB_OK);
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
	case WM_ERASEBKGND:
		return (LRESULT)1;
	case  WM_MOUSEMOVE:
		if (!g_fMouseTracking && !hPop)
		{
			// start tracking if we aren't already
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(TRACKMOUSEEVENT);
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.hwndTrack = hWnd;
			tme.dwHoverTime = HOVER_DEFAULT;
			g_fMouseTracking = TrackMouseEvent(&tme);
		}
		TCHAR debug[100];
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		hdc = BeginPaint(hWnd, &paint);
		_stprintf_s(debug, 100, TEXT("%d %d"), x, y);
		TextOut(hdc, 0, 0, debug, _tcslen(debug));
		EndPaint(hWnd, &paint);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case  WM_MOUSEHOVER:
		g_fMouseTracking = FALSE; // tracking now cancelled

		if (!hPop)
		{

			if (x == dados->aeroportos[0].x && y == dados->aeroportos[0].y) {
				hPop = CreateWindowEx(WS_EX_STATICEDGE, //WS_EX_CLIENTEDGE,
					TEXT("STATIC"),
					TEXT("pop-up"),
					WS_POPUP | WS_BORDER,
					100, 100, 100, 100,
					hWnd, (HMENU)0, hInstance, NULL);
				ShowWindow(hPop, SW_SHOW);

				// set up leave tracking
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				g_fMouseTracking = TrackMouseEvent(&tme);
			}
		}
		return 0;

	case  WM_MOUSELEAVE:
		g_fMouseTracking = FALSE; // tracking now cancelled
		if (hPop)
		{
			// close popup if it's open
			ShowWindow(hPop, SW_HIDE);
			DestroyWindow(hPop);
			hPop = NULL;
		}
		return 0;

	default:
		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);
}

//int _tmain(int argc, TCHAR* argv[]) {
//	HANDLE semaforo_execucao, objMapMem, objMapMod, hThread;
//	HKEY chaveMAX = NULL;
//	DWORD result = 0, cbdata = sizeof(DWORD);
//	TCHAR cmd[BUFFER] = TEXT("");
//	TDados dados;
//#ifdef UNICODE 
//	if (_setmode(_fileno(stdin), _O_WTEXT) == -1) {
//		perror("Impossivel user _setmode()");
//	}
//	if (_setmode(_fileno(stdout), _O_WTEXT) == -1) {
//		perror("Impossivel user _setmode()");
//	}
//	if (_setmode(_fileno(stderr), _O_WTEXT) == -1) {
//		perror("Impossivel user _setmode()");
//	}
//#endif
//
//	// verifica se já está alguma instância em execução
//	semaforo_execucao = CreateSemaphore(NULL, 0, 1, SEMAFORO_CONTROLADOR);
//	result = GetLastError();
//	if (result == ERROR_ALREADY_EXISTS) {
//		_tprintf(TEXT("Já existe um controlador em execução.\nPor favor termine-o para iniciar um novo.\n"));
//		return -1;
//	}
//
//	// inicializar a memoria partilhada
//	objMapMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Memoria), MEMORIA);
//	if (objMapMem == NULL) {
//		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
//		return;
//	}
//	dados.ptr_memoria = (Memoria*)MapViewOfFile(objMapMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
//	if (dados.ptr_memoria == NULL) {
//		_tprintf(TEXT("Impossível criar vista de memória partilhada.\n"));
//		return;
//	}
//
//	// inicilizar a memoria partilhada do modelo produtor consumidor
//	objMapMod = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(Modelo), MODELO);
//	if (objMapMod == NULL) {
//		_tprintf(TEXT("Impossível criar objecto de mapping.\n"));
//		return;
//	}
//	dados.ptr_modelo = (Modelo*)MapViewOfFile(objMapMod, FILE_MAP_ALL_ACCESS, 0, 0, 0);
//	if (dados.ptr_modelo == NULL) {
//		_tprintf(TEXT("Impossivel criar vista de memória partilhada.\n"));
//		return;
//	}
//
//	// verifica se a chave abre
//	RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\temp\\SO2\\TP"), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, &chaveMAX);
//
//	// obter o numero maximo de aeroportos e avioes
//	result = RegQueryValueEx(chaveMAX, TEXT("MAXAE"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxaeroportos, (LPDWORD)&cbdata);
//	if (result != ERROR_SUCCESS) {
//		_tprintf(TEXT("\nNão foi possível ler do registo o número máximo de aeroportos.\nVai ser definido como 10.\n"));
//		dados.ptr_memoria->maxaeroportos = 10;
//	}
//	result = RegQueryValueEx(chaveMAX, TEXT("MAXAV"), NULL, NULL, (LPBYTE)&dados.ptr_memoria->maxavioes, (LPDWORD)&cbdata);
//	if (result != ERROR_SUCCESS) {
//		_tprintf(TEXT("\nNão foi possível ler do registo o número máximo de aviões.\nVai ser definido como 20.\n"));
//		dados.ptr_memoria->maxavioes = 10;
//	}
//
//	RegCloseKey(chaveMAX);
//
//	// inicializa semáforo para controlar instâncias de aviões
//	dados.sem_avioes = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_INSTANCIAS);
//
//	// inicializa os semáforos, mutex para o modelo produtor - consumidor
//	dados.sem_itens = CreateSemaphore(NULL, 0, dados.ptr_memoria->maxavioes, SEMAFORO_ITENS);
//	dados.sem_vazios = CreateSemaphore(NULL, dados.ptr_memoria->maxavioes, dados.ptr_memoria->maxavioes, SEMAFORO_VAZIOS);
//	dados.mutex = CreateMutex(NULL, FALSE, MUTEX_CONTROL);
//
//	// inicializar a condição de paragem a null
//	dados.ptr_memoria->terminar = false;
//	dados.ptr_memoria->navioes = 0;
//	dados.ptr_memoria->naeroportos = 0;
//
//	// DEBUG
//	//_tprintf(TEXT("NUMERO MAXIMO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->maxaeroportos);
//	//_tprintf(TEXT("NUMERO MAXIMO DE AVIOES: %ld\n"), dados.ptr_memoria->maxavioes);
//	//_tprintf(TEXT("NUMERO DE AEROPORTOS: %ld\n"), dados.ptr_memoria->naeroportos);
//	//_tprintf(TEXT("NUMERO DE AVIOES: %ld\n"), dados.ptr_memoria->navioes);
//
//
//	// inicializa array de aeroportos
//	dados.aeroportos = malloc(sizeof(Aeroporto) * dados.ptr_memoria->maxaeroportos);
//	memset(dados.aeroportos, 0, (size_t)dados.ptr_memoria->maxaeroportos * sizeof(Aeroporto));
//
//	/* DEBUG PARA SER MAIS FACIL UTILIZAR */
//	_tcscpy_s(dados.aeroportos[0].nome, BUFFER, TEXT("Lisboa"));
//	dados.aeroportos[0].x = 30;
//	dados.aeroportos[0].y = 40;
//	_tcscpy_s(dados.aeroportos[1].nome, BUFFER, TEXT("Porto"));
//	dados.aeroportos[1].x = 999;
//	dados.aeroportos[1].y = 999;
//	dados.ptr_memoria->naeroportos = 2;
//
//	// lança thread para controlar a entrada de aviões
//	hThread = CreateThread(NULL, 0, RecebeAvioes, &dados, 0, NULL);
//
//	// inicializa a thread para receber alertas
//	HANDLE hAlerta;
//	hAlerta = CreateThread(NULL, 0, RecebeAlerta, &dados, 0, NULL);
//
//	// inicializa suspensao de dados como falso
//	dados.suspend = false;
//
//	// imprimir menu
//	do {
//		_tprintf(TEXT("\nIntroduza a opção do comando que pretende executar: \n"));
//		_tprintf(TEXT("1. Criar aeroporto\n2. Suspender/Ativar registo de aviões\n3. Listar tudo\n"));
//		_tprintf(TEXT("\nOpção: "));
//		_fgetts(cmd, BUFFER, stdin);
//		if (_tcsicmp(cmd, TEXT("\n")) != 0) {
//			cmd[_tcslen(cmd) - 1] = '\0'; // retirar \n
//		}
//		else {
//			continue;
//		}
//		int cmdOpt = _tstoi(cmd);
//		switch (cmdOpt) {
//		case 1:
//			criaAeroporto(dados.aeroportos, &dados.ptr_memoria->naeroportos, dados.ptr_memoria->maxaeroportos);
//			break;
//		case 2:
//			HANDLE susThread;
//			if (!dados.suspend) {
//				susThread = CreateThread(NULL, 0, suspend, &dados, 0, NULL); // thread para ocupar todas as posições do semáforo
//				dados.suspend = true;
//				_tprintf(TEXT("\nRegisto de aviões suspensos.\n"));
//			}
//			else {
//				int release = 0;
//				if (dados.ptr_memoria->maxavioes - dados.ptr_memoria->navioes < 0) {
//					release = 0; // se todas as posições tiverem ocupadas antes da suspensão 
//				}
//				else {
//					release = dados.ptr_memoria->maxavioes - dados.ptr_memoria->navioes; // numero de posições a libertar
//				}
//				ReleaseSemaphore(dados.sem_avioes, release, NULL);
//				dados.suspend = false;
//				_tprintf(TEXT("\nRegisto de aviões ativo.\n"));
//			}
//			break;
//		case 3:
//			listaAeroportos(dados.aeroportos, dados.ptr_memoria->naeroportos);
//			listaAvioes(dados.ptr_memoria->avioes, dados.ptr_memoria->navioes);
//			break;
//		}
//	} while (_tcsicmp(cmd, TEXT("fim")) != 0);
//	
//	// accionar condição de paragem
//	dados.ptr_memoria->terminar = true;
//	WaitForSingleObject(hThread, 0);
//
//	UnmapViewOfFile(dados.ptr_memoria);
//	UnmapViewOfFile(dados.ptr_modelo);
//
//	return 0;
//
//}