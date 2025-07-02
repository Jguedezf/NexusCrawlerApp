#pragma once

// Incluimos las cabeceras estándar que podríamos necesitar más adelante
#include <string>
#include <vector>

// Incluimos la cabecera de la API de Windows para usar funciones nativas como CreateRoundRectRgn
#include <Windows.h>

// **CORRECCIÓN FINAL**: Esta directiva le dice al ENLAZADOR (Linker) que debe incluir
// la librería Gdi32.lib, que contiene el código de la función CreateRoundRectRgn.
// Esto resuelve los errores LNK2019 y LNK2028.
#pragma comment(lib, "Gdi32.lib")

// Comentamos temporalmente la inclusión de la capa de negocio.
// #include "../BusinessLogicLayer/BusinessLogic.h"

namespace NexusCrawlerApp {

	// Declaramos los namespaces que usaremos
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void)
		{
			InitializeComponent();
			SwitchPanel(panelInicio);
		}

	protected:
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		// --- Declaración de Controles Visuales ---
		System::Windows::Forms::Panel^ panelInicio;
		System::Windows::Forms::Panel^ panelCarga;
		System::Windows::Forms::Panel^ panelResultados;
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
			   this->SuspendLayout();
			   this->panelInicio->SuspendLayout();
			   this->panelInputContainer->SuspendLayout();
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->BeginInit();

			   // --- Configuración del FORMULARIO PRINCIPAL (MyForm) ---
			   this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
			   this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
			   this->ClientSize = System::Drawing::Size(800, 600);
			   this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			   this->ForeColor = System::Drawing::Color::White;
			   this->Name = L"MyForm";
			   this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			   this->Text = L"NexusCrawlerApp";
			   this->Controls->Add(this->panelInicio);

			   // --- Configuración del PANEL DE INICIO (panelInicio) ---
			   this->panelInicio->Dock = System::Windows::Forms::DockStyle::Fill;
			   this->panelInicio->Location = System::Drawing::Point(0, 0);
			   this->panelInicio->Name = L"panelInicio";
			   this->panelInicio->Controls->Add(this->panelInputContainer);
			   this->panelInicio->Controls->Add(this->lblTitulo);
			   this->panelInicio->Controls->Add(this->cmbLanguage);

			   // ComboBox para seleccionar el idioma.
			   this->cmbLanguage->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(68)));
			   this->cmbLanguage->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			   this->cmbLanguage->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
			   this->cmbLanguage->ForeColor = System::Drawing::Color::White;
			   this->cmbLanguage->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Español", L"English" });
			   this->cmbLanguage->Location = System::Drawing::Point(650, 25);
			   this->cmbLanguage->Name = L"cmbLanguage";
			   this->cmbLanguage->Size = System::Drawing::Size(121, 25);
			   this->cmbLanguage->SelectedIndex = 0;

			   // Título principal de la aplicación.
			   this->lblTitulo->AutoSize = true;
			   this->lblTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 22, System::Drawing::FontStyle::Bold));
			   this->lblTitulo->Location = System::Drawing::Point(30, 20);
			   this->lblTitulo->Name = L"lblTitulo";
			   this->lblTitulo->Text = L"NexusCrawler - Analizador de Sitios Web";

			   // --- PANEL CONTENEDOR con esquinas redondeadas ---
			   this->panelInputContainer->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
			   this->panelInputContainer->Location = System::Drawing::Point(150, 150);
			   this->panelInputContainer->Name = L"panelInputContainer";
			   this->panelInputContainer->Size = System::Drawing::Size(500, 300);
			   this->panelInputContainer->Region = System::Drawing::Region::FromHrgn((IntPtr)CreateRoundRectRgn(0, 0, 500, 300, 20, 20));
			   this->panelInputContainer->Controls->Add(this->lblUrl);
			   this->panelInputContainer->Controls->Add(this->txtUrl);
			   this->panelInputContainer->Controls->Add(this->lblEjemploUrl);
			   this->panelInputContainer->Controls->Add(this->lblProfundidad);
			   this->panelInputContainer->Controls->Add(this->numProfundidad);
			   this->panelInputContainer->Controls->Add(this->panelBotonAnalisis);

			   // Controles dentro del panel contenedor
			   this->lblUrl->AutoSize = true;
			   this->lblUrl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->lblUrl->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(220)));
			   this->lblUrl->Location = System::Drawing::Point(40, 40);
			   this->lblUrl->Name = L"lblUrl";
			   this->lblUrl->Text = L"Ingrese la URL del sitio web a analizar:";
			   this->txtUrl->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
			   this->txtUrl->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->txtUrl->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->txtUrl->ForeColor = System::Drawing::Color::White;
			   this->txtUrl->Location = System::Drawing::Point(44, 64);
			   this->txtUrl->Name = L"txtUrl";
			   this->txtUrl->Size = System::Drawing::Size(412, 27);
			   this->txtUrl->Text = L"https://uneg.edu.ve";
			   this->lblEjemploUrl->AutoSize = true;
			   this->lblEjemploUrl->ForeColor = System::Drawing::Color::Gray;
			   this->lblEjemploUrl->Location = System::Drawing::Point(42, 94);
			   this->lblEjemploUrl->Name = L"lblEjemploUrl";
			   this->lblEjemploUrl->Text = L"Ej: https://www.uneg.edu.ve";
			   this->lblProfundidad->AutoSize = true;
			   this->lblProfundidad->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->lblProfundidad->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(220)));
			   this->lblProfundidad->Location = System::Drawing::Point(40, 140);
			   this->lblProfundidad->Name = L"lblProfundidad";
			   this->lblProfundidad->Text = L"Nivel de profundidad (0 para solo la página raíz):";
			   this->numProfundidad->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
			   this->numProfundidad->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			   this->numProfundidad->Font = (gcnew System::Drawing::Font(L"Segoe UI", 11.25F));
			   this->numProfundidad->ForeColor = System::Drawing::Color::White;
			   this->numProfundidad->Location = System::Drawing::Point(44, 164);
			   this->numProfundidad->Name = L"numProfundidad";
			   this->numProfundidad->Size = System::Drawing::Size(100, 27);
			   this->numProfundidad->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
			   this->panelBotonAnalisis->Location = System::Drawing::Point(150, 230);
			   this->panelBotonAnalisis->Name = L"panelBotonAnalisis";
			   this->panelBotonAnalisis->Size = System::Drawing::Size(200, 50);
			   this->panelBotonAnalisis->Cursor = System::Windows::Forms::Cursors::Hand;
			   this->panelBotonAnalisis->Region = System::Drawing::Region::FromHrgn((IntPtr)CreateRoundRectRgn(0, 0, 200, 50, 25, 25));
			   this->panelBotonAnalisis->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelBotonAnalisis_Paint);
			   this->panelBotonAnalisis->Click += gcnew System::EventHandler(this, &MyForm::btnIniciarAnalisis_Click);
			   this->panelBotonAnalisis->Controls->Add(this->lblBotonAnalisis);
			   this->lblBotonAnalisis->BackColor = System::Drawing::Color::Transparent;
			   this->lblBotonAnalisis->Dock = System::Windows::Forms::DockStyle::Fill;
			   this->lblBotonAnalisis->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Bold));
			   this->lblBotonAnalisis->ForeColor = System::Drawing::Color::White;
			   this->lblBotonAnalisis->Name = L"lblBotonAnalisis";
			   this->lblBotonAnalisis->Text = L"INICIAR ANÁLISIS";
			   this->lblBotonAnalisis->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			   this->lblBotonAnalisis->Click += gcnew System::EventHandler(this, &MyForm::btnIniciarAnalisis_Click);

			   // --- Reanudamos el diseño ---
			   (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->EndInit();
			   this->panelInputContainer->ResumeLayout(false);
			   this->panelInputContainer->PerformLayout();
			   this->panelInicio->ResumeLayout(false);
			   this->panelInicio->PerformLayout();
			   this->ResumeLayout(false);
		   }
#pragma endregion

		   // --- Lógica y Manejadores de Eventos ---
	private:
		System::Void btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
			MessageBox::Show("¡Ahora sí! ¡Compilación limpia y lista para la lógica!", "Éxito");
		}

	private:
		System::Void panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
			Graphics^ g = e->Graphics;
			System::Drawing::Rectangle rect = (cli::safe_cast<Panel^>(sender))->ClientRectangle;
			LinearGradientBrush^ brush = gcnew LinearGradientBrush(
				rect,
				Color::FromArgb(136, 95, 255),
				Color::FromArgb(95, 175, 255),
				LinearGradientMode::Horizontal);
			g->FillRectangle(brush, rect);
			delete brush;
		}

	private:
		void SwitchPanel(Panel^ panelToShow) {
			this->panelInicio->Visible = false;
			panelToShow->Visible = true;
		}
	};
}