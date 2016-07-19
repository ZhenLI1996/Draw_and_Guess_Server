#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "stdafx.h"
#include "CirQ.h"

using namespace std;

//unsigned long __stdcall echo(void *);

set<SOCKET> client_set;

const char SUCCESS = 1, FAIL = 0;


HANDLE event;
CRITICAL_SECTION ServLock;
CRITICAL_SECTION BdcLock;

int DrawerNum = 0;
wchar_t* GuessAnswer = L"≤‚ ‘";

vector<SOCKET> clt_vec;
//CirQ BdcMsg;
char* BdcData = nullptr;

bool quit = false;


HANDLE hSema = CreateSemaphore(0, 0, 100, 0);


unsigned long _stdcall Broadcast(void* data);
unsigned long _stdcall Serv(void* data);	// data is a pointer to this SOCKET 

int DrawLine(SOCKET);
int Guess(SOCKET);


int _tmain(int argc, _TCHAR* argv[]){
	// ======== copy ========
	ios_base::sync_with_stdio(false);
	WSADATA wsaData;
	sockaddr_in addr;
	SOCKET svr, clt;
	char ip[15];
	
	event = CreateEvent(0, 0, 0, 0);
	InitializeCriticalSection(&ServLock);
	InitializeCriticalSection(&BdcLock);
	cout << "Server IP Addr:\n";
	cin >> ip;
	if ('0' == ip[0]) {
		strcpy_s(ip, "192.168.20.103");
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2225);
	addr.sin_addr.S_un.S_addr = inet_addr(ip);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData)){
		cout << "Socket startup failed.\n";
		return 1;
	}

	svr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (svr == INVALID_SOCKET){
		cout << "Invalid socket.\n";
		return 1;
	}

	if (bind(svr, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in))){
		cout << "bind failed " << GetLastError() << "\n";
		return 1;
	}

	if (listen(svr, SOMAXCONN)){
		cout << "unable to listen\n";
		return 1;
	}

	// ======== end copy =========

	// create a threads for broadcast
	CreateThread(0, 0, Broadcast, 0, 0, 0);
	

	while (clt = accept(svr, 0, 0)){
		cout << "New Client connected.\n";
		clt_vec.push_back(clt);
		CreateThread(0, 0, Serv, static_cast<void*>(&(clt_vec.back())), 0, 0);
	}

	// ======== end ========

	closesocket(svr);
	client_set.clear();
	WSACleanup();
	return 0;
}


unsigned long _stdcall Broadcast(void* data) {
	/*
	while (!quit && !WaitForSingleObject(hSema, INFINITE)) {
		
		EnterCriticalSection(&BdcLock);
		//cout << "Broadcast:\t" << *reinterpret_cast<UINT*>(tBM.data) << ", " << *reinterpret_cast<LPARAM*>(tBM.data + 4) << '\n';
		for (auto iter = clt_vec.begin(); iter != clt_vec.end(); iter++) {
			if ((*iter) == tBM.socket)	continue;
			send((*iter), tBM.data, 15, 0);
		}
		LeaveCriticalSection(&BdcLock);
	}

	quit = true;
	*/
	return 0;
}

unsigned long _stdcall Serv(void* data) {

	SOCKET clt = *reinterpret_cast<SOCKET*>(data);
	char* send_buf = new char[3];
	send_buf[2] = 0;
	int t = 0;
	while (!quit && (t = recv(clt, send_buf, 2, 0)) > 0) {
		cout << "SEND_BUF: " << send_buf << '\n';
		if (!strcmp(send_buf, MY_SEND_DRAW_LINE)) {
			if (DrawLine(clt)) break;
		}
		else if (!strcmp(send_buf, MY_SEND_GUESS)) {
			if (Guess(clt)) break;
			}
		else  {
			// !!!HERE
		}
	}

	EnterCriticalSection(&ServLock);
	for (auto iter = clt_vec.begin(); iter != clt_vec.end(); iter++) {
		if ((*iter) == clt) {
			clt_vec.erase(iter);
			break;
		}
	}
	LeaveCriticalSection(&ServLock);

	cout << "Client Disconnect\n";

	closesocket(clt);
	delete[] send_buf;

	return 0;
}

int DrawLine(SOCKET clt) {
	cout << "DrawLine:\n";
	char* size_buf = new char[5];
	size_buf[4] = 0;
	if (recv(clt, size_buf, 4, 0) <= 0) return 1;
	//cout << "DrawerNum: " << DrawerNum << '\n';
	if (clt == clt_vec[DrawerNum]) {
		unsigned int size = *reinterpret_cast<unsigned int*>(size_buf);
		cout << size << '\n';
		char* buffer = new char[size + 1];

		char* current_pos = buffer;
		int current_size = 0;
		while (current_size < size) {
			int fragment_size = recv(clt, current_pos, size - current_size, 0);
			if (fragment_size <= 0) {
				break;
			}
			current_size += fragment_size;
			current_pos += fragment_size;
			cout << fragment_size << endl;
		}

		for (auto iter = clt_vec.begin(); iter != clt_vec.end(); iter++) {
			if ((*iter) == clt)	continue;
			cout << "HERE\n";
			send((*iter), MY_SEND_DRAW_LINE, 2, 0);
			send((*iter), size_buf, 4, 0);
			send((*iter), buffer, size, 0);
		}
		delete[] buffer;
	}
	delete[] size_buf;
	return 0;
}

int Guess(SOCKET clt) {
	cout << "Guess:\n";
	char* size_buf = new char[5];
	size_buf[4] = 0;
	if (recv(clt, size_buf, 4, 0) <= 0) return 1;

	unsigned int size = *reinterpret_cast<unsigned int*>(size_buf);
	cout << "Size: " << size << '\n';
	char* buffer = new char[size + 3];
	wchar_t* GuessStr = reinterpret_cast<wchar_t*>(buffer);

	int fragment_size;
	if ((fragment_size = recv(clt, buffer, size, false)) <= 0) return 1;
	cout << "fragment_size: " << fragment_size << '\n';
	//cout << "BEFORE wcout \n";
	wcout << GuessStr << endl;
	// Chinese characters received are right but cannot wcout1
	//cout << "AFTER wcout \n";
	if (!wcscmp(GuessStr, GuessAnswer)) {
		// Guess Right
		cout << "SOCKET: " << clt << ", " << "Guess Right\n";
	}
	else {
		// Guess Wrong
		cout << "SOCKET: " << clt << ", " << "Guess Wrong\n";
	}


	delete[] buffer;
	delete[] size_buf;
	return 0;

}