#include <Windows.h>
#include "ServerInterLayer.h"
#pragma comment( lib, "ws2_32.lib" )
ServerInterLayer * server;
ServerInterLayer::ServerInterLayer()
{
	InitializeCriticalSection(&cs_info);
	hMutex_Log = CreateMutex(NULL, false, NULL);
	hMutex_Users = CreateMutex(NULL, false, NULL);
	hMutex_Files = CreateMutex(NULL, false, NULL);
	init();
}
DWORD WINAPI initialize(LPVOID param);
DWORD WINAPI WorkWithClient(LPVOID param);
void ServerInterLayer::init()
{
	server = this;
	DWORD thID;
	CreateThread(NULL, NULL, initialize, NULL, NULL, &thID);
}
DWORD WINAPI initialize(LPVOID param)
{
	info host_info;
	host_info.ID = 0;
	if (WSAStartup(0x202, (WSADATA *)&(host_info.buff[0])))
	{
		server->setStatus(s::error);
		return 0;
	}
	//Скопировать этот код и сделать функцию создания сокета
	SOCKET server_socket;
	sockaddr_in local_addr;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(server->getPort());
	local_addr.sin_addr.s_addr = 0;
	if (bind(server_socket, (sockaddr *)&local_addr, sizeof(local_addr)))
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	//Ожидание подключений
	if (listen(server_socket, 20))
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData); // инициализируем socket'ы используя Ws2_32.dll для процесса

	char HostName[1024]; // создаем буфер для имени хоста
	DWORD m_HostIP = 0;
	LPHOSTENT lphost;

	if (!gethostname(HostName, 1024)) // получаем имя хоста
	{
		if (lphost = gethostbyname(HostName)) // получаем IP хоста, т.е. нашего компа
			m_HostIP = ((LPIN_ADDR)lphost->h_addr)->s_addr; // преобразуем переменную типа LPIN_ADDR в DWORD
	}
	host_info.name = string(HostName);
	host_info.IPv4 = inet_ntoa(*((in_addr*)lphost->h_addr_list[0]));
	host_info.stream = GetCurrentThread();
	host_info.mpath = "D:\\Client";
	server->client_info.push_back(host_info);
	WSACleanup(); // освобождаем сокеты, т.е. завершаем использование Ws2_32.dll
	server->setStatus(s::working);

	int client_addr_size = sizeof(server->client_addr);
	//Извлечение запросов на подключение из очереди
	while (server->setClient_socket(accept(server_socket, (sockaddr *)&(server->client_addr), &client_addr_size)))
	{
		info client;
		client.ID = server->new_ID();

		server->hst = gethostbyaddr((char *)(server->client_addr.sin_addr.s_addr), 4, AF_INET);
		client.name = (server->hst) ? server->hst->h_name : "";
		client.IPv4 = inet_ntoa(server->client_addr.sin_addr);
		client.sock = server->getClient_socket();
		client.mpath = host_info.mpath;

		EnterCriticalSection(&(server->cs_info));
		server->setClient_info(client);
		LeaveCriticalSection(&(server->cs_info));

		DWORD thID;
		CreateThread(NULL, NULL, WorkWithClient, &(server->client_info.back()), NULL, &thID);
	}

	return 0;
}

int ServerInterLayer::new_ID()
{
	int a;
	EnterCriticalSection(&cs_info);
	a = client_info.back().ID + 1;
	LeaveCriticalSection(&cs_info);
	return a;
}

int ServerInterLayer::Exit()
{
	/**/
	return 0;
}

DWORD WINAPI WorkWithClient(LPVOID param)
{
	info* c_client = (info*)param;
	info client = *c_client;
	client.stream = GetCurrentThread();
	//Добавить mailslot

	itoa(client.ID, client.buff, 10);
	send(client.sock, &client.buff[0], strlen(client.buff) + 1, 0);	//отправил команду

	return 0;
}

void ServerInterLayer::quit_client(int ID)
{
	/**/
}

#pragma region Get- и set-методы
s ServerInterLayer::getStatus()
{
	return this->status;
}
void ServerInterLayer::setStatus(s new_status)
{
	this->status = new_status;
}
list<string> ServerInterLayer::getFiles()
{
	WaitForSingleObject(hMutex_Files, INFINITE);
	list<string> res = this->files;
	ReleaseMutex(hMutex_Files);
	return res;
}
void ServerInterLayer::setFile(string new_file)
{
	WaitForSingleObject(hMutex_Files, INFINITE);
	this->files.push_back(new_file);
	ReleaseMutex(hMutex_Files);
}
list<string> ServerInterLayer::getUsers()
{
	WaitForSingleObject(hMutex_Users, INFINITE);
	list<string> res = this->users;
	ReleaseMutex(hMutex_Users);
	return res;
}
void ServerInterLayer::setUser(string new_user)
{
	WaitForSingleObject(hMutex_Users, INFINITE);
	this->users.push_back(new_user);
	ReleaseMutex(hMutex_Users);
}
void ServerInterLayer::setClient_info(info new_client_info)
{
	this->client_info.push_back(new_client_info);
}
SOCKET ServerInterLayer::getClient_socket()
{
	return this->client_socket;
}
SOCKET ServerInterLayer::setClient_socket(SOCKET new_client_socket)
{
	this->client_socket = new_client_socket;
	return new_client_socket;
}
u_short ServerInterLayer::getPort()
{
	return this->port;
}
void ServerInterLayer::pushLog(string message)
{
	WaitForSingleObject(hMutex_Log, INFINITE);
	this->log.push_back(message);
	ReleaseMutex(hMutex_Log);
}
string ServerInterLayer::popLog()
{
	WaitForSingleObject(hMutex_Log, INFINITE);
	string s = this->log.front();
	this->log.pop_front();
	ReleaseMutex(hMutex_Log);
	return s;
}
bool ServerInterLayer::Log_isEmpty()
{
	return this->log.empty();
}
#pragma endregion