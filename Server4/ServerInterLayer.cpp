#include <Windows.h>
#include <cstring>
#include "ServerInterLayer.h"
#pragma comment( lib, "ws2_32.lib" )
//#define client server->client_info[n]
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
	if (WSAStartup(0x202, (WSADATA *)&(host_info.buff[0])) != 0)
	{
		server->setStatus(s::error);
		return 0;
	}
	//Скопировать этот код и сделать функцию создания сокета
	SOCKET server_socket;
	sockaddr_in local_addr;
	local_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	local_addr.sin_port = htons(server->getPort());
	local_addr.sin_family = AF_INET;
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP/*или NULL???*/);
	if (server_socket == INVALID_SOCKET)
	{
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	if (bind(server_socket, (sockaddr *)&local_addr, sizeof(local_addr)) == SOCKET_ERROR)
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	//Ожидание подключений
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	//WSADATA wsaData;
	//WSAStartup(MAKEWORD(1, 1), &wsaData); // инициализируем socket'ы используя Ws2_32.dll для процесса

	char HostName[1024]; // создаем буфер для имени хоста
	DWORD m_HostIP = 0;
	LPHOSTENT lphost;

	if (!gethostname(HostName, 1024)) // получаем имя хоста
	{
		if (lphost = gethostbyname(HostName)) // получаем IP хоста, т.е. нашего компа
			m_HostIP = ((LPIN_ADDR)lphost->h_addr)->s_addr; // преобразуем переменную типа LPIN_ADDR в DWORD
	}
	host_info.status = n::server;
	host_info.stream = GetCurrentThread();
	host_info.mpath = "D:\\Client";
	host_info.sock = server_socket;
	server->IPv4 = inet_ntoa(*((in_addr*)lphost->h_addr_list[0]));
	EnterCriticalSection(&(server->cs_info));
	server->client_info.push_back(host_info);
	LeaveCriticalSection(&(server->cs_info));
	server->setStatus(s::working);

	//Извлечение запросов на подключение из очереди
	while (server->setClient_socket(accept(server_socket, NULL, NULL)) != INVALID_SOCKET)
	{
		info client;
		client.ID = -1;
		client.sock = server->getClient_socket();
		client.mpath = host_info.mpath;

		EnterCriticalSection(&(server->cs_info));
		server->setClient_info(client);
		int i = server->client_info.size() - 1;
		LeaveCriticalSection(&(server->cs_info));

		DWORD thID;
		CreateThread(NULL, NULL, WorkWithClient, &i, NULL, &thID);
	}

	return 0;
}

int ServerInterLayer::new_ID()
{
	int a = 0;
	EnterCriticalSection(&cs_info);
	for each (info user in client_info)
	{
		if (user.ID > a)
			a = user.ID;
	}
	a++;
	LeaveCriticalSection(&cs_info);
	return a;
}

int ServerInterLayer::Exit()
{
	//почистить сокеты?
	for each (info user in client_info)
	{
		closesocket(user.sock);
	}
	WSACleanup();
	server->setStatus(s::error);
	return 0;
}

DWORD WINAPI WorkWithClient(LPVOID param)
{
	int * num = (int *)param;
	int n = *num;
	server->client_info[n].stream = GetCurrentThread();
	server->client_info[n].status = n::on;
	//Добавить mailslot

	if (recv(server->client_info[n].sock, &server->client_info[n].buff[0], sizeof(server->client_info[n].buff), 0) == SOCKET_ERROR)
	{
		;//ошибка сокета
	}
	if (strcmp(server->client_info[n].buff, "new") == 0)
	{
		//новый логин
		EnterCriticalSection(&server->cs_info);
		server->client_info[n].ID = server->new_ID();
		LeaveCriticalSection(&server->cs_info);
		itoa(server->client_info[n].ID, server->client_info[n].buff, 10);
		send(server->client_info[n].sock, &server->client_info[n].buff[0], strlen(server->client_info[n].buff) + 1, 0);	//отправил команду
	}
	else if (int num = atoi(server->client_info[n].buff) > 0)
	{
		//это клиент с номером num, перезарегать его
		;//удалить все записи с таким ID, отдать команду закрыть их потоки. Тогда один ID - одно подключение!!!
		vector<info>::iterator iter = server->client_info.begin();
		while (iter != server->client_info.end())
		{
			for (; iter->ID != num && iter != server->client_info.end(); iter++) {}
			if (iter->ID == num)
			{
				if (iter->status == n::on)

					;//отправить команду завершить работу другого потока
				server->client_info.erase(iter);
			}
		}

		server->client_info[n].ID = num;
		strcpy(server->client_info[n].buff, "done");
		send(server->client_info[n].sock, &server->client_info[n].buff[0], strlen(server->client_info[n].buff) + 1, 0);	//отправил команду
	}
	else
	{
		;//прислано что-то неверное
	}
	server->isOutDated_Users = true;

	bool work = true;
	while (work)
	{
		if (recv(server->client_info[n].sock, &server->client_info[n].buff[0], sizeof(server->client_info[n].buff), 0) == SOCKET_ERROR)
		{
			;//ошибка сокета
		}
		if (strcmp(server->client_info[n].buff, "pause") == 0)
		{
			server->client_info[n].status = n::off;
		}
		else if (strcmp(server->client_info[n].buff, "logout") == 0)
		{

		}




	}



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
vector<string> ServerInterLayer::getFiles()
{
	WaitForSingleObject(hMutex_Files, INFINITE);
	vector<string> res = this->files;
	isOutDated_Files = false;
	ReleaseMutex(hMutex_Files);
	return res;
}
void ServerInterLayer::setFile(string new_file)
{
	WaitForSingleObject(hMutex_Files, INFINITE);
	this->files.push_back(new_file);
	ReleaseMutex(hMutex_Files);
}
vector<string> ServerInterLayer::getUsers()
{
	WaitForSingleObject(hMutex_Users, INFINITE);
	vector<string> mas = { " (server)", " (off)", " (on)" };
	vector<string> res = {};
	for each (info user in client_info)
	{
		res.push_back("user_" + std::to_string(user.ID) + mas.at((int)user.status));
	}
	this->users.clear();
	users = res;
	isOutDated_Users = false;
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