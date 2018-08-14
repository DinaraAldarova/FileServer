#include "ServerForm.h"
#include <clocale>

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void main() {
	setlocale(LC_ALL, "rus");
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	Server4::ServerForm form;
	Application::Run(%form);
}