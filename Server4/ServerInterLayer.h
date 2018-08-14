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
	int new_ID();

public:
	ServerInterLayer();
	s getStatus();
	void setStatus(s new_status);
	list<string> getFiles();
	void setFiles(string new_file);
	list<string> getUsers();
	void setUsers(string new_user);
	list <info> getClient_info();
	void setClient_info(info new_client_info);
	string getPuth();
	void setPuth(string new_puth);
	char * getBuff();
	//void setBuff(char * new_buff);
	SOCKET getClient_socket();
	void setClient_socket(SOCKET new_client_socket);
	sockaddr_in getClient_addr();
	void setClient_addr(sockaddr_in new_client_addr);
	u_short getPort();
	void setPort(u_short new_port);
	HOSTENT * getHst();
	void setHst(HOSTENT * new_hst);
	CRITICAL_SECTION getCs_info();
	void setCs_info(CRITICAL_SECTION new_cs_info);

#pragma endregion

#pragma region Логика работы сервера
private:
	void init();
	//DWORD WINAPI initialize(LPVOID param);
	//DWORD WINAPI WorkWithClient(LPVOID client_socket);
	void quit_client(int ID);
	int Exit();

#pragma endregion
};