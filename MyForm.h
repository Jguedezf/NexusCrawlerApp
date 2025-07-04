#pragma once

// --- Includes ---
#include <string>
#include <vector>
#include <Windows.h>
#pragma comment(lib, "Gdi32.lib")
#include "BusinessLogic.h"
#include <msclr/marshal_cppstd.h>

namespace NexusCrawlerApp {

	// --- Namespaces ---
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;

	// --- Estructura para Argumentos del Hilo ---
	public ref struct CrawlArgs {
		String^ Url;
		int Depth;
	};

	// --- Clase Principal del Formulario ---
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			crawler = new NavigationTree();
			// Ocultamos todos los paneles excepto el de inicio
			panelInicio->Visible = true;
			panelCarga->Visible = false;
			panelResultados->Visible = false;
		}

	protected:
		~MyForm()
		{
			if (components) { delete components; }
			delete crawler;
		}

	private:
		// --- Miembros de la Clase ---
		NavigationTree* crawler;
		System::ComponentModel::BackgroundWorker^ backgroundCrawler;

		// Paneles
		System::Windows::Forms::Panel^ panelInicio;
		System::Windows::Forms::Panel^ panelCarga;
		System::Windows::Forms::Panel^ panelResultados;

		// Controles de Inicio
		System::Windows::Forms::Label^ lblTitulo;
		System::Windows::Forms::Label^ lblUrl;
		System::Windows::Forms::TextBox^ txtUrl;
		System::Windows::Forms::Label^ lblProfundidad;
		System::Windows::Forms::NumericUpDown^ numProfundidad;
		System::Windows::Forms::Label^ lblEjemploUrl;
		System::Windows::Forms::ComboBox^ cmbLanguage;
		System::Windows::Forms::Panel^ panelInputContainer;
		System::Windows::Forms::Panel^ panelBotonAnalisis;
		System::Windows::Forms::Label^ lblBotonAnalisis;

		// Controles de Carga
		System::Windows::Forms::Label^ lblCargando;
		System::Windows::Forms::ProgressBar^ progressBar;

		// Controles del Panel de Resultados
		System::Windows::Forms::Label^ lblResumenTitulo;
		System::Windows::Forms::Label^ lblUrlAnalizada;
		System::Windows::Forms::Label^ lblProfundidadSolicitada;
		System::Windows::Forms::Label^ lblTotalNodos;
		System::Windows::Forms::Label^ lblEnlacesInternos;
		System::Windows::Forms::Label^ lblEnlacesExternos;
		System::Windows::Forms::Label^ lblProfundidadReal;
		System::Windows::Forms::Button^ btnVolver;

	private: System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		   void InitializeComponent(void)
		   {
			   this->components = (gcnew System::ComponentModel::Container());
			   this->panelInicio = (gcnew System::Windows::Forms::Panel());
			   this->panelInputContainer = (gcnew System::Windows::Forms::Panel());
			   this->lblTitulo = (gcnew System::Windows::Forms::Label());
			   this->lblUrl = (gcnew System::Windows::Forms::Label());
			   this->txtUrl = (gcnew System::Windows::Forms::TextBox());
			   this->lblEjemploUrl = (gcnew System::Windows::Forms::Label());
			   this->lblProfundidad = (gcnew System::Windows::Forms::Label());
			   this->numProfundidad = (gcnew System::Windows::Forms::NumericUpDown());
			   this->panelBotonAnalisis = (gcnew System::Windows::Forms::Panel());
			   this->lblBotonAnalisis = (gcnew System::Windows::Forms::Label());
			   this->cmbLanguage = (gcnew System::Windows::Forms::ComboBox());
			   this->panelCarga = (gcnew System::Windows::Forms::Panel());
			   this->lblCargando = (gcnew System::Windows::Forms::Label());
			   this->progressBar = (gcnew System::Windows::Forms::ProgressBar());
			   this->panelResultados = (gcnew System::Windows::Forms::Panel());
			   this->lblResumenTitulo = (gcnew System::Windows::Forms::Label());
			   this->lblUrlAnalizada = (gcnew System::Windows::Forms::Label());
			   this->lblProfundidadSolicitada = (gcnew System::Windows::Forms::Label());
			   this->lblTotalNodos = (gcnew System::Windows::Forms::Label());
			   this->lblEnlacesInternos = (gcnew System::Windows::Forms::Label());
			   this->lblEnlacesExternos = (gcnew System::Windows::Forms::Label());
			   this->lblProfundidadReal = (gcnew System::Windows::Forms::Label());
			   this->btnVolver = (gcnew System::Windows::Forms::Button());
			   this->backgroundCrawler = (gcnew System::ComponentModel::BackgroundWorker());
			   this->panelInicio->SuspendLayout();
			   this->panelInputContainer->SuspendLayout();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->BeginInit();
			   this->panelCarga->SuspendLayout();
			   this->panelResultados->SuspendLayout();
			   this->SuspendLayout();
			   // 
			   // panelInicio
			   // 
			   this->panelInicio->Controls->Add(this->panelInputContainer);
			   this->panelInicio->Controls->Add(this->lblTitulo);
			   this->panelInicio->Controls->Add(this->cmbLanguage);
			   this->panelInicio->Dock = System::Windows::Forms::DockStyle::Fill;
			   this->panelInicio->Location = System::Drawing::Point(0, 0);
			   this->panelInicio->Name = L"panelInicio";
			   this->panelInicio->Size = System::Drawing::Size(800, 600);
			   this->panelInicio->TabIndex = 0;
			   // 
			   // panelInputContainer
			   // 
			   this->panelInputContainer->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
			   this->panelInputContainer->Controls->Add(this->lblUrl);
			   this->panelInputContainer->Controls->Add(this->txtUrl);
			   this->panelInputContainer->Controls->Add(this->lblEjemploUrl);
			   this->panelInputContainer->Controls->Add(this->lblProfundidad);
			   this->panelInputContainer->Controls->Add(this->numProfundidad);
			   this->panelInputContainer->Controls->Add(this->panelBotonAnalisis);
			   this->panelInputContainer->Location = System::Drawing::Point(150, 150);
			   this->panelInputContainer->Name = L"panelInputContainer";
			   this->panelInputContainer->Size = System::Drawing::Size(500, 300);
			   this->panelInputContainer->TabIndex = 2;
			   this->panelInputContainer->Region = System::Drawing::Region::FromHrgn((IntPtr)CreateRoundRectRgn(0, 0, 500, 300, 20, 20));
			   // 
			   // lblTitulo
			   // 
			   this->lblTitulo->AutoSize = true;
			   this->lblTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 22.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			   this->lblTitulo->Location = System::Drawing::Point(30, 20);
			   this->lblTitulo->Name = L"lblTitulo";
			   this->lblTitulo->Size = System::Drawing::Size(588, 50);
			   this->lblTitulo->TabIndex = 1;
			   this->lblTitulo->Text = L"NexusCrawler - Analizador de Sitios Web";
			   // 
			   // lblUrl
			   // 
			   this->lblUrl->AutoSize = true;
			   this->lblUrl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->lblUrl->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(220)));
			   this->lblUrl->Location = System::Drawing::Point(40, 40);
			   this->lblUrl->Name = L"lblUrl";
			   this->lblUrl->Size = System::Drawing::Size(332, 25);
			   this->lblUrl->TabIndex = 0;
			   this->lblUrl->Text = L"Ingrese la URL del sitio web a analizar:";
			   // 
			   // txtUrl
			   // 
			   this->txtUrl->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
			   this->txtUrl->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->txtUrl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->txtUrl->ForeColor = System::Drawing::Color::White;
			   this->txtUrl->Location = System::Drawing::Point(44, 64);
			   this->txtUrl->Name = L"txtUrl";
			   this->txtUrl->Size = System::Drawing::Size(412, 32);
			   this->txtUrl->TabIndex = 1;
			   this->txtUrl->Text = L"https://uneg.edu.ve";
			   // 
			   // lblEjemploUrl
			   // 
			   this->lblEjemploUrl->AutoSize = true;
			   this->lblEjemploUrl->ForeColor = System::Drawing::Color::Gray;
			   this->lblEjemploUrl->Location = System::Drawing::Point(42, 94);
			   this->lblEjemploUrl->Name = L"lblEjemploUrl";
			   this->lblEjemploUrl->Size = System::Drawing::Size(222, 23);
			   this->lblEjemploUrl->TabIndex = 2;
			   this->lblEjemploUrl->Text = L"Ej: https://www.uneg.edu.ve";
			   // 
			   // lblProfundidad
			   // 
			   this->lblProfundidad->AutoSize = true;
			   this->lblProfundidad->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->lblProfundidad->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(220)));
			   this->lblProfundidad->Location = System::Drawing::Point(40, 140);
			   this->lblProfundidad->Name = L"lblProfundidad";
			   this->lblProfundidad->Size = System::Drawing::Size(377, 25);
			   this->lblProfundidad->TabIndex = 3;
			   this->lblProfundidad->Text = L"Nivel de profundidad (0 para solo la página raíz):";
			   // 
			   // numProfundidad
			   // 
			   this->numProfundidad->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
			   this->numProfundidad->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->numProfundidad->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->numProfundidad->ForeColor = System::Drawing::Color::White;
			   this->numProfundidad->Location = System::Drawing::Point(44, 164);
			   this->numProfundidad->Name = L"numProfundidad";
			   this->numProfundidad->Size = System::Drawing::Size(100, 32);
			   this->numProfundidad->TabIndex = 4;
			   this->numProfundidad->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
			   // 
			   // panelBotonAnalisis
			   // 
			   this->panelBotonAnalisis->Controls->Add(this->lblBotonAnalisis);
			   this->panelBotonAnalisis->Cursor = System::Windows::Forms::Cursors::Hand;
			   this->panelBotonAnalisis->Location = System::Drawing::Point(150, 230);
			   this->panelBotonAnalisis->Name = L"panelBotonAnalisis";
			   this->panelBotonAnalisis->Size = System::Drawing::Size(200, 50);
			   this->panelBotonAnalisis->TabIndex = 5;
			   this->panelBotonAnalisis->Region = System::Drawing::Region::FromHrgn((IntPtr)CreateRoundRectRgn(0, 0, 200, 50, 25, 25));
			   this->panelBotonAnalisis->Click += gcnew System::EventHandler(this, &MyForm::btnIniciarAnalisis_Click);
			   this->panelBotonAnalisis->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelBotonAnalisis_Paint);
			   // 
			   // lblBotonAnalisis
			   // 
			   this->lblBotonAnalisis->BackColor = System::Drawing::Color::Transparent;
			   this->lblBotonAnalisis->Dock = System::Windows::Forms::DockStyle::Fill;
			   this->lblBotonAnalisis->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			   this->lblBotonAnalisis->ForeColor = System::Drawing::Color::White;
			   this->lblBotonAnalisis->Location = System::Drawing::Point(0, 0);
			   this->lblBotonAnalisis->Name = L"lblBotonAnalisis";
			   this->lblBotonAnalisis->Size = System::Drawing::Size(200, 50);
			   this->lblBotonAnalisis->TabIndex = 0;
			   this->lblBotonAnalisis->Text = L"INICIAR ANÁLISIS";
			   this->lblBotonAnalisis->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			   this->lblBotonAnalisis->Click += gcnew System::EventHandler(this, &MyForm::btnIniciarAnalisis_Click);
			   // 
			   // cmbLanguage
			   // 
			   this->cmbLanguage->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(68)));
			   this->cmbLanguage->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			   this->cmbLanguage->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			   this->cmbLanguage->ForeColor = System::Drawing::Color::White;
			   this->cmbLanguage->FormattingEnabled = true;
			   this->cmbLanguage->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Español", L"English" });
			   this->cmbLanguage->Location = System::Drawing::Point(650, 25);
			   this->cmbLanguage->Name = L"cmbLanguage";
			   this->cmbLanguage->Size = System::Drawing::Size(121, 31);
			   this->cmbLanguage->TabIndex = 0;
			   this->cmbLanguage->SelectedIndex = 0;
			   // 
			   // panelCarga
			   // 
			   this->panelCarga->Controls->Add(this->progressBar);
			   this->panelCarga->Controls->Add(this->lblCargando);
			   this->panelCarga->Dock = System::Windows::Forms::DockStyle::Fill;
			   this->panelCarga->Location = System::Drawing::Point(0, 0);
			   this->panelCarga->Name = L"panelCarga";
			   this->panelCarga->Size = System::Drawing::Size(800, 600);
			   this->panelCarga->TabIndex = 3;
			   // 
			   // lblCargando
			   // 
			   this->lblCargando->Font = (gcnew System::Drawing::Font(L"Segoe UI", 18, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			   this->lblCargando->Location = System::Drawing::Point(0, 240);
			   this->lblCargando->Name = L"lblCargando";
			   this->lblCargando->Size = System::Drawing::Size(800, 40);
			   this->lblCargando->TabIndex = 1;
			   this->lblCargando->Text = L"Analizando sitio web, por favor espere...";
			   this->lblCargando->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			   // 
			   // progressBar
			   // 
			   this->progressBar->Location = System::Drawing::Point(150, 290);
			   this->progressBar->MarqueeAnimationSpeed = 30;
			   this->progressBar->Name = L"progressBar";
			   this->progressBar->Size = System::Drawing::Size(500, 23);
			   this->progressBar->Style = System::Windows::Forms::ProgressBarStyle::Marquee;
			   this->progressBar->TabIndex = 2;
			   // 
			   // panelResultados
			   // 
			   this->panelResultados->Controls->Add(this->btnVolver);
			   this->panelResultados->Controls->Add(this->lblProfundidadReal);
			   this->panelResultados->Controls->Add(this->lblEnlacesExternos);
			   this->panelResultados->Controls->Add(this->lblEnlacesInternos);
			   this->panelResultados->Controls->Add(this->lblTotalNodos);
			   this->panelResultados->Controls->Add(this->lblProfundidadSolicitada);
			   this->panelResultados->Controls->Add(this->lblUrlAnalizada);
			   this->panelResultados->Controls->Add(this->lblResumenTitulo);
			   this->panelResultados->Dock = System::Windows::Forms::DockStyle::Fill;
			   this->panelResultados->Location = System::Drawing::Point(0, 0);
			   this->panelResultados->Name = L"panelResultados";
			   this->panelResultados->Size = System::Drawing::Size(800, 600);
			   this->panelResultados->TabIndex = 4;
			   // 
			   // lblResumenTitulo
			   // 
			   this->lblResumenTitulo->AutoSize = true;
			   this->lblResumenTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 18, System::Drawing::FontStyle::Bold));
			   this->lblResumenTitulo->Location = System::Drawing::Point(50, 40);
			   this->lblResumenTitulo->Name = L"lblResumenTitulo";
			   this->lblResumenTitulo->Size = System::Drawing::Size(434, 41);
			   this->lblResumenTitulo->TabIndex = 0;
			   this->lblResumenTitulo->Text = L">>> RESUMEN DEL ANÁLISIS";
			   // 
			   // lblUrlAnalizada
			   // 
			   this->lblUrlAnalizada->AutoSize = true;
			   this->lblUrlAnalizada->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			   this->lblUrlAnalizada->Location = System::Drawing::Point(53, 120);
			   this->lblUrlAnalizada->Name = L"lblUrlAnalizada";
			   this->lblUrlAnalizada->Size = System::Drawing::Size(183, 28);
			   this->lblUrlAnalizada->TabIndex = 1;
			   this->lblUrlAnalizada->Text = L"URL Raíz Analizada:";
			   // 
			   // lblProfundidadSolicitada
			   // 
			   this->lblProfundidadSolicitada->AutoSize = true;
			   this->lblProfundidadSolicitada->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			   this->lblProfundidadSolicitada->Location = System::Drawing::Point(53, 155);
			   this->lblProfundidadSolicitada->Name = L"lblProfundidadSolicitada";
			   this->lblProfundidadSolicitada->Size = System::Drawing::Size(326, 28);
			   this->lblProfundidadSolicitada->TabIndex = 2;
			   this->lblProfundidadSolicitada->Text = L"Profundidad de Análisis Solicitada:";
			   // 
			   // lblTotalNodos
			   // 
			   this->lblTotalNodos->AutoSize = true;
			   this->lblTotalNodos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			   this->lblTotalNodos->Location = System::Drawing::Point(53, 225);
			   this->lblTotalNodos->Name = L"lblTotalNodos";
			   this->lblTotalNodos->Size = System::Drawing::Size(349, 28);
			   this->lblTotalNodos->TabIndex = 3;
			   this->lblTotalNodos->Text = L"Total de Nodos/Páginas Descubiertas:";
			   // 
			   // lblEnlacesInternos
			   // 
			   this->lblEnlacesInternos->AutoSize = true;
			   this->lblEnlacesInternos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			   this->lblEnlacesInternos->Location = System::Drawing::Point(53, 260);
			   this->lblEnlacesInternos->Name = L"lblEnlacesInternos";
			   this->lblEnlacesInternos->Size = System::Drawing::Size(282, 28);
			   this->lblEnlacesInternos->TabIndex = 4;
			   this->lblEnlacesInternos->Text = L"Enlaces Internos Encontrados:";
			   // 
			   // lblEnlacesExternos
			   // 
			   this->lblEnlacesExternos->AutoSize = true;
			   this->lblEnlacesExternos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			   this->lblEnlacesExternos->Location = System::Drawing::Point(53, 295);
			   this->lblEnlacesExternos->Name = L"lblEnlacesExternos";
			   this->lblEnlacesExternos->Size = System::Drawing::Size(286, 28);
			   this->lblEnlacesExternos->TabIndex = 5;
			   this->lblEnlacesExternos->Text = L"Enlaces Externos Encontrados:";
			   // 
			   // lblProfundidadReal
			   // 
			   this->lblProfundidadReal->AutoSize = true;
			   this->lblProfundidadReal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
			   this->lblProfundidadReal->Location = System::Drawing::Point(53, 330);
			   this->lblProfundidadReal->Name = L"lblProfundidadReal";
			   this->lblProfundidadReal->Size = System::Drawing::Size(325, 28);
			   this->lblProfundidadReal->TabIndex = 6;
			   this->lblProfundidadReal->Text = L"Profundidad Máxima Real del Árbol:";
			   // 
			   // btnVolver
			   // 
			   this->btnVolver->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(70)), static_cast<System::Int32>(static_cast<System::Byte>(90)));
			   this->btnVolver->FlatAppearance->BorderSize = 0;
			   this->btnVolver->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			   this->btnVolver->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			   this->btnVolver->Location = System::Drawing::Point(325, 520);
			   this->btnVolver->Name = L"btnVolver";
			   this->btnVolver->Size = System::Drawing::Size(150, 45);
			   this->btnVolver->TabIndex = 7;
			   this->btnVolver->Text = L"VOLVER";
			   this->btnVolver->UseVisualStyleBackColor = false;
			   this->btnVolver->Click += gcnew System::EventHandler(this, &MyForm::btnVolver_Click);
			   // 
			   // backgroundCrawler
			   // 
			   this->backgroundCrawler->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::backgroundCrawler_DoWork);
			   this->backgroundCrawler->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::backgroundCrawler_RunWorkerCompleted);
			   // 
			   // MyForm
			   // 
			   this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
			   this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
			   this->ClientSize = System::Drawing::Size(800, 600);
			   this->Controls->Add(this->panelResultados);
			   this->Controls->Add(this->panelCarga);
			   this->Controls->Add(this->panelInicio);
			   this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
			   this->ForeColor = System::Drawing::Color::White;
			   this->Name = L"MyForm";
			   this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			   this->Text = L"NexusCrawlerApp";
			   this->panelInicio->ResumeLayout(false);
			   this->panelInicio->PerformLayout();
			   this->panelInputContainer->ResumeLayout(false);
			   this->panelInputContainer->PerformLayout();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->EndInit();
			   this->panelCarga->ResumeLayout(false);
			   this->panelResultados->ResumeLayout(false);
			   this->panelResultados->PerformLayout();
			   this->ResumeLayout(false);

		   }
#pragma endregion

	private:
		System::Void btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
			if (String::IsNullOrWhiteSpace(this->txtUrl->Text)) {
				MessageBox::Show("Por favor, ingrese una URL.", "Error");
				return;
			}
			if (this->backgroundCrawler->IsBusy) return;

			SwitchPanel(panelCarga);
			CrawlArgs^ args = gcnew CrawlArgs();
			args->Url = this->txtUrl->Text;
			args->Depth = static_cast<int>(this->numProfundidad->Value);
			this->backgroundCrawler->RunWorkerAsync(args);
		}

	private:
		System::Void backgroundCrawler_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
			CrawlArgs^ args = static_cast<CrawlArgs^>(e->Argument);
			msclr::interop::marshal_context context;
			std::string stdUrl = context.marshal_as<std::string>(args->Url);
			crawler->startCrawling(stdUrl, args->Depth);
		}

	private:
		System::Void backgroundCrawler_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
			if (e->Error != nullptr) {
				MessageBox::Show("Ocurrió un error durante el análisis: " + e->Error->Message, "Error");
				SwitchPanel(panelInicio);
			}
			else {
				AnalysisResult result = crawler->getAnalysisResult();

				this->lblUrlAnalizada->Text = "URL Raíz Analizada: " + this->txtUrl->Text;
				this->lblProfundidadSolicitada->Text = "Profundidad de Análisis Solicitada: " + this->numProfundidad->Value.ToString();
				this->lblTotalNodos->Text = "Total de Nodos/Páginas Descubiertas: " + result.totalNodes;
				this->lblEnlacesInternos->Text = "Enlaces Internos Encontrados: " + result.internalLinks;
				this->lblEnlacesExternos->Text = "Enlaces Externos Encontrados: " + result.externalLinks;
				this->lblProfundidadReal->Text = "Profundidad Máxima Real del Árbol: " + result.maxDepth;

				SwitchPanel(panelResultados);
			}
		}

	private:
		System::Void btnVolver_Click(System::Object^ sender, System::EventArgs^ e) {
			SwitchPanel(panelInicio);
		}

	private:
		System::Void panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
			Graphics^ g = e->Graphics;
			System::Drawing::Rectangle rect = (cli::safe_cast<Panel^>(sender))->ClientRectangle;
			LinearGradientBrush^ brush = gcnew LinearGradientBrush(rect, Color::FromArgb(136, 95, 255), Color::FromArgb(95, 175, 255), LinearGradientMode::Horizontal);
			g->FillRectangle(brush, rect);
			delete brush;
		}

	private:
		void SwitchPanel(Panel^ panelToShow) {
			this->panelInicio->Visible = false;
			this->panelCarga->Visible = false;
			this->panelResultados->Visible = false;
			panelToShow->Visible = true;
		}
	};
}