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
	//����������� ���� ��� � ������� ������� �������� ������
	SOCKET server_socket;
	sockaddr_in local_addr;
	local_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	local_addr.sin_port = htons(server->getPort());
	local_addr.sin_family = AF_INET;
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP/*��� NULL???*/);
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
	//�������� �����������
	if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(server_socket);
		WSACleanup();
		server->setStatus(s::error);
		return 0;
	}
	//WSADATA wsaData;
	//WSAStartup(MAKEWORD(1, 1), &wsaData); // �������������� socket'� ��������� Ws2_32.dll ��� ��������

	char HostName[1024]; // ������� ����� ��� ����� �����
	DWORD m_HostIP = 0;
	LPHOSTENT lphost;

	if (!gethostname(HostName, 1024)) // �������� ��� �����
	{
		if (lphost = gethostbyname(HostName)) // �������� IP �����, �.�. ������ �����
			m_HostIP = ((LPIN_ADDR)lphost->h_addr)->s_addr; // ����������� ���������� ���� LPIN_ADDR � DWORD
	}
	host_info.status = n::server;
	host_info.stream = GetCurrentThread();
	server->mpath = "D:\\Server\\";
	host_info.sock = server_socket;
	server->IPv4 = inet_ntoa(*((in_addr*)lphost->h_addr_list[0]));
	EnterCriticalSection(&(server->cs_info));
	server->client_info.push_back(host_info);
	LeaveCriticalSection(&(server->cs_info));
	server->setStatus(s::working);

	//���������� �������� �� ����������� �� �������
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
	//��������� ������?
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
	//�������� mailslot

	server->receive(id);
	if (strcmp(server->client_info[id].buff, "new") == 0)
	{
		//����� �����
		EnterCriticalSection(&server->cs_info);
		server->client_info[id].name = server->new_name();
		LeaveCriticalSection(&server->cs_info);
		itoa(server->client_info[id].name, server->client_info[id].buff, 10);
		server->send_buff(id);
		server->new_user(id);
	}
	else if (int num = atoi(server->client_info[id].buff) > 0)
	{
		//��� ������ � ������ num, �������� ���
		server->client_info[id].name = num;
		strcpy(server->client_info[id].buff, "done");
		server->send_buff(id);
	}
	else
	{
		server->pushLog("�������� ���-�� ��������");
	}
	server->isOutDated_Users = true;

	bool work = true;
	while (work)
	{
		server->receive(id);
		if (strcmp(server->client_info[id].buff, "pause") == 0)
		{
			server->client_info[id].status = n::off;
			//���� �� ��������
		}
		else if (strcmp(server->client_info[id].buff, "logout") == 0)
		{
			server->pushLog("�����");
			//��� �� ��������
		}
		else if (strcmp(server->client_info[id].buff, "update") == 0)
		{
			server->sendFiles_Users(id);
		}




	}



	return 0;
}

bool ServerInterLayer::update_clientFiles(int id)
{
	client_info[id].files.clear();
	for (int i = 0; i < access[id].size(); i++)
	{
		if (access[id][i])
			client_info[id].files.push_back(files[i]);
	}
	return true;
}

bool ServerInterLayer::updateFiles_Users()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	pushLog("�������� ������ ������������� � ������");

	//�������� �������
	for (int i = 0; i < client_info.size(); i++)
	{
		string name = "user_" + to_string(client_info[i].name);
		n status = client_info[i].status;
		if (status == n::off)
			name += "(off)";
		else if (status == n::on)
			name += "(on)";
		else
			name += "(server)";
		while (users.size() <= client_info[i].name)
			users.push_back("noname");
		users[client_info[i].name] = name;
	}

	if (access.size() == 0)
	{
		vector<bool> buf;
		for (int i = 0; i < files.size(); i++)
			buf.push_back(true);
		access.push_back(buf);
	}

	//�������� ����������� ������
	for (int i = 0; i < files.size(); i++)
	{
		ifstream file(mpath + files[i]);
		if (file.is_open())
		{
			//���� ��������
			file.close();
		}
		else
		{
			//���� ����������
			files.erase(files.begin() + i);
			for (int j = 0; j < access.size(); j++)
			{
				access[j].erase(access[j].begin() + i);
			}
		}
	}

	//�������� ������ ��������� ������ ��� ���� �������������
	for (int i = 1; i < client_info.size(); i++)
	{
		update_clientFiles(i);
	}
	isOutDated_Files = true;
	isOutDated_Users = true;
	pushLog("������ ������������� � ������ ���������");
	ReleaseMutex(hMutex_Users_Files);
	return true;
}

bool ServerInterLayer::sendFiles_Users(int id)
{
	update_clientFiles(id);
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
	return true;
}

int ServerInterLayer::send_buff(int id)
{
	return send(client_info[id].sock, &client_info[id].buff[0], strlen(client_info[id].buff) + 1, 0);	//�������� �������
}

int ServerInterLayer::receive(int id)
{
	int res;
	if (res = recv(client_info[id].sock, &client_info[id].buff[0], sizeof(client_info[id].buff), 0) == SOCKET_ERROR)
	{
		//������ ������!
		quit_client(id);
	}
	return res;
}

void ServerInterLayer::quit_client(int id)
{
	//�������� ����� ������ � ��������
	client_info[id].status = n::off;
	ExitThread(0);
}

void ServerInterLayer::new_user(int name)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	int size = users.size();
	if (name > size)
	{
		//��� �� ��� ������ ������? ��������� ���� (��� ������) ��� �������
		users.reserve(name + 1);
		users[name] = "user" + to_string(name);
		access.reserve(name + 1);
		access[name] = access[0];
		for (int i = size; i < name; i++)
		{
			access[i] = {};//��� ��� ����� ������������ ��� �� ���������
		}
	}
	else if (size == name)
	{
		users.push_back("user_" + to_string(name) + " (on)");
		access.push_back(access[0]);
	}
	else
	{
		//� ��� ��� ����������� ������
		users.push_back("user" + to_string(name));
		access[name] = access[0];
	}
	ReleaseMutex(hMutex_Users_Files);
}

void ServerInterLayer::new_file(string name)
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	/*int size = users.size();
	if (name > size)
	{
		//��� �� ��� ������ ������? ��������� ���� (��� ������) ��� �������
		users.reserve(name + 1);
		users[name] = "user" + to_string(name);
		access.reserve(name + 1);
		access[name] = access[0];
		for (int i = size; i < name; i++)
		{
			access[i] = {};//��� ��� ����� ������������ ��� �� ���������
		}
	}
	else if (size == name)
	{
		users.push_back("user_" + to_string(name) + " (on)");
		access.push_back(access[0]);
	}
	else
	{
		//� ��� ��� ����������� ������
		users.push_back("user" + to_string(name));
		access[name] = access[0];
	}*/
	ReleaseMutex(hMutex_Users_Files);
}

bool ServerInterLayer::save_backup()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	ofstream backup(mpath + "backup_files.txt", ios_base::out | ios_base::trunc);
	backup << files.size();
	for (int i = 0; i < files.size(); i++)
	{
		backup << endl << files[i];
	}
	backup.close();
	pushLog("�������� ��������� ����� ������ ������");

	backup.open(mpath + "backup_users.txt", ios_base::out | ios_base::trunc);
	backup << users.size();
	for (int i = 0; i < users.size(); i++)
	{
		backup << endl << users[i];
	}
	backup.close();
	pushLog("�������� ��������� ����� ������ �������������");

	backup.open(mpath + "backup_access.txt", ios_base::out | ios_base::trunc);
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
	pushLog("�������� ��������� ����� ������� ������� � ������");
	ReleaseMutex(hMutex_Users_Files);
	return true;
}

bool ServerInterLayer::load_from_backup()
{
	WaitForSingleObject(hMutex_Users_Files, INFINITE);
	ifstream backup(mpath + "backup_files.txt");
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
		pushLog("��������� ��������� ����� ������ ������");
		if (i < s_files)
		{
			error = true;
			pushLog("��������� ����� ������ ������ ���� ��������");
		}
	}
	else
	{
		error = true;
		pushLog("�� ������� ��������� ��������� ����� ������ ������");
	}

	backup.open(mpath + "backup_users.txt");
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
		pushLog("��������� ��������� ����� ������ �������������");
		if (i < s_users)
		{
			error = true;
			pushLog("��������� ����� ������ ������������� ���� ��������");
		}
	}
	else
	{
		error = true;
		pushLog("�� ������� ��������� ��������� ����� ������ �������������");
	}

	backup.open(mpath + "backup_access.txt");
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
			pushLog("��������� ����� ������ ������������� ��� ������ ���� ��������");
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
				update_clientFiles(i);
				if (j < size_f)
				{
					error = true;
					pushLog("��������� ����� ������� ������� ���� ��������: ����������� ����� �����");
				}
			}

			backup.close();
			pushLog("��������� ��������� ����� ������� ������� � ������");
			if (i < size_u)
			{
				error = true;
				pushLog("��������� ����� ������� ������� ���� ��������: ����������� ����� ������������");
			}
		}
	}
	else
	{
		error = true;
		pushLog("�� ������� ��������� ��������� ����� ������� ������� � ������");
	}
	if (error)
	{
		files.clear();
		users.clear();
		access.clear();
		pushLog("��-�� ������ � ��������� ����� ������ ������ � ������������� ��������");

	}
	isOutDated_Files = true;
	isOutDated_Users = true;
	ReleaseMutex(hMutex_Users_Files);
	updateFiles_Users();
	return true;
}

#pragma region Get- � set-������
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