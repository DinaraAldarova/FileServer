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
	

public:
	char buff[4096] = "";
	sockaddr_in client_addr;
	HOSTENT * hst;
	list <info> client_info = {};
	list<string> files;
	list<string> users = {};
	bool isOutDated_Users = false;
	bool isOutDated_Files = false;
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
	void setFiles(string new_file);
	void setUsers(string new_user);
	void setClient_info(info new_client_info);
	SOCKET getClient_socket();
	SOCKET setClient_socket(SOCKET new_client_socket);
	u_short getPort();

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