#include <Windows.h>
#include <cstring>
#include "ServerInterLayer.h"
#pragma comment( lib, "ws2_32.lib" )
//#define client server->client_info[id]
ServerInterLayer * server;
ServerInterLayer::ServerInterLayer()
{
	InitializeCriticalSection(&cs_info);
	hMutex_Log = CreateMutex(NULL, false, NULL);
	hMutex_Users_Files = CreateMutex(NULL, false, NULL);
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
	host_info.name = 0;
	if (WSAStartup(0x202, (WSADATA *)&(host_info.buff[0])) != 0)
	{
		server->setStatus(s::error);
		return 0;
	}
	//—копировать этот код и сделать функцию создани€ сокета
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
	//ќжидание подключений
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	//WSADATA wsaData;
	//WSAStartup(MAKEWORD(1, 1), &wsaData); // инициализируем socket'ы использу€ Ws2_32.dll дл€ процесса

	char HostName[1024]; // создаем буфер дл€ имени хоста
	DWORD m_HostIP = 0;
	LPHOSTENT lphost;

	if (!gethostname(HostName, 1024)) // получаем им€ хоста
	{
		if (lphost = gethostbyname(HostName)) // получаем IP хоста, т.е. нашего компа
			m_HostIP = ((LPIN_ADDR)lphost->h_addr)->s_addr; // преобразуем переменную типа LPIN_ADDR в DWORD
	}
	host_info.status = n::server;
	host_info.stream = GetCurrentThread();
	server->mpath = "D:\\Client";
	host_info.sock = server_socket;
	server->IPv4 = inet_ntoa(*((in_addr*)lphost->h_addr_list[0]));
	EnterCriticalSection(&(server->cs_info));
	server->client_info.push_back(host_info);
	LeaveCriticalSection(&(server->cs_info));
	server->setStatus(s::working);

	//»звлечение запросов на подключение из очереди
	while (server->setClient_socket(accept(server_socket, NULL, NULL)) != INVALID_SOCKET)
	{
		info client;
		client.name = -1;
		client.sock = server->getClient_socket();

		EnterCriticalSection(&(server->cs_info));
		server->setClient_info(client);
		int i = server->client_info.size() - 1;
		LeaveCriticalSection(&(server->cs_info));

		DWORD thID;
		CreateThread(NULL, NULL, WorkWithClient, &i, NULL, &thID);
	}

	return 0;
}

int ServerInterLayer::new_name()
{
	int a = 0;
	EnterCriticalSection(&cs_info);
	for each (info user in client_info)
	{
		if (user.name > a)
			a = user.name;
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
	const int id = *num;
	server->client_info[id].stream = GetCurrentThread();
	server->client_info[id].status = n::on;
	//ƒобавить mailslot

	server->receive(id);
	if (strcmp(server->client_info[id].buff, "new") == 0)
	{
		//новый логин
		EnterCriticalSection(&server->cs_info);
		server->client_info[id].name = server->new_name();
		LeaveCriticalSection(&server->cs_info);
		itoa(server->client_info[id].name, server->client_info[id].buff, 10);
		server->send_buff(id);
	}
	else if (int num = atoi(server->client_info[id].buff) > 0)
	{
		//это клиент с именем num, зарегать его
		server->client_info[id].name = num;
		strcpy(server->client_info[id].buff, "done");
		server->send_buff(id);
	}
	else
	{
		;//прислано что-то неверное
	}
	server->isOutDated_Users = true;

	bool work = true;
	while (work)
	{
		server->receive(id);
		if (strcmp(server->client_info[id].buff, "pause") == 0)
		{
			server->client_info[id].status = n::off;
			//пока не дописала
		}
		else if (strcmp(server->client_info[id].buff, "logout") == 0)
		{
			;//еще не написала
		}
		else if (strcmp(server->client_info[id].buff, "update") == 0)
		{
			server->sendFiles(id);
		}




	}



	return 0;
}

bool ServerInterLayer::sendFiles(int id)
{
	int i = 0;
	while (i < client_info[id].files.size())
	{
		bool end = false;
		int j = 0;
		strcpy(client_info[id].buff, "");
		while (i < client_info[id].files.size() && !end)
		{
			if (client_info[id].files[i].size() < (size_buff - j - 1))
			{
				strcat(client_info[id].buff, client_info[id].files[i].c_str());
				strcat(client_info[id].buff, "|");
				j += client_info[id].files[i].size() + 1;
				i++;
			}
			else
			{
				end = true;
			}
		}
		strcat(client_info[id].buff, "*");
		send_buff(id);
	}
	strcpy(client_info[id].buff, "done");
	send_buff(id);
	return true;
}

int ServerInterLayer::send_buff(int id)
{
	return send(client_info[id].sock, &client_info[id].buff[0], strlen(client_info[id].buff) + 1, 0);	//отправил команду
}

int ServerInterLayer::receive(int id)
{
	int res;
	if (res = recv(client_info[id].sock, &client_info[id].buff[0], sizeof(client_info[id].buff), 0) == SOCKET_ERROR)
	{
		//ошибка сокета!
		quit_client(id);
	}
	return res;
}

void ServerInterLayer::quit_client(int id)
{
	//сохран€ю текущие данные:
	string name = mpath + to_string(client_info[id].name) + ".txt";
	//создаю файлик с названием name.txt
	ofstream save(name);
	//пишу туда содержимое files
	for (int i = 0; i < client_info[id].files.size(); i++)
		save << client_info[id].files[i] << endl;
	save.close();

	//закрываю поток работы с клиентом
	client_info[id].status = n::off;
	ExitThread(0);
}

void ServerInterLayer::new_user(int name)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	int size = users.size();
	if (name > size)
	{
		;//как ты это сделал вообще? пропустил одно (или больше) им€ клиента
	}
	else if (size == name)
	{
		users.reserve(name + 1);
		users[name] = users[0];
	}
	ReleaseMutex(hMutex_Users_Files);
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
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	vector<string> res = this->files;
	isOutDated_Files = false;
	ReleaseMutex(hMutex_Users_Files);
	return res;
}
void ServerInterLayer::setFile(string new_file)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	this->files.push_back(new_file);
	ReleaseMutex(hMutex_Users_Files);
}
vector<string> ServerInterLayer::getUsers()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	vector<string> mas = { " (server)", " (off)", " (on)" };
	vector<string> res = {};
	for each (info user in client_info)
	{
		res.push_back("user_" + std::to_string(user.name) + mas.at((int)user.status));
	}
	this->users.clear();
	users = res;
	isOutDated_Users = false;
	ReleaseMutex(hMutex_Users_Files);
	return res;
}
void ServerInterLayer::setUser(string new_user)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	this->users.push_back(new_user);
	ReleaseMutex(hMutex_Users_Files);
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