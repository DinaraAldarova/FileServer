#include "ServerInterLayer.h"
#pragma comment( lib, "ws2_32.lib" )
class ServerInterLayer
{
	ServerInterLayer::ServerInterLayer()
	{
		init();
	}
	DWORD WINAPI ServerInterLayer::initialize(LPVOID param)
	{
		InitializeCriticalSection(&cs_info);
		if (WSAStartup(0x202, (WSADATA *)&buff[0]))
		{
			status = s::error;
			return 0;
		}
		SOCKET server_socket;
		sockaddr_in local_addr;
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (server_socket < 0)
		{
			WSACleanup();
			status = s::error;
			return 0;
		}
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(port);
		local_addr.sin_addr.s_addr = 0;
		if (bind(server_socket, (sockaddr *)&local_addr, sizeof(local_addr)))
		{
			closesocket(server_socket);
			WSACleanup();
			status = s::error;
			return 0;
		}
		//ќжидание подключений
		if (listen(server_socket, 20))
		{
			closesocket(server_socket);
			WSACleanup();
			status = s::error;
			return 0;
		}
		EnterCriticalSection(&cs_info);
		info host_info;
		host_info.ID = 0;
		WSADATA wsaData;
		WSAStartup(MAKEWORD(1, 1), &wsaData); // инициализируем socket'ы использу€ Ws2_32.dll дл€ процесса

		char HostName[1024]; // создаем буфер дл€ имени хоста
		DWORD m_HostIP = 0;
		LPHOSTENT lphost;

		if (!gethostname(HostName, 1024)) // получаем им€ хоста
		{
			if (lphost = gethostbyname(HostName)) // получаем IP хоста, т.е. нашего компа
				m_HostIP = ((LPIN_ADDR)lphost->h_addr)->s_addr; // преобразуем переменную типа LPIN_ADDR в DWORD
		}
		host_info.name = string(HostName);
		host_info.IPv4 = inet_ntoa(*((in_addr*)lphost->h_addr_list[0]));
		LeaveCriticalSection(&cs_info);
		WSACleanup(); // освобождаем сокеты, т.е. завершаем использование Ws2_32.dll

		int client_addr_size = sizeof(client_addr);
		//»звлечение запросов на подключение из очереди
		while ((client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_addr_size)))
		{
			info client;
			client.ID = new_ID();

			hst = gethostbyaddr((char *)&client_addr.sin_addr.s_addr, 4, AF_INET);
			client.name = (hst) ? hst->h_name : "";
			client.IPv4 = inet_ntoa(client_addr.sin_addr);
			client.sock = &client_socket;
			DWORD thID;
			client_info.push_back(client);
			client.stream = CreateThread(NULL, NULL, WorkWithClient, &client_info.back(), NULL, &thID);
		}

		return 0;
	}
	void ServerInterLayer::init()
	{
		DWORD thID;
		CreateThread(NULL, NULL, initialize, NULL, NULL, &thID);
	}

	int ServerInterLayer::new_ID()
	{
		EnterCriticalSection(&cs_info);
		int a = client_info.back().ID + 1;
		LeaveCriticalSection(&cs_info);
		return a;
	}

	s ServerInterLayer::Status()
	{
		return status;
	}

	int ServerInterLayer::Exit()
	{
		return 0;
	}

	DWORD WINAPI ServerInterLayer::WorkWithClient(LPVOID param)
	{
		info* c_client = (info*)param;
		info client = *c_client;

		return 0;
	}

	void ServerInterLayer::quit_client(int ID)
	{

	}
};