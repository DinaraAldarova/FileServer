#pragma once
#include <Windows.h>
#include "ServerInterLayer.h"
namespace Server4 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Threading;
	ServerInterLayer server;
	/// <summary>
	/// Сводка для ServerForm
	/// </summary>
	public ref class ServerForm : public System::Windows::Forms::Form
	{
	public:
		ServerForm(void)
		{
			InitializeComponent();
			//
			//TODO: добавьте код конструктора
			//
		}

	protected:
		/// <summary>
		/// Освободить все используемые ресурсы.
		/// </summary>
		~ServerForm()
		{
			server.Exit();
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ImageList^  imageListPlay;
	private: System::Windows::Forms::ListView^  listView_Users;
	private: System::Windows::Forms::ListView^  listView_Files;
	private: System::Windows::Forms::Label^  label_IP;
	private: System::Windows::Forms::Label^  label_IP2;
	private: System::Windows::Forms::ToolTip^  toolTip1;
	private: System::Windows::Forms::TabControl^  tabControl1;
	private: System::Windows::Forms::TabPage^  tabPage1;
	private: System::Windows::Forms::TabPage^  tabPage2;
	private: System::Windows::Forms::TabPage^  tabPage3;

	private: System::Windows::Forms::GroupBox^  groupBoxIP;
	private: System::Windows::Forms::Label^  label_Log;


	private: System::ComponentModel::IContainer^  components;
			 /// <summary>
			 /// Обязательная переменная конструктора.
			 /// </summary>
#pragma region Windows Form Designer generated code
		/// <summary>
		/// Требуемый метод для поддержки конструктора — не изменяйте 
		/// содержимое этого метода с помощью редактора кода.
		/// </summary>
			 void InitializeComponent(void)
			 {
				 this->components = (gcnew System::ComponentModel::Container());
				 System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(ServerForm::typeid));
				 this->imageListPlay = (gcnew System::Windows::Forms::ImageList(this->components));
				 this->listView_Users = (gcnew System::Windows::Forms::ListView());
				 this->listView_Files = (gcnew System::Windows::Forms::ListView());
				 this->label_IP = (gcnew System::Windows::Forms::Label());
				 this->label_IP2 = (gcnew System::Windows::Forms::Label());
				 this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
				 this->tabControl1 = (gcnew System::Windows::Forms::TabControl());
				 this->tabPage1 = (gcnew System::Windows::Forms::TabPage());
				 this->tabPage2 = (gcnew System::Windows::Forms::TabPage());
				 this->tabPage3 = (gcnew System::Windows::Forms::TabPage());
				 this->label_Log = (gcnew System::Windows::Forms::Label());
				 this->groupBoxIP = (gcnew System::Windows::Forms::GroupBox());
				 this->tabControl1->SuspendLayout();
				 this->tabPage1->SuspendLayout();
				 this->tabPage2->SuspendLayout();
				 this->tabPage3->SuspendLayout();
				 this->groupBoxIP->SuspendLayout();
				 this->SuspendLayout();
				 // 
				 // imageListPlay
				 // 
				 this->imageListPlay->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^>(resources->GetObject(L"imageListPlay.ImageStream")));
				 this->imageListPlay->TransparentColor = System::Drawing::Color::Transparent;
				 this->imageListPlay->Images->SetKeyName(0, L"play");
				 this->imageListPlay->Images->SetKeyName(1, L"stop");
				 // 
				 // listView_Users
				 // 
				 this->listView_Users->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
				 this->listView_Users->Dock = System::Windows::Forms::DockStyle::Fill;
				 this->listView_Users->LabelWrap = false;
				 this->listView_Users->Location = System::Drawing::Point(3, 3);
				 this->listView_Users->MultiSelect = false;
				 this->listView_Users->Name = L"listView_Users";
				 this->listView_Users->ShowGroups = false;
				 this->listView_Users->Size = System::Drawing::Size(323, 167);
				 this->listView_Users->Sorting = System::Windows::Forms::SortOrder::Ascending;
				 this->listView_Users->TabIndex = 17;
				 this->listView_Users->UseCompatibleStateImageBehavior = false;
				 this->listView_Users->View = System::Windows::Forms::View::List;
				 // 
				 // listView_Files
				 // 
				 this->listView_Files->Dock = System::Windows::Forms::DockStyle::Fill;
				 this->listView_Files->LabelWrap = false;
				 this->listView_Files->Location = System::Drawing::Point(3, 3);
				 this->listView_Files->MultiSelect = false;
				 this->listView_Files->Name = L"listView_Files";
				 this->listView_Files->ShowGroups = false;
				 this->listView_Files->Size = System::Drawing::Size(323, 167);
				 this->listView_Files->Sorting = System::Windows::Forms::SortOrder::Ascending;
				 this->listView_Files->TabIndex = 18;
				 this->listView_Files->UseCompatibleStateImageBehavior = false;
				 this->listView_Files->View = System::Windows::Forms::View::List;
				 // 
				 // label_IP
				 // 
				 this->label_IP->AutoSize = true;
				 this->label_IP->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Underline, System::Drawing::GraphicsUnit::Point,
					 static_cast<System::Byte>(204)));
				 this->label_IP->Location = System::Drawing::Point(6, 16);
				 this->label_IP->Name = L"label_IP";
				 this->label_IP->Size = System::Drawing::Size(52, 13);
				 this->label_IP->TabIndex = 21;
				 this->label_IP->Text = L"127.0.0.1";
				 // 
				 // label_IP2
				 // 
				 this->label_IP2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
				 this->label_IP2->AutoSize = true;
				 this->label_IP2->Location = System::Drawing::Point(252, 16);
				 this->label_IP2->Name = L"label_IP2";
				 this->label_IP2->Size = System::Drawing::Size(79, 13);
				 this->label_IP2->TabIndex = 22;
				 this->label_IP2->Text = L"(или 127.0.0.1)";
				 this->toolTip1->SetToolTip(this->label_IP2, L"Только для клиента, запущенного на этом компьютере!");
				 // 
				 // tabControl1
				 // 
				 this->tabControl1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
					 | System::Windows::Forms::AnchorStyles::Left)
					 | System::Windows::Forms::AnchorStyles::Right));
				 this->tabControl1->Controls->Add(this->tabPage1);
				 this->tabControl1->Controls->Add(this->tabPage2);
				 this->tabControl1->Controls->Add(this->tabPage3);
				 this->tabControl1->Location = System::Drawing::Point(12, 50);
				 this->tabControl1->Name = L"tabControl1";
				 this->tabControl1->SelectedIndex = 0;
				 this->tabControl1->Size = System::Drawing::Size(337, 199);
				 this->tabControl1->TabIndex = 24;
				 this->tabControl1->Click += gcnew System::EventHandler(this, &ServerForm::tabControl1_Click);
				 // 
				 // tabPage1
				 // 
				 this->tabPage1->Controls->Add(this->listView_Users);
				 this->tabPage1->Location = System::Drawing::Point(4, 22);
				 this->tabPage1->Name = L"tabPage1";
				 this->tabPage1->Padding = System::Windows::Forms::Padding(3);
				 this->tabPage1->Size = System::Drawing::Size(329, 173);
				 this->tabPage1->TabIndex = 0;
				 this->tabPage1->Text = L"Пользователи";
				 this->tabPage1->UseVisualStyleBackColor = true;
				 // 
				 // tabPage2
				 // 
				 this->tabPage2->Controls->Add(this->listView_Files);
				 this->tabPage2->Location = System::Drawing::Point(4, 22);
				 this->tabPage2->Name = L"tabPage2";
				 this->tabPage2->Padding = System::Windows::Forms::Padding(3);
				 this->tabPage2->Size = System::Drawing::Size(329, 173);
				 this->tabPage2->TabIndex = 1;
				 this->tabPage2->Text = L"Файлы";
				 this->tabPage2->UseVisualStyleBackColor = true;
				 // 
				 // tabPage3
				 // 
				 this->tabPage3->Controls->Add(this->label_Log);
				 this->tabPage3->Location = System::Drawing::Point(4, 22);
				 this->tabPage3->Name = L"tabPage3";
				 this->tabPage3->Padding = System::Windows::Forms::Padding(3);
				 this->tabPage3->Size = System::Drawing::Size(329, 173);
				 this->tabPage3->TabIndex = 2;
				 this->tabPage3->Text = L"Сообщения";
				 this->tabPage3->UseVisualStyleBackColor = true;
				 // 
				 // label_Log
				 // 
				 this->label_Log->Dock = System::Windows::Forms::DockStyle::Fill;
				 this->label_Log->Location = System::Drawing::Point(3, 3);
				 this->label_Log->Name = L"label_Log";
				 this->label_Log->Size = System::Drawing::Size(323, 167);
				 this->label_Log->TabIndex = 2;
				 // 
				 // groupBoxIP
				 // 
				 this->groupBoxIP->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
					 | System::Windows::Forms::AnchorStyles::Right));
				 this->groupBoxIP->Controls->Add(this->label_IP);
				 this->groupBoxIP->Controls->Add(this->label_IP2);
				 this->groupBoxIP->Location = System::Drawing::Point(12, 12);
				 this->groupBoxIP->Name = L"groupBoxIP";
				 this->groupBoxIP->Size = System::Drawing::Size(337, 32);
				 this->groupBoxIP->TabIndex = 25;
				 this->groupBoxIP->TabStop = false;
				 this->groupBoxIP->Text = L"IP-адрес сервера";
				 // 
				 // ServerForm
				 // 
				 this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
				 this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
				 this->ClientSize = System::Drawing::Size(361, 261);
				 this->Controls->Add(this->groupBoxIP);
				 this->Controls->Add(this->tabControl1);
				 this->MinimumSize = System::Drawing::Size(300, 200);
				 this->Name = L"ServerForm";
				 this->Text = L"ServerForm";
				 this->Load += gcnew System::EventHandler(this, &ServerForm::ServerForm_Load);
				 this->tabControl1->ResumeLayout(false);
				 this->tabPage1->ResumeLayout(false);
				 this->tabPage2->ResumeLayout(false);
				 this->tabPage3->ResumeLayout(false);
				 this->groupBoxIP->ResumeLayout(false);
				 this->groupBoxIP->PerformLayout();
				 this->ResumeLayout(false);

			 }
#pragma endregion

	private: System::Void ServerForm_Load(System::Object^  sender, System::EventArgs^  e)
	{
		while (server.getStatus() != s::working)
		{
			Sleep(100);
		}
		label_IP->Text = gcnew String(server.IPv4.c_str());
	}
	private: void update_info()
	{
		if (server.isOutDated_Files)
		{
			//обновление списка доступных для скачивания файлов
			listView_Files->Items->Clear();
			ListViewItem ^ file;
			vector<string> files = server.getFiles();
			for each (string file_name in files)
			{
				file = gcnew ListViewItem(gcnew String(file_name.c_str()));
				listView_Files->Items->Add(file);
			}
		}
		if (server.isOutDated_Users)
		{
			//обновление списка доступных пользователей
			listView_Users->Items->Clear();
			ListViewItem ^ user;
			vector<string> users = server.getUsers();
			for each (string user_name in users)
			{
				user = gcnew ListViewItem(gcnew String(user_name.c_str()));
				listView_Users->Items->Add(user);
			}
		}
		while (!server.Log_isEmpty())
		{
			label_Log->Text += gcnew String((server.popLog() + "\n").c_str());
		}
	}
	private: System::Void tabControl1_Click(System::Object^  sender, System::EventArgs^  e) 
	{
		update_info();
		server.save_backup();
	}
};
}