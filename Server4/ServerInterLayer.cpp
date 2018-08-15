#include "ServerInterLayer.h"
#pragma comment( lib, "ws2_32.lib" )
ServerInterLayer::ServerInterLayer()
{
	init();
}
DWORD WINAPI initialize(LPVOID param);
DWORD WINAPI WorkWithClient(LPVOID param);
void ServerInterLayer::init()
{
	DWORD thID;
	CreateThread(NULL, NULL, initialize, NULL, NULL, &thID);
}
DWORD WINAPI initialize(LPVOID param)
{
	ServerInterLayer * server = (ServerInterLayer *)param;
	InitializeCriticalSection(&(server->getCs_info()));
	if (WSAStartup(0x202, (WSADATA *)server->getBuff()))
	{
		server->setStatus(s::error);
		return 0;
	}
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
	//ќжидание подключений
	if (listen(server_socket, 20))
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	EnterCriticalSection(&(server->getCs_info()));
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
	LeaveCriticalSection(&(server->getCs_info()));
	WSACleanup(); // освобождаем сокеты, т.е. завершаем использование Ws2_32.dll

	int client_addr_size = sizeof(server->getClient_addr());
	//»звлечение запросов на подключение из очереди
	while (server->setClient_socket(accept(server_socket, (sockaddr *)&(server->getClient_addr()), &client_addr_size)))
	{
		info client;
		client.ID = server->new_ID();

		server->setHst(gethostbyaddr((char *)(server->getClient_addr().sin_addr.s_addr), 4, AF_INET));
		client.name = (server->getHst()) ? server->getHst()->h_name : "";
		client.IPv4 = inet_ntoa(server->getClient_addr().sin_addr);
		client.sock = server->getClient_socket();
		DWORD thID;
		server->setClient_info(client);
		client.stream = CreateThread(NULL, NULL, WorkWithClient, &server->getClient_info().back(), NULL, &thID);
	}

	return 0;
}

int ServerInterLayer::new_ID()
{
	EnterCriticalSection(&cs_info);
	int a = client_info.back().ID + 1;
	LeaveCriticalSection(&cs_info);
	return a;
}

int ServerInterLayer::Exit()
{
	return 0;
}

DWORD WINAPI WorkWithClient(LPVOID param)
{
	info* c_client = (info*)param;
	info client = *c_client;

	return 0;
}

void ServerInterLayer::quit_client(int ID)
{

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
	return this->files;
}
void ServerInterLayer::setFiles(string new_file)
{
	this->files.push_back(new_file);
}
list<string> ServerInterLayer::getUsers()
{
	return this->users;
}
void ServerInterLayer::setUsers(string new_user)
{
	this->users.push_back(new_user);
}
list <info> ServerInterLayer::getClient_info()
{
	return this->client_info;
}
void ServerInterLayer::setClient_info(info new_client_info)
{
	this->client_info.push_back(new_client_info);
}
string ServerInterLayer::getPuth()
{
	return this->puth;
}
void ServerInterLayer::setPuth(string new_puth)
{
	this->puth = new_puth;
}
char * ServerInterLayer::getBuff()
{
	return this->buff;
}
/*void ServerInterLayer::setBuff(char * new_buff)
{
	this->buff = new_buff;
}*/
SOCKET ServerInterLayer::getClient_socket()
{
	return this->client_socket;
}
SOCKET ServerInterLayer::setClient_socket(SOCKET new_client_socket)
{
	this->client_socket = new_client_socket;
	return new_client_socket;
}
sockaddr_in ServerInterLayer::getClient_addr()
{
	return this->client_addr;
}
void ServerInterLayer::setClient_addr(sockaddr_in new_client_addr)
{
	this->client_addr = new_client_addr;
}
u_short ServerInterLayer::getPort()
{
	return this->port;
}
void ServerInterLayer::setPort(u_short new_port)
{
	this->port = new_port;
}
HOSTENT * ServerInterLayer::getHst()
{
	return this->hst;
}
void ServerInterLayer::setHst(HOSTENT * new_hst)
{
	this->hst = new_hst;
}
CRITICAL_SECTION ServerInterLayer::getCs_info()
{
	return this->cs_info;
}
void ServerInterLayer::setCs_info(CRITICAL_SECTION new_cs_info)
{
	this->cs_info = new_cs_info;
}
#pragma endregion