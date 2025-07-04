#include "MyForm.h"
#include "BusinessLogic.h"
#include "DataAccess.h"

using namespace NexusCrawlerApp;

// --- Implementaci�n del Constructor y Destructor ---

MyForm::MyForm(void)
{
	InitializeComponent();
	crawler = new NavigationTree();
	SwitchPanel(panelInicio);
}

MyForm::~MyForm()
{
	if (components) {
		delete components;
	}
	delete crawler;
}


// --- Implementaci�n de los Manejadores de Eventos ---

System::Void MyForm::nuevoAnalisisToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	SwitchPanel(panelInicio);
}

System::Void MyForm::exportarArbolToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	if (crawler->getRoot() == nullptr) {
		MessageBox::Show("Primero debe realizar un an�lisis para poder exportar el �rbol.",
			"�rbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}

	SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
	saveFileDialog->Filter = "Archivos de Texto (*.txt)|*.txt|Todos los archivos (*.*)|*.*";
	saveFileDialog->Title = "Exportar �rbol de Navegaci�n";
	saveFileDialog->FileName = "analisis_arbol.txt";

	if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		msclr::interop::marshal_context context;
		std::string filePath = context.marshal_as<std::string>(saveFileDialog->FileName);

		if (DataAccess::exportTreeToFile(crawler->getRoot(), filePath)) {
			MessageBox::Show("El �rbol se ha exportado exitosamente a:\n" + saveFileDialog->FileName,
				"Exportaci�n Exitosa", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
		else {
			MessageBox::Show("Ocurri� un error al intentar guardar el archivo.",
				"Error de Exportaci�n", MessageBoxButtons::OK, MessageBoxIcon::Error);
		}
	}
}

System::Void MyForm::salirToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	this->Close();
}

System::Void MyForm::acercaDeToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e) {
	MessageBox::Show("NexusCrawlerApp v1.0\nDesarrollado por: Equipo 5\nProyecto de Estructuras de Datos, 2025.",
		"Acerca de NexusCrawlerApp", MessageBoxButtons::OK, MessageBoxIcon::Information);
}

System::Void MyForm::btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
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

System::Void MyForm::backgroundCrawler_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	CrawlArgs^ args = static_cast<CrawlArgs^>(e->Argument);
	msclr::interop::marshal_context context;
	std::string stdUrl = context.marshal_as<std::string>(args->Url);
	crawler->startCrawling(stdUrl, args->Depth);
}

System::Void MyForm::backgroundCrawler_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	if (e->Error != nullptr) {
		MessageBox::Show("Ocurri� un error durante el an�lisis: " + e->Error->Message, "Error");
		SwitchPanel(panelInicio);
	}
	else {
		AnalysisResult result = crawler->getAnalysisResult();

		this->lblUrlAnalizada->Text = "URL Ra�z Analizada: " + this->txtUrl->Text;
		this->lblProfundidadSolicitada->Text = "Profundidad de An�lisis Solicitada: " + this->numProfundidad->Value.ToString();
		this->lblTotalNodos->Text = "Total de Nodos/P�ginas Descubiertas: " + result.totalNodes;
		this->lblEnlacesInternos->Text = "Enlaces Internos Encontrados: " + result.internalLinks;
		this->lblEnlacesExternos->Text = "Enlaces Externos Encontrados: " + result.externalLinks;
		this->lblProfundidadReal->Text = "Profundidad M�xima Real del �rbol: " + result.maxDepth;

		SwitchPanel(panelResultados);
	}
}

System::Void MyForm::btnVolver_Click(System::Object^ sender, System::EventArgs^ e) {
	SwitchPanel(panelInicio);
}

System::Void MyForm::panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	System::Drawing::Rectangle rect = (cli::safe_cast<Panel^>(sender))->ClientRectangle;
	LinearGradientBrush^ brush = gcnew LinearGradientBrush(rect, Color::FromArgb(136, 95, 255), Color::FromArgb(95, 175, 255), LinearGradientMode::Horizontal);
	g->FillRectangle(brush, rect);
	delete brush;
}

void MyForm::SwitchPanel(Panel^ panelToShow) {
	this->panelInicio->Visible = false;
	this->panelCarga->Visible = false;
	this->panelResultados->Visible = false;
	panelToShow->Visible = true;

	if (this->menuStrip->Items->Count > 1) {
		bool isResultPanel = (panelToShow == panelResultados);
		System::Windows::Forms::ToolStripMenuItem^ accionesMenu = safe_cast<System::Windows::Forms::ToolStripMenuItem^>(this->menuStrip->Items[1]);
		accionesMenu->Enabled = isResultPanel;
	}
}

#pragma region Windows Form Designer generated code
void MyForm::InitializeComponent(void)
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
	this->menuStrip = (gcnew System::Windows::Forms::MenuStrip());
	System::Windows::Forms::ToolStripMenuItem^ archivoToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	System::Windows::Forms::ToolStripMenuItem^ nuevoAnalisisToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	System::Windows::Forms::ToolStripMenuItem^ salirToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	System::Windows::Forms::ToolStripMenuItem^ accionesToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	System::Windows::Forms::ToolStripMenuItem^ exportarArbolToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	System::Windows::Forms::ToolStripMenuItem^ ayudaToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	System::Windows::Forms::ToolStripMenuItem^ acercaDeToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
	this->panelInicio->SuspendLayout();
	this->panelInputContainer->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->BeginInit();
	this->panelCarga->SuspendLayout();
	this->panelResultados->SuspendLayout();
	this->menuStrip->SuspendLayout();
	this->SuspendLayout();
	// 
	// menuStrip
	// 
	this->menuStrip->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(55)), static_cast<System::Int32>(static_cast<System::Byte>(68)));
	this->menuStrip->ForeColor = System::Drawing::Color::White;
	this->menuStrip->ImageScalingSize = System::Drawing::Size(20, 20);
	this->menuStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {
		archivoToolStripMenuItem,
			accionesToolStripMenuItem,
			ayudaToolStripMenuItem
	});
	this->menuStrip->Location = System::Drawing::Point(0, 0);
	this->menuStrip->Name = L"menuStrip";
	this->menuStrip->Size = System::Drawing::Size(800, 28);
	this->menuStrip->TabIndex = 5;
	this->menuStrip->Text = L"menuStrip";
	// 
	// archivoToolStripMenuItem
	// 
	archivoToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) { nuevoAnalisisToolStripMenuItem, salirToolStripMenuItem });
	archivoToolStripMenuItem->Name = L"archivoToolStripMenuItem";
	archivoToolStripMenuItem->Text = L"&Archivo";
	// 
	// nuevoAnalisisToolStripMenuItem
	// 
	nuevoAnalisisToolStripMenuItem->Name = L"nuevoAnalisisToolStripMenuItem";
	nuevoAnalisisToolStripMenuItem->Text = L"&Nuevo An�lisis";
	nuevoAnalisisToolStripMenuItem->Click += gcnew System::EventHandler(this, &MyForm::nuevoAnalisisToolStripMenuItem_Click);
	// 
	// salirToolStripMenuItem
	// 
	salirToolStripMenuItem->Name = L"salirToolStripMenuItem";
	salirToolStripMenuItem->Text = L"&Salir";
	salirToolStripMenuItem->Click += gcnew System::EventHandler(this, &MyForm::salirToolStripMenuItem_Click);
	// 
	// accionesToolStripMenuItem
	// 
	accionesToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { exportarArbolToolStripMenuItem });
	accionesToolStripMenuItem->Name = L"accionesToolStripMenuItem";
	accionesToolStripMenuItem->Text = L"&Acciones";
	// 
	// exportarArbolToolStripMenuItem
	// 
	exportarArbolToolStripMenuItem->Name = L"exportarArbolToolStripMenuItem";
	exportarArbolToolStripMenuItem->Text = L"&Exportar �rbol...";
	exportarArbolToolStripMenuItem->Click += gcnew System::EventHandler(this, &MyForm::exportarArbolToolStripMenuItem_Click);
	// 
	// ayudaToolStripMenuItem
	// 
	ayudaToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { acercaDeToolStripMenuItem });
	ayudaToolStripMenuItem->Name = L"ayudaToolStripMenuItem";
	ayudaToolStripMenuItem->Text = L"A&yuda";
	// 
	// acercaDeToolStripMenuItem
	// 
	acercaDeToolStripMenuItem->Name = L"acercaDeToolStripMenuItem";
	acercaDeToolStripMenuItem->Text = L"&Acerca de...";
	acercaDeToolStripMenuItem->Click += gcnew System::EventHandler(this, &MyForm::acercaDeToolStripMenuItem_Click);
	// 
	// panelInicio
	// 
	this->panelInicio->Controls->Add(this->panelInputContainer);
	this->panelInicio->Controls->Add(this->lblTitulo);
	this->panelInicio->Controls->Add(this->cmbLanguage);
	this->panelInicio->Dock = System::Windows::Forms::DockStyle::Fill;
	this->panelInicio->Location = System::Drawing::Point(0, 28);
	this->panelInicio->Name = L"panelInicio";
	this->panelInicio->Size = System::Drawing::Size(800, 572);
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
	this->lblProfundidad->Text = L"Nivel de profundidad (0 para solo la p�gina ra�z):";
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
	this->lblBotonAnalisis->Text = L"INICIAR AN�LISIS";
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
	this->cmbLanguage->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Espa�ol", L"English" });
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
	this->panelCarga->Location = System::Drawing::Point(0, 28);
	this->panelCarga->Name = L"panelCarga";
	this->panelCarga->Size = System::Drawing::Size(800, 572);
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
	this->panelResultados->Location = System::Drawing::Point(0, 28);
	this->panelResultados->Name = L"panelResultados";
	this->panelResultados->Size = System::Drawing::Size(800, 572);
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
	this->lblResumenTitulo->Text = L">>> RESUMEN DEL AN�LISIS";
	// 
	// lblUrlAnalizada
	// 
	this->lblUrlAnalizada->AutoSize = true;
	this->lblUrlAnalizada->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
	this->lblUrlAnalizada->Location = System::Drawing::Point(53, 120);
	this->lblUrlAnalizada->Name = L"lblUrlAnalizada";
	this->lblUrlAnalizada->Size = System::Drawing::Size(183, 28);
	this->lblUrlAnalizada->TabIndex = 1;
	this->lblUrlAnalizada->Text = L"URL Ra�z Analizada:";
	// 
	// lblProfundidadSolicitada
	// 
	this->lblProfundidadSolicitada->AutoSize = true;
	this->lblProfundidadSolicitada->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
	this->lblProfundidadSolicitada->Location = System::Drawing::Point(53, 155);
	this->lblProfundidadSolicitada->Name = L"lblProfundidadSolicitada";
	this->lblProfundidadSolicitada->Size = System::Drawing::Size(326, 28);
	this->lblProfundidadSolicitada->TabIndex = 2;
	this->lblProfundidadSolicitada->Text = L"Profundidad de An�lisis Solicitada:";
	// 
	// lblTotalNodos
	// 
	this->lblTotalNodos->AutoSize = true;
	this->lblTotalNodos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12));
	this->lblTotalNodos->Location = System::Drawing::Point(53, 225);
	this->lblTotalNodos->Name = L"lblTotalNodos";
	this->lblTotalNodos->Size = System::Drawing::Size(349, 28);
	this->lblTotalNodos->TabIndex = 3;
	this->lblTotalNodos->Text = L"Total de Nodos/P�ginas Descubiertas:";
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
	this->lblProfundidadReal->Text = L"Profundidad M�xima Real del �rbol:";
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
	this->Controls->Add(this->menuStrip);
	this->MainMenuStrip = this->menuStrip;
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
	this->menuStrip->ResumeLayout(false);
	this->menuStrip->PerformLayout();
	this->ResumeLayout(false);
	this->PerformLayout();

}
#pragma endregion