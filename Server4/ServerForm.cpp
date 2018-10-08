#include "ServerForm.h"
#include <clocale>

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void main() {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	Server4::ServerForm form;
	Application::Run(%form);
}