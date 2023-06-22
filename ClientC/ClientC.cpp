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
        cout << "Unrecognized packet: " << packettype << endl;
    }
    return true;
}

void ClientHandler()
{
    Packet packettype;
    while (true)
    {
        if (recv(Connection, (char*)&packettype, sizeof(Packet), NULL) != SOCKET_ERROR)
        {
            if (!ProcessPacket(packettype))
                break;
        }
        else
        {
            int error = WSAGetLastError();
            if (error == WSAECONNRESET)
            {
                cout << "Server closed the connection." << endl;
                break;
            }
            else
            {
                cout << "Error receiving packet. Error code: " << error << endl;
                break;
            }
        }
    }
    closesocket(Connection);
}

int main()
{
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 2);
    if (WSAStartup(DLLVersion, &wsaData) != 0)
    {
        cout << "Failed to initialize winsock." << endl;
        return 1;
    }
    SOCKET Connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Connection == INVALID_SOCKET)
    {
        cout << "Failed to create socket. Error code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    addr.sin_port = htons(1234); 
    addr.sin_family = AF_INET;
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cout << "Failed to connect to server. Error code: " << WSAGetLastError() << endl;
        closesocket(Connection);
        WSACleanup();
        return 1;
    }

    cout << "Connected to the server!" << endl;

    CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, NULL, NULL, NULL);

    string msg;
    while (true)
    {
        getline(cin, msg);
        int msg_size = msg.size();
        Packet packettype = P_ChatMessage;
        send(Connection, (char*)&packettype, sizeof(Packet), NULL);
        send(Connection, (char*)&msg_size, sizeof(int), NULL);
        send(Connection, msg.c_str(), msg_size, NULL);
        Sleep(10);
    }

    closesocket(Connection);
    WSACleanup();

    return 0;
}
