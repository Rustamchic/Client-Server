#include <iostream>
#include <WinSock2.h>
//#include "stdafx.h"
#define _WINSOCK__DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
using namespace std;
#pragma warning(disable: 4996)

SOCKET Connections[100];
int Counter = 0;
enum Packet
{
	P_ChatMessage,
	P_Test
};
bool ProcessPacket(int index, Packet packettype)
{
	switch (packettype)
	{
	case P_ChatMessage:
	{
		int msg_size;
		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL);
		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';
		recv(Connections[index], msg, msg_size, NULL);
		for (int i = 0; i < Counter; i++)
		{
			if (i == index)
			{
				continue;
			}
			Packet msgtype = P_ChatMessage;
			send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
			send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
			send(Connections[i], msg, msg_size, NULL);
			delete[] msg;
			break;
		}
	}
	break;
	default:
		cout << "Unrecognized! packet: " << packettype << endl;
	}
	return true;
}
void ClientHandler(int index)
{
	Packet packettype;
	while (true)
	{
		recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL);
		if (!ProcessPacket(index, packettype))
		{
			break;
		}
	}
	closesocket(Connections[index]);
}

int main()
{
	// WSASrartup
	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	if (WSAStartup(DLLVersion, &wsaData) != 0)
	{
		cout << "Error!" << endl;
		exit(1);
	}
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);
	SOCKET newConnection;
	cout << "Server started. Listening for incoming connections..." << endl;
	while (true)
	{
		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr); // возвращает указатель на новый сокет(для общения с клиентом)
		if (newConnection == 0)
		{
			cout << "Error!" << endl;
		}
		else
		{
			char clientIP[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(addr.sin_addr), clientIP, INET_ADDRSTRLEN);
			cout << "Client connected. IP: " << clientIP << endl;

			// Get current time
			time_t now = time(0);
			char* dateTime = ctime(&now);

			string msg = "Connected at ";
			msg += dateTime;
			int msg_size = msg.size();

			Packet msgtype = P_ChatMessage;
			send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
			send(newConnection, (char*)&msg_size, sizeof(int), NULL);
			send(newConnection, msg.c_str(), sizeof(msg), NULL);
			Connections[Counter] = newConnection;
			Counter++;
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(Counter - 1), NULL, NULL);
			Packet testpacket = P_Test;
			send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
		}
	}
	return 0;
}
