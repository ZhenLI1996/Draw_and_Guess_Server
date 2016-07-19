#include "CirQ.h"

CirQ::CirQ() :
front(0),
back(0),
empty(true)
{
}

CirQ::~CirQ()
{
}

int CirQ::PopFront(BroadcastMessage* pDst) {
	//if (empty)	return 1;
	pDst->socket = message[front].socket;
	for (int i = 0; i < 15; i++)
		pDst->data[i] = message[front].data[i];
	front++;
	if (front >= 100) front -= 100;
	if (front == back) empty = true;
	return 0;
}
int CirQ::PushBack(SOCKET socket, char data[15]) {
	//if (!empty && front == back)	return 1;
	message[back].socket = socket;
	for (int i = 0; i < 15; i++)
		message[back].data[i] = data[i];
	back++;
	if (back < 0) back += 100;
	empty = false;
	return 0;

}