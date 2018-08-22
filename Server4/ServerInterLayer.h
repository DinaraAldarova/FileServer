#pragma once
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include < Windows.h >

using namespace std;

enum class s { error, working };

struct info
{
	int ID;
	string name;
	string IPv4;
	char buff[4096] = "";
	HANDLE stream; //где идет работа с клиентом, обработка входа/выхода
	SOCKET sock;
	HANDLE mailslot;
	//Как я буду использовать путь?
	string mpath;
};
class ServerInterLayer
{
#pragma region Атрибуты
private:
	s status = s::error;
	SOCKET client_socket;//создавать новый сокет
	u_short port = 665;
	list <string> log = {"11111", "2222", "333", "44", "5"};
	list<string> files = {"f1", "f2", "f3", "f4", "f5"};
	list<string> users = {"u1", "u2", "u3", "u4", "u5"};
	HANDLE hMutex_Log;
	HANDLE hMutex_Users;
	HANDLE hMutex_Files;

public:
	sockaddr_in client_addr;
	HOSTENT * hst;
	list <info> client_info = {};
	//Заменить на false, ставить true в проверке резервной копии данных
	bool isOutDated_Users = true;
	bool isOutDated_Files = true;
	//Критическая секция для работы с client_info
	CRITICAL_SECTION cs_info;
	CRITICAL_SECTION cs_files;
	CRITICAL_SECTION cs_users;
#pragma endregion

#pragma region Get- и set-методы
private:


public:
	int new_ID();
	ServerInterLayer();
	s getStatus();
	void setStatus(s new_status);
	list<string> getFiles();
	void setFile(string new_file);
	list<string> getUsers();
	void setUser(string new_user);
	void setClient_info(info new_client_info);
	SOCKET getClient_socket();
	SOCKET setClient_socket(SOCKET new_client_socket);
	u_short getPort();
	void pushLog(string message);
	string popLog();
	bool Log_isEmpty();

#pragma endregion

#pragma region Логика работы сервера
private:
	//DWORD WINAPI initialize(LPVOID param);
	//DWORD WINAPI WorkWithClient(LPVOID client_socket);
	void quit_client(int ID);
	int Exit();

public:
	void init();

#pragma endregion
};