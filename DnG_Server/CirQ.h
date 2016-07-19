#pragma once
#include "stdafx.h"

struct BroadcastMessage {
	SOCKET socket;
	char data[15];
};

class CirQ
{
private:
	BroadcastMessage message[105];
	int front;
	int back;
	bool empty;
public:
	CirQ();
	CirQ(const CirQ& CQ) = delete;
	virtual ~CirQ();

	int PopFront(BroadcastMessage* pBM);
	int PushBack(SOCKET socket, char data[15]);
};

