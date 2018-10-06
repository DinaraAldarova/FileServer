#pragma once
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>
#include < Windows.h >

using namespace std;

enum class s { error, working };
enum class n { server, off, on };

#define size_buff 4096
#define size_block 10

struct info
{
	int name;
	n status;
	char buff[size_buff] = "";
	HANDLE stream; //��� ���� ������ � ��������, ��������� �����/������
	SOCKET sock;
	HANDLE mailslot;
	vector <string> files;
	CRITICAL_SECTION cs_buf;
};
struct loading_files
{
	string name;
	string f_access;
	vector<int> access_users;
};
class ServerInterLayer
{
#pragma region ��������
private:
	s status = s::error;
	SOCKET client_socket;//��������� ����� �����
	const u_short port = 665;
	list <string> log;
	vector <string> files;
	vector <string> users;
	vector <vector<bool>> access;
	vector <loading_files> loading;
	HANDLE hMutex_Log;
	HANDLE hMutex_Users_Files;

public:
	HOSTENT * hst;
	string IPv4;
	string path;
	vector <info> client_info = {};
	//�������� �� false, ������� true � �������� ��������� ����� ������
	bool isOutDated_Users = true;
	bool isOutDated_Files = true;
	//����������� ������ ��� ������ � client_info
	CRITICAL_SECTION cs_info;
#pragma endregion

#pragma region Get- � set-������
private:


public:
	int new_name();
	ServerInterLayer();
	s getStatus();
	void setStatus(s new_status);
	vector<string> getFiles();
	void setFile(string new_file);
	vector<string> getUsers();
	void setUser(string new_user);
	void setClient_info(info new_client_info);
	SOCKET getClient_socket();
	SOCKET setClient_socket(SOCKET new_client_socket);
	u_short getPort();
	void pushLog(string message);
	string popLog();
	bool Log_isEmpty();

#pragma endregion

#pragma region ������ ������ �������
private:
	//DWORD WINAPI initialize(LPVOID param);
	//DWORD WINAPI WorkWithClient(LPVOID client_socket);


public:
	bool init();
	bool update_clientFiles(int id);
	bool updateFiles_Users();
	bool sendFiles_Users(int id);
	int send_buff(int id);
	int receive(int id);
	bool new_user(int id);
	bool new_file(string name, string f_access, vector <int> access_users);
	int new_loading_file(string name, string f_access, vector <string> access_users, int id);
	bool uploadFile(int id);
	bool downloadFile(int id);
	bool save_backup();
	bool load_from_backup();
	bool quit_client(int id);
	int Exit();

#pragma endregion
};