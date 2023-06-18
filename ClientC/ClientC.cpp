#include <iostream>
#include <WinSock2.h>
#include <string>
#define _WINSOCK__DEPRECATED_NO_WARNINGS
//#include "stdafx.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;
#pragma warning(disable: 4996)
SOCKET Connection;
enum Packet
{
	P_ChatMessage,
	P_Test
};
bool ProcessPacket(Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
		{
		int msg_size;
		recv(Connection, (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connection, msg, msg_size, NULL);
		cout << msg << endl;
		delete[] msg;
		break;
		}
	case P_Test:
	{
		cout << "Test packet" << endl;
		break;
	}
	default:
		cout << "Unrecognized! packet" << packettype << endl;
	}
	return true;
}
void ClientHandler()
{
	Packet packettype;
	while (true)
	{
		recv(Connection, (char*)&packettype, sizeof(Packet), NULL);
		if (!ProcessPacket(packettype))
		{
			break;
		}
	}
	closesocket(Connection);
}
int main()
{
	//WSASrartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		cout << "Error!" << endl;
		exit(1);
	}
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;
	Connection = socket(AF_INET, SOCK_STREAM, NULL);
	if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) != 0)
	{
		cout << "Failed to connect!" << endl;
		return 1;
	}
	else
	{
		cout << "Connected!" << endl; 
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);
		string msg1;
		while (true)
		{
			getline(cin, msg1);
			int msg_size = msg1.size();
			Packet packettype = P_ChatMessage;
			send(Connection, (char*)&packettype, sizeof(Packet), NULL);
			send(Connection, (char*)&msg_size, sizeof(int), NULL);
			send(Connection, msg1.c_str(), msg_size, NULL);
			Sleep(10);
		}
	}
	system("pause");
	return 0;
}