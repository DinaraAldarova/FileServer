#include <Windows.h>
#include <cstring>
#include <algorithm>  // sort
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
ServerInterLayer::~ServerInterLayer()
{
	Exit();
}

DWORD WINAPI initialize(LPVOID param);
DWORD WINAPI WorkWithClient(LPVOID param);

bool ServerInterLayer::init()
{
	server = this;
	DWORD thID;
	CreateThread(NULL, NULL, initialize, NULL, NULL, &thID);
	return true;
}

DWORD WINAPI initialize(LPVOID param)
{
	server->pushLog("Запуск сервера");
	info host_info;
	host_info.name = 0;
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
	server->path = "D:\\Server\\";
	host_info.sock = server_socket;
	server->IPv4 = inet_ntoa(*((in_addr*)lphost->h_addr_list[0]));
	EnterCriticalSection(&(server->cs_info));
	server->client_info.push_back(host_info);
	LeaveCriticalSection(&(server->cs_info));
	server->setStatus(s::working);
	server->pushLog("Сервер запущен");

	server->load_from_backup();

	//Извлечение запросов на подключение из очереди
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

/*	if (server->client_info[0].stream == GetCurrentThread())
		server->pushLog("Поток указан верно");
	else
		server->pushLog("Поток указан ошибочно");*/
	return 0;
}

int ServerInterLayer::new_name()
{
	EnterCriticalSection(&cs_info);
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	int i = 0;
	for (; i < users.size() && users[i] != "noname"; i++);
	ReleaseMutex(hMutex_Users_Files);
	LeaveCriticalSection(&cs_info);
	return i;
}

int ServerInterLayer::Exit()
{
	save_backup();
	for (int i = 1; i < client_info.size(); i++)
		quit_client(i);
	closesocket(client_info[0].sock);
	WaitForSingleObject(client_info[0].stream, INFINITE);
	DeleteCriticalSection(&cs_info);
	CloseHandle(&hMutex_Log);
	CloseHandle(&hMutex_Users_Files);
	WSACleanup();
	server->setStatus(s::error);
	//pushLog("Сервер отключен/n");
	return 0;
}

DWORD WINAPI WorkWithClient(LPVOID param)
{
	int * num = (int *)param;
	const int id = *num;
	server->client_info[id].stream = GetCurrentThread();
	server->client_info[id].status = n::on;
	InitializeCriticalSection(&server->client_info[id].cs_buf);
	//Добавить mailslot

	if (server->receive(id) == -1) { server->quit_client(id); return false; }
	if (strcmp(server->client_info[id].buff, "new") == 0)
	{
		//новый логин
		EnterCriticalSection(&server->cs_info);
		server->client_info[id].name = server->new_name();
		itoa(server->client_info[id].name, server->client_info[id].buff, 10);
		LeaveCriticalSection(&server->cs_info);
		server->send_buff(id);
		server->new_user(id);
		server->pushLog(server->getUsers()[server->client_info[id].name] + " login");
	}
	else
	{
		int num = atoi(server->client_info[id].buff);
		if (num > 0)
		{
			//это клиент с именем num, зарегать его
			EnterCriticalSection(&server->cs_info);
			server->client_info[id].name = num;
			if (num >= server->getUsers().size())
			{
				while (num > server->getUsers().size())
					server->setUser("noname");
				server->setUser("user_" + to_string(num) + "(on)");
			}
			strcpy(server->client_info[id].buff, "done");
			LeaveCriticalSection(&server->cs_info);
			server->send_buff(id);
			server->updateFiles_Users();
			server->pushLog(server->getUsers()[server->client_info[id].name] + " connected");
		}
		else
		{
			server->pushLog("Прислано что-то неверное");
			server->quit_client(id);
		}
	}

	bool work = true;
	while (work)
	{
		if (server->receive(id) == -1) { server->quit_client(id); return false; }
		if (strcmp(server->client_info[id].buff, "pause") == 0)
		{
			server->pushLog(server->getUsers()[server->client_info[id].name] + " pause");
			server->quit_client(id);
		}
		else if (strcmp(server->client_info[id].buff, "logout") == 0)
		{
			server->pushLog(server->getUsers()[server->client_info[id].name] + " logout");
			server->quit_client(id);
		}
		else if (strcmp(server->client_info[id].buff, "update") == 0)
		{
			server->sendFiles_Users(id);
		}
		else if (strncmp(server->client_info[id].buff, "upload", 6) == 0)
		{
			server->uploadFile(id);
		}
		else if (strncmp(server->client_info[id].buff, "download", 8) == 0)
		{
			server->downloadFile(id);
		}







	}
	return 0;
}

bool ServerInterLayer::update_clientFiles(int id)
{
	vector<string> mas;
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	EnterCriticalSection(&server->cs_info);
	int name = client_info[id].name;
	for (int i = 0; i < access[name].size(); i++)
	{
		if (access[name][i])
			mas.push_back(files[i]);
	}
	for (int j = 0; j < client_info.size(); j++)
	{
		if (client_info[j].name == name)
		{
			client_info[j].files = mas;
		}
	}
	LeaveCriticalSection(&server->cs_info);
	ReleaseMutex(hMutex_Users_Files);
	return true;
}

bool ServerInterLayer::updateFiles_Users()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	pushLog("Обновляю списки пользователей и файлов");

	if (users.size() != access.size() || users.size() == 0 || access.size() == 0)
	{
		pushLog("Количество пользователей рассинхронизировано, сбрасываю список пользователей");

		if (users.size() > 0)
		{
			string s = users[0];
			users.clear();
			users.push_back(s);
		}
		else
		{
			users.push_back("user_0(server)");
		}

		if (access.size() > 0)
		{
			vector <bool> a = access[0];
			access.clear();
			access.push_back(a);
		}
		else
		{
			access.push_back({});
			files.clear();
		}
	}
	if (files.size() != access[0].size())
	{
		pushLog("Количество файлов рассинхронизировано, сбрасываю список файлов");

		files.clear();
		access[0].clear();
	}
	bool need_message = false;
	for (int i = 1; i < access.size(); i++)
	{
		if (access[i].size() != access[0].size())
		{
			need_message = true;

			access[i] = access[0];
		}
	}

	if (need_message)
		pushLog("Таблица доуступа рассинхронизирована, сбрасываю ее");

	//обновляю статусы
	for (int id = 0; id < client_info.size(); id++)
	{
		int name = client_info[id].name;

		string str_name = "user_" + to_string(name);
		n status = client_info[id].status;
		if (status == n::off)
			str_name += "(off)";
		else if (status == n::on)
			str_name += "(on)";
		else
			str_name += "(server)";
		while (users.size() <= name)
		{
			users.push_back("noname");
			access.push_back(access[0]);
		}
		users[name] = str_name;
	}

	//проверяю доступность файлов
	for (int i = 0; i < files.size(); i++)
	{
		ifstream file(path + files[i]);
		if (file.is_open())
		{
			//файл доступен
			file.close();
		}
		else
		{
			//файл недоступен
			files.erase(files.begin() + i);
			for (int j = 0; j < access.size(); j++)
			{
				access[j].erase(access[j].begin() + i);
			}
		}
	}

	//обновляю список доступных файлов для всех пользователей
	for (int i = 1; i < client_info.size(); i++)
	{
		update_clientFiles(i);
	}
	isOutDated_Files = true;
	isOutDated_Users = true;
	pushLog("Списки пользователей и файлов обновлены");
	ReleaseMutex(hMutex_Users_Files);
	save_backup();	
	return true;
}

bool ServerInterLayer::sendFiles_Users(int id)
{
	update_clientFiles(id);
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
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

	i = 0;
	while (i < users.size())
	{
		bool end = false;
		int j = 0;
		strcpy(client_info[id].buff, "");
		while (i < users.size() && !end)
		{
			if (users[i].size() < (size_buff - j - 1))
			{
				strcat(client_info[id].buff, users[i].c_str());
				strcat(client_info[id].buff, "|");
				j += users[i].size() + 1;
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
	ReleaseMutex(hMutex_Users_Files);
	return true;
}

int ServerInterLayer::send_buff(int id, int i)
{
	EnterCriticalSection(&client_info[id].cs_buf);
	Sleep(50);
	int res = send(client_info[id].sock, &client_info[id].buff[0], i /*strlen(client_info[id].buff) + 1*/, 0);	//отправил команду 
	LeaveCriticalSection(&client_info[id].cs_buf);
	return res;
}

int ServerInterLayer::receive(int id)
{
	EnterCriticalSection(&client_info[id].cs_buf);
	for (int i = 0; i < size_buff; i++)
		client_info[id].buff[i] = '/0';
	int res = recv(client_info[id].sock, &client_info[id].buff[0], sizeof(client_info[id].buff), 0);
	LeaveCriticalSection(&client_info[id].cs_buf);
	return res;
}

bool ServerInterLayer::quit_client(int id)
{
	//закрываю поток работы с клиентом
	HANDLE stream = GetCurrentThread();
	EnterCriticalSection(&cs_info);
	if (id >= client_info.size() || client_info[id].stream != stream)
	{
		pushLog("Ошибка! Попытка отключить клиента " + to_string(client_info[id].name) + " (порядковый номер " + to_string(id) + ": такой пользователь не существует");
		return false;
	}
	pushLog(users.at(client_info[id].name) + " вышел");
	client_info[id].status = n::off;
	LeaveCriticalSection(&cs_info);
	updateFiles_Users();
	DeleteCriticalSection(&client_info[id].cs_buf);
	ExitThread(0);
	return true;
}

bool ServerInterLayer::new_user(int id)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	int name = client_info[id].name;
	if (users.size() != access.size())
	{
		pushLog("Списки пользователей и доступа рассинхронизированы");
		ReleaseMutex(hMutex_Users_Files);
		updateFiles_Users();
		WaitForSingleObject(hMutex_Users_Files, INFINITE);
	}
	while (name > users.size())
	{
		//если клиент забежал
		users.push_back("noname");
		access.push_back(access[0]);
	}
	if (name == users.size())
	{
		//все ок
		users.push_back("user_" + to_string(name) + " (on)");
		access.push_back(access[0]);
	}
	else
	{
		//а это запоздавший клиент
		users[name] = "user_" + to_string(name) + " (on)";
		access[name] = access[0];
	}
	ReleaseMutex(hMutex_Users_Files);
	updateFiles_Users();
	return true;
}

int ServerInterLayer::new_loading_file(string name, string f_access, vector <string> access_users, int id)
{
	//WaitForSingleObject(hMutex_Users_Files, INFINITE);

	loading_files newfile;
	newfile.name = name;
	newfile.f_access = f_access;
	if (f_access == "private")
	{
		newfile.access_users.push_back(client_info[id].name);
	}
	else if (f_access == "protected")
	{
		for (int i = 0; i < access_users.size(); i++)
		{
			string num = "";
			for (int j = 0; j < access_users[i].length(); j++)
				if (access_users[i][j] >= '0' && access_users[i][j] <= '9')
					num += access_users[i][j];
			newfile.access_users.push_back(stoi(num));
		}
		if (!newfile.access_users.empty())
			sort(newfile.access_users.begin(), newfile.access_users.end());
	}

	int res = loading.size();
	loading.push_back(newfile);

	//ReleaseMutex(hMutex_Users_Files);
	return res;
}

bool ServerInterLayer::new_file(int id)
{
	updateFiles_Users();
	WaitForSingleObject(hMutex_Users_Files, INFINITE);

	string name = loading[id].name;
	string f_access = loading[id].f_access;
	vector<int> access_users = loading[id].access_users;

	loading.erase(loading.begin() + id);

	vector <bool> vec;
	if (f_access == "private")
	{
		for (int i = 0; i < users.size(); i++)
		{
			vec.push_back(false);
		}
		vec[access_users[0]] = true;
	}
	else if (f_access == "public")
	{
		for (int i = 0; i < users.size(); i++)
		{
			vec.push_back(true);
		}
	}
	else if (f_access == "protected")
	{
		int j = 0;
		for (int u = 0; u < users.size(); u++)
		{
			vec.push_back(false);
		}
		for (int u = 0; u < access_users.size(); u++)
		{
			vec[access_users[u]] = true;
		}
	}
	else
	{
		pushLog("Неправильный тип доступа: " + f_access);
	}

	files.push_back(name);
	for (int i = 0; i < access.size(); i++)
		access[i].push_back(vec[i]);

	ReleaseMutex(hMutex_Users_Files);
	updateFiles_Users();
	return true;
}

bool ServerInterLayer::uploadFile(int id)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	int i = 7;
	string s = "";
	while (client_info[id].buff[i] != '|')
	{
		s += client_info[id].buff[i];
		i++;
	}
	string name = s;
	i++;
	s = "";
	while (client_info[id].buff[i] != '|')
	{
		s += client_info[id].buff[i];
		i++;
	}
	string f_access = s;
	vector<string> access_users = {};
	if (f_access == "protected")
	{
		while (client_info[id].buff[i] != '*')
		{
			s = "";
			while (client_info[id].buff[i] != '|')
			{
				s += client_info[id].buff[i];
				i++;
			}
			access_users.push_back(s);
			i++;
		}
	}
	//добавить дозагрузку файла
	int id_file = new_loading_file(name, f_access, access_users, id);

	FILE * file;
	file = fopen((path + name).c_str(), "wb+");
	if (file == NULL)
	{
		pushLog("Ошибка! Файл невозможно создать. Получение невозможно");
		strcpy(client_info[id].buff, "error");
		send_buff(id);
		ReleaseMutex(hMutex_Users_Files);
		return false;
	}

	strcpy(client_info[id].buff, "ready");
	send_buff(id);

	if (receive(id) == -1) { pushLog("Ошибка сокета, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }
	long size_file = atol(client_info[id].buff);
	if (size_file == 0) { pushLog("Клиент прислал не размер файла, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); return false; }

	char buff_2[size_buff] = "";

	for (long pos = 0; pos < size_file; )
	{
		if (receive(id) == -1) { pushLog("Ошибка сокета, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }
		strcpy(buff_2, client_info[id].buff);
		//потом заменить на вычисление контрольной суммы
		strcpy(client_info[id].buff, "ok");
		send_buff(id);

		if (receive(id) == -1) { pushLog("Ошибка сокета, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }

		if (client_info[id].buff[0] == '\0')
		{
			client_info[id].buff[0] = '_';
		}
		if (!strcmp(client_info[id].buff, "next") || !strcmp(client_info[id].buff, "_next"))
			//загружаем следующий блок
		{
			if (size_file - pos < size_buff)
			{
				fwrite(&buff_2, size_file - pos, 1, file);
				pos += size_file - pos;
			}
			else
			{
				fwrite(&buff_2, sizeof(buff_2), 1, file);
				pos += sizeof(buff_2);
			}
			strcpy(client_info[id].buff, "next");
		}
		else if (!strcmp(client_info[id].buff, "repeat"))
			//повторить загрузку блока
			strcpy(client_info[id].buff, "repeat");
		else
			//загружено что-то еще
		{
			pushLog("Клиент ответил отрицательно, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); return false;
		}

		send_buff(id);
	}
	fclose(file);
	if (receive(id) == -1) { pushLog("Ошибка сокета после загрузки файла"); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }
	if (!strcmp(client_info[id].buff, "end"))
	{
		pushLog("Файл загружен");
		strcpy(client_info[id].buff, "end");
		send_buff(id);
		ReleaseMutex(hMutex_Users_Files);

		new_file(id_file);
		return true;
	}
	else
	{
		pushLog("Клиент отправил ошибку после загрузки файла");
		strcpy(client_info[id].buff, "error");
		send_buff(id);
		ReleaseMutex(hMutex_Users_Files);
		return false;
	}
}

bool ServerInterLayer::downloadFile(int id)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);

	int i = 7;
	string s = "";
	while (client_info[id].buff[i] != '|')
	{
		s += client_info[id].buff[i];
		i++;
	}
	string name = s;
	i++;
	s = "";
	while (client_info[id].buff[i] != '|')
	{
		s += client_info[id].buff[i];
		i++;
	}
	string f_access = s;
	vector<string> access_users = {};
	if (f_access == "protected")
	{
		while (client_info[id].buff[i] != '*')
		{
			s = "";
			while (client_info[id].buff[i] != '|')
			{
				s += client_info[id].buff[i];
				i++;
			}
			access_users.push_back(s);
			i++;
		}
	}
	//добавить дозагрузку файла
	int id_file = new_loading_file(name, f_access, access_users, id);

	FILE * file;
	file = fopen((path + name).c_str(), "wb+");
	if (file == NULL)
	{
		pushLog("Ошибка! Файл невозможно создать. Получение невозможно");
		strcpy(client_info[id].buff, "error");
		send_buff(id);
		ReleaseMutex(hMutex_Users_Files);
		return false;
	}

	strcpy(client_info[id].buff, "ready");
	send_buff(id);

	if (receive(id) == -1) { pushLog("Ошибка сокета, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }
	long size_file = atol(client_info[id].buff);
	if (size_file == 0) { pushLog("Клиент прислал не размер файла, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); return false; }

	char buff_2[size_buff] = "";

	for (long pos = 0; pos < size_file; )
	{
		if (receive(id) == -1) { pushLog("Ошибка сокета, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }
		strcpy(buff_2, client_info[id].buff);
		//потом заменить на вычисление контрольной суммы
		strcpy(client_info[id].buff, "ok");
		send_buff(id);

		if (receive(id) == -1) { pushLog("Ошибка сокета, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }

		if (client_info[id].buff[0] == '\0')
		{
			client_info[id].buff[0] = '_';
		}
		if (!strcmp(client_info[id].buff, "next") || !strcmp(client_info[id].buff, "_next"))
			//загружаем следующий блок
		{
			if (size_file - pos < size_buff)
			{
				fwrite(&buff_2, size_file - pos, 1, file);
				pos += size_file - pos;
			}
			else
			{
				fwrite(&buff_2, sizeof(buff_2), 1, file);
				pos += sizeof(buff_2);
			}
			strcpy(client_info[id].buff, "next");
		}
		else if (!strcmp(client_info[id].buff, "repeat"))
			//повторить загрузку блока
			strcpy(client_info[id].buff, "repeat");
		else
			//загружено что-то еще
		{
			pushLog("Клиент ответил отрицательно, загрузка прервана"); fclose(file); ReleaseMutex(hMutex_Users_Files); return false;
		}

		send_buff(id);
	}
	fclose(file);
	if (receive(id) == -1) { pushLog("Ошибка сокета после загрузки файла"); ReleaseMutex(hMutex_Users_Files); quit_client(id); return false; }
	if (!strcmp(client_info[id].buff, "end"))
	{
		pushLog("Файл загружен");
		strcpy(client_info[id].buff, "end");
		send_buff(id);
		ReleaseMutex(hMutex_Users_Files);

		new_file(id_file);
		return true;
	}
	else
	{
		pushLog("Клиент отправил ошибку после загрузки файла");
		strcpy(client_info[id].buff, "error");
		send_buff(id);
		ReleaseMutex(hMutex_Users_Files);
		return false;
	}

	ReleaseMutex(hMutex_Users_Files);
	return true;
}

bool ServerInterLayer::save_backup()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	ofstream backup(path + "backup\\" + "backup_files.txt", ios_base::out | ios_base::trunc);
	backup << files.size();
	for (int i = 0; i < files.size(); i++)
	{
		backup << endl << files[i];
	}
	backup.close();
	pushLog("Записана резервная копия списка файлов");

	backup.open(path + "backup\\" + "backup_users.txt", ios_base::out | ios_base::trunc);
	backup << users.size();
	for (int i = 0; i < users.size(); i++)
	{
		backup << endl << users[i];
	}
	backup.close();
	pushLog("Записана резервная копия списка пользователей");

	backup.open(path + "backup\\" + "backup_access.txt", ios_base::out | ios_base::trunc);
	backup << access.size();
	if (access.size() > 0)
		backup << endl << access[0].size();
	for (int i = 0; i < access.size(); i++)
	{
		backup << endl;
		for (int j = 0; j < access[i].size(); j++)
		{
			backup << access[i][j];
		}
	}
	backup.close();
	pushLog("Записана резервная копия таблицы доступа к файлам");

	backup.open(path + "backup\\" + "backup_loading_files.txt", ios_base::out | ios_base::trunc);
	backup << loading.size();
	for (int i = 0; i < loading.size(); i++)
	{
		backup << endl << loading[i].name << endl << loading[i].f_access << endl << loading[i].access_users.size();
		for (int j = 0; j < loading[i].access_users.size(); j++)
		{
			backup << endl << loading[i].access_users[j];
		}
	}
	backup.close();
	pushLog("Записана резервная копия таблицы недогруженных файлов");

	ReleaseMutex(hMutex_Users_Files);
	return true;
}

bool ServerInterLayer::load_from_backup()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	ifstream backup(path + "backup\\" + "backup_files.txt");
	char buf[50];
	int s_files, s_users;
	bool error = false;
	if (backup.is_open())
	{
		files.clear();
		backup.getline(buf, sizeof(buf));
		s_files = atoi(buf);
		int i = 0;
		for (; i < s_files && !backup.eof(); i++)
		{
			backup.getline(buf, sizeof(buf));
			files.push_back(buf);
		}
		backup.close();
		pushLog("Загружена резервная копия списка файлов");
		if (i < s_files)
		{
			error = true;
			pushLog("Резервная копия списка файлов была нарушена");
		}
	}
	else
	{
		error = true;
		pushLog("Не удалось загрузить резервную копию списка файлов");
	}

	backup.open(path + "backup\\" + "backup_users.txt");
	if (backup.is_open())
	{
		users.clear();
		backup.getline(buf, sizeof(buf));
		s_users = atoi(buf);
		int i = 0;
		for (; i < s_users && !backup.eof(); i++)
		{
			backup.getline(buf, sizeof(buf));
			users.push_back(buf);
		}
		backup.close();
		pushLog("Загружена резервная копия списка пользователей");
		if (i < s_users)
		{
			error = true;
			pushLog("Резервная копия списка пользователей была нарушена");
		}
	}
	else
	{
		error = true;
		pushLog("Не удалось загрузить резервную копию списка пользователей");
	}

	backup.open(path + "backup\\" + "backup_access.txt");
	if (backup.is_open())
	{
		access.clear();
		backup.getline(buf, sizeof(buf));
		int size_u = atoi(buf);
		backup.getline(buf, sizeof(buf));
		int size_f = atoi(buf);
		if (size_f != s_files || size_u != s_users)
		{
			error = true;
			pushLog("Резервная копия списка пользователей или файлов была нарушена");
		}
		else
		{
			int i = 0;
			for (; i < size_u && !backup.eof(); i++)
			{
				backup.getline(buf, sizeof(buf));
				vector<bool> f;
				int j = 0;
				for (; j < size_f; j++)
					f.push_back(atoi(buf));
				access.push_back(f);
				if (j < size_f)
				{
					error = true;
					pushLog("Резервная копия таблицы доступа была нарушена: отсутствует метка файла");
				}
			}

			backup.close();
			pushLog("Загружена резервная копия таблицы доступа к файлам");
			if (i < size_u)
			{
				error = true;
				pushLog("Резервная копия таблицы доступа была нарушена: отсутствует метка пользователя");
			}
		}
	}
	else
	{
		error = true;
		pushLog("Не удалось загрузить резервную копию таблицы доступа к файлам");
	}

	backup.open(path + "backup\\" + "backup_loading_files.txt");
	if (backup.is_open())
	{
		loading.clear();
		backup.getline(buf, sizeof(buf));
		int size_f = atoi(buf);
		int i = 0;
		for (; i < size_f && !backup.eof(); i++)
		{
			loading_files load;
			backup.getline(buf, sizeof(buf));
			load.name = buf;
			backup.getline(buf, sizeof(buf));
			load.f_access = buf;
			backup.getline(buf, sizeof(buf));
			int size_u = atoi(buf);
			vector<int> u;
			int j = 0;
			for (; j < size_u; j++)
				u.push_back(atoi(buf));
			load.access_users = u;
			loading.push_back(load);
			if (j < size_u)
			{
				error = true;
				pushLog("Резервная копия таблицы недогруженных файлов была нарушена: неверное количество пользователей");
			}
		}
		if (i < size_f)
		{
			error = true;
			pushLog("Резервная копия таблицы доступа была нарушена: отсутствует метка файла");
		}
		backup.close();
		pushLog("Загружена резервная копия таблицы недогруженных файлов");
	}
	else
	{
		//error = true;
		pushLog("Не удалось загрузить резервную копию таблицы недогруженных файлов");
	}

	if (error)
	{
		files.clear();
		users.clear();
		loading.clear();
		pushLog("Из-за ошибки в резервной копии списки файлов и пользователей обнулены");

	}
	isOutDated_Files = true;
	isOutDated_Users = true;
	ReleaseMutex(hMutex_Users_Files);
	updateFiles_Users();
	return true;
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
	vector<string> res = users;
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
	ofstream file(path + "backup\\" + "log.txt", ios_base::app);
	file << s << endl;
	ReleaseMutex(hMutex_Log);
	return s;
}

bool ServerInterLayer::Log_isEmpty()
{
	return this->log.empty();
}
#pragma endregion