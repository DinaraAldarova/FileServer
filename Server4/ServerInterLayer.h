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

struct info
{
	int name;
	n status;
	char buff[size_buff] = "";
	HANDLE stream; //где идет работа с клиентом, обработка входа/выхода
	SOCKET sock;
	HANDLE mailslot;	
	vector <string> files = { "f1", "f2", "f3", "f4", "f5" };
};
class ServerInterLayer
{
#pragma region Атрибуты
private:
	s status = s::error;
	SOCKET client_socket;//создавать новый сокет
	const u_short port = 665;
	list <string> log = { "11111", "2222", "333", "44", "5" };
	vector <string> files = { "f1", "f2", "f3", "f4", "f5" };
	vector <string> users = { "u1", "u2", "u3", "u4", "u5" };
	vector<vector<bool>> access{ {true, true, false}, {true, false}, {false}, { } };
	HANDLE hMutex_Log;
	HANDLE hMutex_Users_Files;

public:
	HOSTENT * hst;
	string IPv4;
	string mpath;
	vector <info> client_info = {};
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

#pragma region Логика работы сервера
private:
	//DWORD WINAPI initialize(LPVOID param);
	//DWORD WINAPI WorkWithClient(LPVOID client_socket);
	void quit_client(int id);

public:
	void init();
	bool updateFiles(int id);
	bool sendFiles(int id);
	bool sendUsers(int id);
	int send_buff(int id);
	int receive(int id);
	void new_user(int name);
	bool save_backup();
	bool load_from_backup();
	int Exit();

#pragma endregion
};