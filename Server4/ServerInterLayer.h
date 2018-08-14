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
	HANDLE stream; //��� ���� ������ � ��������, ��������� �����/������
	SOCKET * sock;
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
	list <info> client_info = {};
	string puth = "D:\\Client";
	char buff[4096] = "";
	SOCKET client_socket;
	sockaddr_in client_addr;
	u_short port = 665;
	HOSTENT * hst;

	//����������� ������ ��� ������ � client_info
	CRITICAL_SECTION cs_info;

#pragma endregion

#pragma region Get- � set-������
private:
	void quit_client(int ID);
	int new_ID();

public:
	ServerInterLayer();
	s Status();

#pragma endregion

#pragma region ������ ������ �������
private:
	void init();
	DWORD WINAPI initialize(LPVOID param);
	DWORD WINAPI WorkWithClient(LPVOID client_socket);
	int Exit();

#pragma endregion
};