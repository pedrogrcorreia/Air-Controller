//#include <windows.h>
//#include <tchar.h>
//#include <math.h>
//#include <stdio.h>
//#include <fcntl.h> 
//#include <io.h>
//#include <stdbool.h>
//#include <strsafe.h>
//#include "resource.h"
//#include "../util.h"
//
//#define BUFSIZE 2048
//#define Ps_Sz sizeof(Passageiro)
//
//#define Msg_Sz sizeof(Msg)
//
//void listaAeroportos(Aeroporto aeroportos[], int nae) {
//	_tprintf(TEXT("\nExistem %d aeroportos.\n"), nae);
//	for (int i = 0; i < nae; i++) {
//		_tprintf(TEXT("\nAeroporto %s, localizado em %d, %d.\n"), aeroportos[i].nome, aeroportos[i].x, aeroportos[i].y);
//	}
//}
//
//void listaAvioes(Aviao avioes[], int nav) {
//	_tprintf(TEXT("\nExistem %d aviões.\n"), nav);
//	for (int i = 0; i < nav; i++) {
//		_tprintf(TEXT("\nAvião %d na posição %d, %d.\n"), avioes[i].id, avioes[i].x, avioes[i].y);
//		if (avioes[i].setDestino) {
//			_tprintf(TEXT("\nAvião %d com destino a %s em %d %d.\n"), avioes[i].id, avioes[i].destino.nome, avioes[i].destino.x, avioes[i].destino.y);
//		}
//	}
//}
//
//DWORD WINAPI suspend(LPVOID param) {
//	TDados** dados = (TDados**)param;
//	for (int i = (*dados)->ptr_memoria->navioes; i < (*dados)->ptr_memoria->maxavioes; i++) {
//		WaitForSingleObject((*dados)->sem_avioes, INFINITE);
//	}
//	return 0;
//}
//
//void iniciaPassageiros(TDados* dados) {
//	int i;
//	for (i = 0; i < MAXPASSAG; i++) {
//		dados->p[i].hPipe = NULL;
//	}
//}
//
//void adicionaPassageiro(TDados* dados, HANDLE pass) {
//	int i;
//	for (i = 0; i < MAXPASSAG; i++) {
//		if (dados->p[i].hPipe == NULL) {
//			dados->p[i].hPipe = pass;
//			dados->numpassag++;
//			return;
//		}
//	}
//}
//
//void removePassageiro(TDados* dados, HANDLE pass) {
//	int i;
//	for (i = 0; i < MAXPASSAG; i++) {
//		if (dados->p[i].hPipe == pass) {
//			dados->p[i].hPipe = NULL;
//			dados->numpassag--;
//			return;
//		}
//	}
//}
//
//void registaPassageiro(TDados* dados, Passageiro p) {
//	for (int i = 0; i < MAXPASSAG; i++) {
//		if (dados->p[i].hPipe == p.hPipe) {
//			_tcscpy_s(dados->p[i].inicial, BUFFER, p.inicial);
//			_tcscpy_s(dados->p[i].destino, BUFFER, p.destino);
//			return;
//		}
//	}
//}
//
//void embarcaPassageiro(TDados* dados, Aviao a){
//	for (int i = 0; i < dados->numpassag; i++) {
//		if (_tcsicmp(dados->p[i].inicial, a.inicial.nome) == 0 && _tcsicmp(dados->p[i].destino, a.destino.nome) == 0) {
//			//_tprintf(TEXT("%s %s"), dados->p[i].inicial, a.inicial.nome);
//			//_tprintf(TEXT("%s %s"), dados->p[i].destino, a.destino.nome);
//			dados->p[i].voo = a.id;
//			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Embarcar"));
//			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
//			return; // falta meter a lotacao
//		}
//	}
//}
//
//void movePassageiro(TDados* dados, Aviao a) {
//	for (int i = 0; i < dados->numpassag; i++) {
//		if (dados->p[i].voo == a.id) {
//			dados->p[i].x = a.x;
//			dados->p[i].y = a.y;
//			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Viajar"));
//			//writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
//			return;
//		}
//	}
//}
//
//void avisaChegada(TDados* dados, Aviao a) {
//	for (int i = 0; i < dados->numpassag; i++) {
//		if (dados->p[i].voo == a.id) {
//			_tcscpy_s(dados->p[i].mensagem, BUFFER, TEXT("Chegada"));
//			writePassageiroASINC(dados->p[i].hPipe, dados->p[i]);
//			return;
//		}
//	}
//}
//
//HANDLE WriteReady;
//
//int writePassageiroASINC(HANDLE hPipe, Passageiro p) {
//	DWORD cbWritten = 0;
//	BOOL fSuccess = FALSE;
//
//	OVERLAPPED OverlWr = { 0 };
//
//	ZeroMemory(&OverlWr, sizeof(OverlWr));
//	ResetEvent(WriteReady);
//	OverlWr.hEvent = WriteReady;
//
//	fSuccess = WriteFile(hPipe, &p, Ps_Sz, &cbWritten, &OverlWr);
//
//	WaitForSingleObject(WriteReady, INFINITE);
//	GetOverlappedResult(hPipe, &OverlWr, &cbWritten, FALSE);
//	return 1;
//}
//
//
//int broadcastPassageiros(TDados* dados, Passageiro p) {
//	int i, numwrites = 0;
//	for (i = 0; i < MAXPASSAG; i++) {
//		if (dados->p[i].hPipe != 0) {
//			numwrites += writePassageiroASINC(dados->p[i].hPipe, p);
//		}
//	}
//	return numwrites;
//}
//
//DWORD WINAPI PassagThread(LPVOID param) {
//	Passageiro Recebido, Enviado;
//	DWORD cbBytesRead = 0, cbReplyBytes = 0;
//	int numresp = 0;
//	BOOL fSuccess = FALSE;
//	TDados* dados = (TDados*)param;
//	HANDLE hPipe = dados->cPipe;
//	HANDLE ReadReady;
//	OVERLAPPED OverlRd = { 0 };
//	ReadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//	adicionaPassageiro(dados, hPipe);
//
//	while (1) {
//		ZeroMemory(&OverlRd, sizeof(OverlRd));
//		ResetEvent(ReadReady);
//		OverlRd.hEvent = ReadReady;
//
//		fSuccess = ReadFile(hPipe, &Recebido, Ps_Sz, &cbBytesRead, &OverlRd);
//
//		WaitForSingleObject(ReadReady, INFINITE);
//
//		GetOverlappedResult(hPipe, &OverlRd, &cbBytesRead, FALSE);
//		_tprintf(TEXT("Passageiro definiu aeroportos %s %s\n"), Recebido.inicial, Recebido.destino);
//		for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
//			if (_tcsicmp(Recebido.inicial, dados->aeroportos[i].nome) == 0) {
//				_tcscpy_s(Enviado.inicial, BUFFER, Recebido.inicial);
//				Enviado.termina = false;
//			}
//			else {
//				_tcscpy_s(Enviado.mensagem, BUFFER, TEXT("O aeroporto inicial não existe. O programa vai terminar."));
//				Enviado.termina = true;
//			}
//		}
//		for (int i = 0; i < dados->ptr_memoria->naeroportos; i++) {
//			if (_tcsicmp(Recebido.destino, dados->aeroportos[i].nome) == 0) {
//				_tcscpy_s(Enviado.destino, BUFFER, Recebido.destino);
//				Enviado.termina = false;
//			}
//			else {
//				_tcscpy_s(Enviado.mensagem, BUFFER, TEXT("O aeroporto de destino não existe. O programa vai terminar."));
//				Enviado.termina = true;
//			}
//		}
//
//		if (!Enviado.termina) {
//			Enviado.hPipe = hPipe;
//			registaPassageiro(dados, Enviado);
//			_tcscpy_s(Enviado.mensagem, BUFFER, TEXT("Avião no aeroporto inicial."));
//		}
//		numresp = broadcastPassageiros(dados, Enviado);
//		//Enviado.termina = false;
//		if (Enviado.termina == true || Recebido.termina == true) {
//			break;
//		}
//		//_tprintf(TEXT("Servidor: %d respostas enviadas\n"), numresp);
//	}
//
//	removePassageiro(dados, hPipe);
//	FlushFileBuffers(hPipe);
//	DisconnectNamedPipe(hPipe);
//	CloseHandle(hPipe);
//
//	_tprintf(TEXT("Thread a terminar"));
//	return 1;
//}
//
//DWORD WINAPI RecebePassageiros(LPVOID param) {
//	TDados* dados = (TDados*)param;
//	BOOL fConnected = FALSE;
//	DWORD dwThreadId = 0;
//	HANDLE hThread = NULL;
//	HANDLE hPipe;
//	WriteReady = CreateEvent(NULL, TRUE, FALSE, NULL);
//
//	iniciaPassageiros(dados);
//	while (1) {
//		hPipe = CreateNamedPipe(PIPE_CONTROL, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, BUFSIZE, BUFSIZE, 5000, NULL);
//		_tprintf(TEXT("\nServidor a aguardar por clientes\n"));
//		dados->cPipe = hPipe;
//		fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
//
//		if (fConnected) {
//			hThread = CreateThread(NULL, 0, PassagThread, (LPVOID)dados, 0, &dwThreadId);			
//			if (hThread == NULL) {
//				return -1;
//			}
//			else {
//				CloseHandle(hThread);
//			}
//		}
//		else {
//			CloseHandle(hPipe);
//		}
//	}
//	return 0;
//}
//
//DWORD WINAPI RecebeAvioes(LPVOID param) {
//	TDados* dados = (TDados*)param;
//	Aviao aviao;
//
//	while (!dados->ptr_memoria->terminar) {
//		bool novoAviao = true;
//		int pos = -1;
//		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR inicio
//
//		// esperar semáforo dos aviões
//		WaitForSingleObject(dados->sem_itens, INFINITE);
//
//		// copiar o aviao recebido
//		CopyMemory(&aviao, &dados->ptr_modelo->avioesBuffer[dados->ptr_modelo->entAviao], sizeof(Aviao));
//
//		for (int i = 0; i <= dados->ptr_memoria->navioes; i++) {
//			if (aviao.id == dados->ptr_memoria->avioes[i].id) {
//				novoAviao = false;
//				pos = i;
//				//CopyMemory(&dados->ptr_memoria->avioes[i], &aviao, sizeof(Aviao));
//			}
//		}
//
//		HANDLE eventoComandos; // evento para lidar com a definição de um destino
//		eventoComandos = CreateEvent(NULL, FALSE, FALSE, EVENTO_COMANDOS);
//
//		if (novoAviao) { // é um novo aviao
//
//			// criar o evento de alerta para cada avião
//			TCHAR alerta[BUFFER];
//			_stprintf_s(alerta, BUFFER, TEXT("alerta %d"), aviao.id);
//			dados->eventos[dados->ptr_memoria->navioes] = CreateEvent(NULL, FALSE, FALSE, alerta);
//
//			if (checkNome(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos)) {
//				aviao.x = -1;
//				aviao.y = -1;
//				aviao.terminar = true;
//				CopyMemory(&dados->ptr_memoria->avioes[dados->ptr_memoria->navioes], &aviao, sizeof(Aviao)); // acrescenta o novo avião 
//				dados->ptr_memoria->navioes++; // incrementa o numero de aviões
//			}
//			else {
//				aviao.inicial = getAeroporto(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
//				aviao.x = getX(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
//				aviao.y = getY(aviao.inicial.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
//				CopyMemory(&dados->ptr_memoria->avioes[dados->ptr_memoria->navioes], &aviao, sizeof(Aviao)); // acrescenta o avião
//				dados->ptr_memoria->navioes++; // incrementa o número de aviões
//			}
//		}
//		else {
//			if (aviao.setDestino) { // esta a definir um novo destino
//				if (checkNome(aviao.destino.nome, dados->aeroportos, dados->ptr_memoria->naeroportos)) {
//					aviao.destino.x = -1;
//					aviao.destino.y = -1;
//					aviao.setDestino = false;
//					_tcscpy_s(aviao.destino.nome, BUFFER, TEXT(""));
//					CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
//				}
//				else {
//					aviao.destino = getAeroporto(aviao.destino.nome, dados->aeroportos, dados->ptr_memoria->naeroportos);
//					aviao.setDestino = true;
//					CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
//				}
//				SetEvent(eventoComandos);
//			}
//			if (aviao.embarcar) { // embarcar passageiros
//				//Passageiro p;
//				//_tcscpy_s(p.mensagem, BUFFER, TEXT("Embarcar"));
//				//_tcscpy_s(p.inicial, BUFFER, aviao.inicial.nome);
//				//_tcscpy_s(p.destino, BUFFER, aviao.destino.nome);
//				//p.voo = aviao.id;
//				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
//				embarcaPassageiro(dados, aviao);
//				SetEvent(eventoComandos);
//			}
//			if (aviao.viajar) { // aviao está em movimento
//				//Passageiro p;
//				//_tcscpy_s(p.mensagem, BUFFER, TEXT("Viajar"));
//				//_tcscpy_s(p.inicial, BUFFER, aviao.inicial.nome);
//				//_tcscpy_s(p.destino, BUFFER, aviao.destino.nome);
//				//p.x = aviao.x;
//				//p.y = aviao.y;
//				//p.voo = aviao.id;
//				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
//				movePassageiro(dados, aviao);
//				//broadcastPassageiros(dados, p);
//			}
//			if (aviao.terminarViagem) { // aviao chegou
//				Passageiro p;
//				_tcscpy_s(p.mensagem, BUFFER, TEXT("Chegada"));
//				_tcscpy_s(p.destino, BUFFER, aviao.destino.nome);
//				_tprintf(TEXT("O avião %d chegou ao Aeroporto de destino.\n"), aviao.id);
//				aviao.inicial = aviao.destino;
//				CopyMemory(&dados->ptr_memoria->avioes[pos], &aviao, sizeof(Aviao));
//				avisaChegada(dados, aviao);
//			}
//		}
//		//InvalidateRect(dados->hWnd, NULL, TRUE);
//		dados->ptr_modelo->entAviao = (dados->ptr_modelo->entAviao + 1) % TAM; // incrementar a posicao de leitura
//		// assinala semáforo
//		ReleaseSemaphore(dados->sem_vazios, 1, NULL);
//		// MODELO CONSUMIDOR ------ N PRODUTORES 1 CONSUMIDOR fim
//
//		//_tprintf(TEXT("Aviao %d na posicao %d, %d.\n"), aviao.id, aviao.x, aviao.y);
//	}
//	return 0;
//}
//
//DWORD WINAPI RecebeAlerta(LPVOID param) {
//	TDados* dados = (TDados*)param;
//	DWORD result;
//	TCHAR alerta[BUFFER];
//	while (!dados->ptr_memoria->terminar) {
//		if (dados->ptr_memoria->navioes > 0) {
//			for (int i = 0; i < dados->ptr_memoria->navioes; i++) {
//				result = WaitForSingleObject(dados->eventos[i], 1);
//				if (result == WAIT_TIMEOUT) {
//					_tprintf(TEXT("Avião %d desligou-se.\n"), dados->ptr_memoria->avioes[i].id); // se não chegar é porque o avião se desligou
//					dados->ptr_memoria->avioes[i] = dados->ptr_memoria->avioes[i + 1]; // retira o avião
//					dados->ptr_memoria->navioes--; // decrementa o numero de aviões
//					ReleaseSemaphore(dados->sem_avioes, 1, NULL); // retira o aviao do semáforo
//					InvalidateRect(dados->hWnd, NULL, TRUE);
//				}
//			}
//		}
//		Sleep(3000);
//	}
//	return 0;
//}
//
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
//
//	dados.numpassag = 0;
//	//inicializa a thread para receber passageiros
//	HANDLE hPassag;
//	hPassag = CreateThread(NULL, 0, RecebePassageiros, &dados, 0, NULL);
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
//		case 4:
//			for (int i = 0; i < dados.numpassag; i++) {
//				_tprintf(TEXT("%s %s"), dados.p[i].inicial, dados.p[i].destino);
//			}
//			break;
//		}
//			
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