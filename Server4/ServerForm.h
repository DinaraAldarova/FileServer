#pragma once
#include "ServerInterLayer.h"
namespace Server4 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
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
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ImageList^  imageListPlay;

	protected:

	private: System::Windows::Forms::Label^  label_Addres;
	private: System::Windows::Forms::ListView^  listView_Users;
	private: System::Windows::Forms::ListView^  listView_Files;




	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;


	private: System::Windows::Forms::Label^  label_IP;
	private: System::Windows::Forms::Label^  label_Addres2;


	private: System::Windows::Forms::ToolTip^  toolTip1;
	private: System::ComponentModel::IContainer^  components;

	protected:

	private:
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
			this->label_Addres = (gcnew System::Windows::Forms::Label());
			this->listView_Users = (gcnew System::Windows::Forms::ListView());
			this->listView_Files = (gcnew System::Windows::Forms::ListView());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->label_IP = (gcnew System::Windows::Forms::Label());
			this->label_Addres2 = (gcnew System::Windows::Forms::Label());
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->SuspendLayout();
			// 
			// imageListPlay
			// 
			this->imageListPlay->ImageStream = (cli::safe_cast<System::Windows::Forms::ImageListStreamer^>(resources->GetObject(L"imageListPlay.ImageStream")));
			this->imageListPlay->TransparentColor = System::Drawing::Color::Transparent;
			this->imageListPlay->Images->SetKeyName(0, L"play");
			this->imageListPlay->Images->SetKeyName(1, L"stop");
			// 
			// label_Addres
			// 
			this->label_Addres->AutoSize = true;
			this->label_Addres->Location = System::Drawing::Point(12, 9);
			this->label_Addres->Name = L"label_Addres";
			this->label_Addres->Size = System::Drawing::Size(98, 13);
			this->label_Addres->TabIndex = 14;
			this->label_Addres->Text = L"IP-адрес сервера:";
			// 
			// listView_Users
			// 
			this->listView_Users->LabelWrap = false;
			this->listView_Users->Location = System::Drawing::Point(12, 38);
			this->listView_Users->MultiSelect = false;
			this->listView_Users->Name = L"listView_Users";
			this->listView_Users->ShowGroups = false;
			this->listView_Users->Size = System::Drawing::Size(166, 211);
			this->listView_Users->Sorting = System::Windows::Forms::SortOrder::Ascending;
			this->listView_Users->TabIndex = 17;
			this->listView_Users->UseCompatibleStateImageBehavior = false;
			this->listView_Users->View = System::Windows::Forms::View::List;
			// 
			// listView_Files
			// 
			this->listView_Files->LabelWrap = false;
			this->listView_Files->Location = System::Drawing::Point(187, 38);
			this->listView_Files->MultiSelect = false;
			this->listView_Files->Name = L"listView_Files";
			this->listView_Files->ShowGroups = false;
			this->listView_Files->Size = System::Drawing::Size(165, 211);
			this->listView_Files->Sorting = System::Windows::Forms::SortOrder::Ascending;
			this->listView_Files->TabIndex = 18;
			this->listView_Files->UseCompatibleStateImageBehavior = false;
			this->listView_Files->View = System::Windows::Forms::View::List;
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(12, 22);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(127, 13);
			this->label3->TabIndex = 19;
			this->label3->Text = L"Список пользователей:";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(184, 22);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(88, 13);
			this->label4->TabIndex = 20;
			this->label4->Text = L"Список файлов:";
			// 
			// label_IP
			// 
			this->label_IP->AutoSize = true;
			this->label_IP->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Underline, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(204)));
			this->label_IP->Location = System::Drawing::Point(116, 9);
			this->label_IP->Name = L"label_IP";
			this->label_IP->Size = System::Drawing::Size(70, 13);
			this->label_IP->TabIndex = 21;
			this->label_IP->Text = L"192.168.56.1";
			// 
			// label_Addres2
			// 
			this->label_Addres2->AutoSize = true;
			this->label_Addres2->Location = System::Drawing::Point(270, 9);
			this->label_Addres2->Name = L"label_Addres2";
			this->label_Addres2->Size = System::Drawing::Size(79, 13);
			this->label_Addres2->TabIndex = 22;
			this->label_Addres2->Text = L"(или 127.0.0.1)";
			this->toolTip1->SetToolTip(this->label_Addres2, L"Только для клиента, запущенного на этом компьютере!");
			// 
			// ServerForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(361, 261);
			this->Controls->Add(this->label_Addres2);
			this->Controls->Add(this->label_IP);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->listView_Files);
			this->Controls->Add(this->listView_Users);
			this->Controls->Add(this->label_Addres);
			this->Name = L"ServerForm";
			this->Text = L"ServerForm";
			this->Load += gcnew System::EventHandler(this, &ServerForm::ServerForm_Load);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion

	private: System::Void ServerForm_Load(System::Object^  sender, System::EventArgs^  e)
	{

	}
	};
}
