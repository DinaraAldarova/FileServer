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
	HANDLE stream; //��� ���� ������ � ��������, ��������� �����/������
	SOCKET sock;
	HANDLE mailslot;
	string mpath;
};
class ServerInterLayer
{
#pragma region ��������
private:
	s status = s::error;
	list<string> files;
	list<string> users = {};

	string puth = "D:\\Client";
	SOCKET client_socket;
	sockaddr_in client_addr;
	u_short port = 665;
	HOSTENT * hst;
	//����������� ������ ��� ������ � client_info
	CRITICAL_SECTION cs_info;

public:
	char buff[4096] = "";
	list <info> client_info = {};
#pragma endregion

#pragma region Get- � set-������
private:


public:
	int new_ID();
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
	//char * getBuff();
	//void setBuff(char * new_buff);
	SOCKET getClient_socket();
	SOCKET setClient_socket(SOCKET new_client_socket);
	sockaddr_in getClient_addr();
	void setClient_addr(sockaddr_in new_client_addr);
	u_short getPort();
	void setPort(u_short new_port);
	HOSTENT * getHst();
	void setHst(HOSTENT * new_hst);
	CRITICAL_SECTION getCs_info();
	void setCs_info(CRITICAL_SECTION new_cs_info);

#pragma endregion

#pragma region ������ ������ �������
private:
	//DWORD WINAPI initialize(LPVOID param);
	//DWORD WINAPI WorkWithClient(LPVOID client_socket);
	void quit_client(int ID);
	int Exit();

public:
	void init();

#pragma endregion
};