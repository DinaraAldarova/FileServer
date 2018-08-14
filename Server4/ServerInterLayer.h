#pragma once
#include <string>
#include <list>
#include <iostream>
#include <fstream>
#include < Windows.h >

using namespace std;

enum class s {error, working};

struct info
{
	int ID;
	string name;
	string IPv4;
	HANDLE stream; //где идет работа с клиентом, обработка входа/выхода
	SOCKET * sock;
	HANDLE mailslot;
	string mpath;
};
class ServerInterLayer
{
#pragma region Атрибуты
private:
	s status = s::error;
	list<string> files;
	list<string> users = {};
	list <info> client_info = {};
	string puth = "D:\\Client";
	char buff[4096] = "";
	SOCKET client_socket;
	sockaddr_in client_addr;
	u_short port = 665;
	HOSTENT * hst;

	//Критическая секция для работы с client_info
	CRITICAL_SECTION cs_info;

#pragma endregion

#pragma region Get- и set-методы
private:
	void quit_client(int ID);
	int new_ID();

public:
	ServerInterLayer();
	s Status();

#pragma endregion

#pragma region Логика работы сервера
private:
	void init();
	DWORD WINAPI initialize(LPVOID param);
	DWORD WINAPI WorkWithClient(LPVOID client_socket);
	int Exit();

#pragma endregion
};