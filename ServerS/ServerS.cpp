#include <iostream>
#include <WinSock2.h>
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
        if (recv(Connections[index], (char*)&msg_size, sizeof(int), NULL) != SOCKET_ERROR)
        {
            char* msg = new char[msg_size + 1];
            msg[msg_size] = '\0';
            if (recv(Connections[index], msg, msg_size, NULL) != SOCKET_ERROR)
            {
                for (int i = 0; i < Counter; i++)
                {
                    if (i == index)
                        continue;

                    Packet msgtype = P_ChatMessage;
                    send(Connections[i], (char*)&msgtype, sizeof(Packet), NULL);
                    send(Connections[i], (char*)&msg_size, sizeof(int), NULL);
                    send(Connections[i], msg, msg_size, NULL);
                }
            }
            delete[] msg;
        }
        break;
    }
    default:
        cout << "Unrecognized packet: " << packettype << endl;
    }
    return true;
}

void ClientHandler(int index)
{
    Packet packettype;
    while (true)
    {
        if (recv(Connections[index], (char*)&packettype, sizeof(Packet), NULL) != SOCKET_ERROR)
        {
            if (!ProcessPacket(index, packettype))
                break;
        }
        else
        {
            int error = WSAGetLastError();
            if (error == WSAECONNRESET)
            {
                cout << "Client disconnected." << endl;
            }
            else
            {
                cout << "Error receiving packet from client. Error code: " << error << endl;
            }
            break;
        }
    }
    closesocket(Connections[index]);
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
    SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sListen == INVALID_SOCKET)
    {
        cout << "Failed to create socket. Error code: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = INADDR_ANY;  
    addr.sin_port = htons(1234);  
    addr.sin_family = AF_INET;
    if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        cout << "Failed to bind socket. Error code: " << WSAGetLastError() << endl;
        closesocket(sListen);
        WSACleanup();
        return 1;
    }
    if (listen(sListen, SOMAXCONN) == SOCKET_ERROR)
    {
        cout << "Failed to listen on socket. Error code: " << WSAGetLastError() << endl;
        closesocket(sListen);
        WSACleanup();
        return 1;
    }

    cout << "Server started. Listening for incoming connections..." << endl;

    while (true)
    {
        SOCKET newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);
        if (newConnection == INVALID_SOCKET)
        {
            cout << "Failed to accept connection. Error code: " << WSAGetLastError() << endl;
            continue;
        }
        Connections[Counter] = newConnection;
        Counter++;
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr.sin_addr), clientIP, INET_ADDRSTRLEN);
        cout << "Client connected. IP: " << clientIP << endl;
        string msg = "Welcome to the server!";
        int msg_size = msg.size();

        Packet msgtype = P_ChatMessage;
        send(newConnection, (char*)&msgtype, sizeof(Packet), NULL);
        send(newConnection, (char*)&msg_size, sizeof(int), NULL);
        send(newConnection, msg.c_str(), msg_size, NULL);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandler, (LPVOID)(Counter - 1), NULL, NULL);
        Packet testpacket = P_Test;
        send(newConnection, (char*)&testpacket, sizeof(Packet), NULL);
    }

    closesocket(sListen);
    WSACleanup();

    return 0;
}
