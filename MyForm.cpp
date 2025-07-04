#include "MyForm.h"
#include "BusinessLogic.h"
#include "DataAccess.h"
#include <vector>
#include <string>

using namespace NexusCrawlerApp;

MyForm::MyForm(void)
{
	InitializeComponent();
	crawler = new NavigationTree();
	this->panelCarga->Visible = false;
	this->panelResultados->Visible = false;
	this->panelInicio->Visible = true;
	this->panelInicio->BringToFront();
}

MyForm::~MyForm()
{
	if (components) {
		delete components;
	}
	delete crawler;
}

System::Void MyForm::btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
	if (String::IsNullOrWhiteSpace(this->txtUrl->Text)) {
		MessageBox::Show("Por favor, ingrese una URL.", "Error");
		return;
	}
	if (this->crawlWorker->IsBusy) return;
	SwitchPanel(panelCarga);
	CrawlArgs^ args = gcnew CrawlArgs();
	args->Url = this->txtUrl->Text;
	args->Depth = static_cast<int>(this->numProfundidad->Value);
	this->crawlWorker->RunWorkerAsync(args);
}

System::Void MyForm::crawlWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	CrawlArgs^ args = static_cast<CrawlArgs^>(e->Argument);
	msclr::interop::marshal_context context;
	std::string stdUrl = context.marshal_as<std::string>(args->Url);
	crawler->startCrawling(stdUrl, args->Depth);
}

System::Void MyForm::crawlWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
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

System::Void MyForm::btnExportar_Click(System::Object^ sender, System::EventArgs^ e) {
	if (crawler->getRoot() == nullptr) {
		MessageBox::Show("Primero debe realizar un análisis.", "Árbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}
	SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
	saveFileDialog->Filter = "Archivos de Texto (*.txt)|*.txt|Todos los archivos (*.*)|*.*";
	saveFileDialog->Title = "Exportar Árbol de Navegación";
	saveFileDialog->FileName = "analisis_arbol.txt";
	if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		msclr::interop::marshal_context context;
		std::string filePath = context.marshal_as<std::string>(saveFileDialog->FileName);
		if (DataAccess::exportTreeToFile(crawler->getRoot(), filePath)) {
			MessageBox::Show("El árbol se ha exportado exitosamente.", "Exportación Exitosa", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
		else {
			MessageBox::Show("Ocurrió un error al guardar el archivo.", "Error de Exportación", MessageBoxButtons::OK, MessageBoxIcon::Error);
		}
	}
}

System::Void MyForm::btnDetectarRotos_Click(System::Object^ sender, System::EventArgs^ e) {
	if (crawler->getRoot() == nullptr) {
		MessageBox::Show("Primero debe realizar un análisis.", "Árbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}
	if (linkCheckWorker->IsBusy) return;
	this->grpAcciones->Enabled = false;
	this->lblAccionResultadoTitulo->Text = "Verificando enlaces...";
	this->rtbAccionResultado->Text = "Por favor espere...";
	linkCheckWorker->RunWorkerAsync();
}

System::Void MyForm::btnBuscarPalabra_Click(System::Object^ sender, System::EventArgs^ e) {
	MessageBox::Show("Funcionalidad 'Buscar Palabra Clave' pendiente de implementación.", "En Desarrollo");
}

System::Void MyForm::linkCheckWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	std::vector<std::string> brokenLinks = crawler->checkAllLinksStatus();
	System::Collections::Generic::List<String^>^ resultList = gcnew System::Collections::Generic::List<String^>();
	for (const auto& link : brokenLinks) {
		resultList->Add(gcnew String(link.c_str()));
	}
	e->Result = resultList;
}

System::Void MyForm::linkCheckWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	this->grpAcciones->Enabled = true;
	if (e->Error != nullptr) {
		this->lblAccionResultadoTitulo->Text = "Error en la Verificación";
		this->rtbAccionResultado->Text = "Ocurrió un error al verificar los enlaces.";
		return;
	}
	System::Collections::Generic::List<String^>^ brokenLinks = safe_cast<System::Collections::Generic::List<String^>^>(e->Result);
	this->lblAccionResultadoTitulo->Text = "Resultados de 'Enlaces Rotos':";
	this->rtbAccionResultado->Text = "";
	if (brokenLinks->Count == 0) {
		this->rtbAccionResultado->ForeColor = Color::LightGreen;
		this->rtbAccionResultado->SelectionFont = gcnew System::Drawing::Font("Segoe UI Emoji", 10);
		this->rtbAccionResultado->AppendText("✅ ");
		this->rtbAccionResultado->SelectionFont = gcnew System::Drawing::Font(this->rtbAccionResultado->Font->FontFamily, 10.2f);
		this->rtbAccionResultado->AppendText("¡Felicidades! No se encontraron enlaces rotos.");
	}
	else {
		this->rtbAccionResultado->ForeColor = Color::Gainsboro;
		this->rtbAccionResultado->AppendText("Se encontraron " + brokenLinks->Count + " enlaces rotos:\r\n\r\n");

		System::Drawing::Font^ emojiFont = gcnew System::Drawing::Font("Segoe UI Emoji", 10);
		System::Drawing::Font^ regularFont = gcnew System::Drawing::Font("Consolas", 10.2F);

		for each (String ^ link in brokenLinks) {
			rtbAccionResultado->SelectionFont = emojiFont;
			rtbAccionResultado->SelectionColor = Color::Tomato;
			rtbAccionResultado->AppendText("❌ ");

			rtbAccionResultado->SelectionFont = regularFont;
			rtbAccionResultado->SelectionColor = Color::Gainsboro;
			rtbAccionResultado->AppendText(link + "\r\n");
		}
	}
}

System::Void MyForm::btnVisualizarArbol_Click(System::Object^ sender, System::EventArgs^ e) {
	this->grpResumen->Visible = false;
	this->grpAcciones->Visible = false;
	this->grpAccionResultado->Visible = false;

	this->panelArbolGrafico->Visible = true;
	this->btnVolverResumen->Visible = true;
	this->panelArbolGrafico->BringToFront();
	this->btnVolverResumen->BringToFront();
	this->panelArbolGrafico->Invalidate();
}

System::Void MyForm::btnVolverResumen_Click(System::Object^ sender, System::EventArgs^ e) {
	this->grpResumen->Visible = true;
	this->grpAcciones->Visible = true;
	this->grpAccionResultado->Visible = true;

	this->panelArbolGrafico->Visible = false;
	this->btnVolverResumen->Visible = false;
}

void MyForm::SwitchPanel(Panel^ panelToShow) {
	this->panelInicio->Visible = false;
	this->panelCarga->Visible = false;
	this->panelResultados->Visible = false;
	panelToShow->Visible = true;
	panelToShow->BringToFront();
}

System::Void MyForm::panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	System::Drawing::Rectangle rect = (cli::safe_cast<Panel^>(sender))->ClientRectangle;
	LinearGradientBrush^ brush = gcnew LinearGradientBrush(rect, Color::FromArgb(136, 95, 255), Color::FromArgb(95, 175, 255), LinearGradientMode::Horizontal);
	g->FillRectangle(brush, rect);
	delete brush;
}

System::Void MyForm::panelArbolGrafico_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	g->SmoothingMode = SmoothingMode::AntiAlias;
	g->Clear(panelArbolGrafico->BackColor);
	if (crawler->getRoot() == nullptr) return;
	String^ texto = "Aquí se dibujará el árbol gráfico...";
	System::Drawing::Font^ font = gcnew System::Drawing::Font("Segoe UI", 12);
	Brush^ brush = gcnew SolidBrush(Color::White);
	g->DrawString(texto, font, brush, 20, 20);
}

System::Void MyForm::panelAccion_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	g->SmoothingMode = SmoothingMode::AntiAlias;
	Panel^ panel = cli::safe_cast<Panel^>(sender);
	System::Drawing::Rectangle rect = panel->ClientRectangle;
	GraphicsPath^ path = gcnew GraphicsPath();
	int cornerRadius = 15;
	path->AddArc(rect.X, rect.Y, cornerRadius, cornerRadius, 180, 90);
	path->AddArc(rect.Right - cornerRadius, rect.Y, cornerRadius, cornerRadius, 270, 90);
	path->AddArc(rect.Right - cornerRadius, rect.Bottom - cornerRadius, cornerRadius, cornerRadius, 0, 90);
	path->AddArc(rect.X, rect.Bottom - cornerRadius, cornerRadius, cornerRadius, 90, 90);
	path->CloseFigure();
	panel->Region = gcnew System::Drawing::Region(path);
	LinearGradientBrush^ brush = gcnew LinearGradientBrush(rect, Color::FromArgb(136, 95, 255), Color::FromArgb(95, 50, 230), LinearGradientMode::Vertical);
	g->FillPath(brush, path);
	delete brush;
	delete path;
}

#pragma region Windows Form Designer generated code
void MyForm::InitializeComponent(void)
{
	this->components = (gcnew System::ComponentModel::Container());
	this->panelInicio = (gcnew System::Windows::Forms::Panel());
	this->panelInputContainer = (gcnew System::Windows::Forms::Panel());
	this->lblUrl = (gcnew System::Windows::Forms::Label());
	this->txtUrl = (gcnew System::Windows::Forms::TextBox());
	this->lblEjemploUrl = (gcnew System::Windows::Forms::Label());
	this->lblProfundidad = (gcnew System::Windows::Forms::Label());
	this->numProfundidad = (gcnew System::Windows::Forms::NumericUpDown());
	this->panelBotonAnalisis = (gcnew System::Windows::Forms::Panel());
	this->lblBotonAnalisis = (gcnew System::Windows::Forms::Label());
	this->lblTitulo = (gcnew System::Windows::Forms::Label());
	this->cmbLanguage = (gcnew System::Windows::Forms::ComboBox());
	this->panelCarga = (gcnew System::Windows::Forms::Panel());
	this->progressBar = (gcnew System::Windows::Forms::ProgressBar());
	this->lblCargando = (gcnew System::Windows::Forms::Label());
	this->panelResultados = (gcnew System::Windows::Forms::Panel());
	this->grpResumen = (gcnew System::Windows::Forms::GroupBox());
	this->lblResumenTitulo = (gcnew System::Windows::Forms::Label());
	this->lblUrlAnalizada = (gcnew System::Windows::Forms::Label());
	this->lblProfundidadSolicitada = (gcnew System::Windows::Forms::Label());
	this->lblTotalNodos = (gcnew System::Windows::Forms::Label());
	this->lblEnlacesInternos = (gcnew System::Windows::Forms::Label());
	this->lblEnlacesExternos = (gcnew System::Windows::Forms::Label());
	this->lblProfundidadReal = (gcnew System::Windows::Forms::Label());
	this->grpAcciones = (gcnew System::Windows::Forms::GroupBox());
	this->panelBtnVisualizar = (gcnew System::Windows::Forms::Panel());
	this->lblBtnVisualizar = (gcnew System::Windows::Forms::Label());
	this->panelBtnBuscarPalabra = (gcnew System::Windows::Forms::Panel());
	this->lblBtnBuscarPalabra = (gcnew System::Windows::Forms::Label());
	this->panelBtnDetectarRotos = (gcnew System::Windows::Forms::Panel());
	this->lblBtnDetectarRotos = (gcnew System::Windows::Forms::Label());
	this->panelBtnExportar = (gcnew System::Windows::Forms::Panel());
	this->lblBtnExportar = (gcnew System::Windows::Forms::Label());
	this->grpAccionResultado = (gcnew System::Windows::Forms::GroupBox());
	this->txtPalabraClave = (gcnew System::Windows::Forms::TextBox());
	this->lblAccionResultadoTitulo = (gcnew System::Windows::Forms::Label());
	this->rtbAccionResultado = (gcnew System::Windows::Forms::RichTextBox());
	this->panelArbolGrafico = (gcnew System::Windows::Forms::Panel());
	this->btnVolverResumen = (gcnew System::Windows::Forms::Button());
	this->crawlWorker = (gcnew System::ComponentModel::BackgroundWorker());
	this->linkCheckWorker = (gcnew System::ComponentModel::BackgroundWorker());
	this->panelInicio->SuspendLayout();
	this->panelInputContainer->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->BeginInit();
	this->panelCarga->SuspendLayout();
	this->panelResultados->SuspendLayout();
	this->grpResumen->SuspendLayout();
	this->grpAcciones->SuspendLayout();
	this->panelBtnVisualizar->SuspendLayout();
	this->panelBtnBuscarPalabra->SuspendLayout();
	this->panelBtnDetectarRotos->SuspendLayout();
	this->panelBtnExportar->SuspendLayout();
	this->grpAccionResultado->SuspendLayout();
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
	this->panelInicio->Size = System::Drawing::Size(900, 700);
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
	this->panelInputContainer->Location = System::Drawing::Point(200, 150);
	this->panelInputContainer->Name = L"panelInputContainer";
	this->panelInputContainer->Size = System::Drawing::Size(500, 300);
	this->panelInputContainer->TabIndex = 2;
	this->panelInputContainer->Region = System::Drawing::Region::FromHrgn((IntPtr)CreateRoundRectRgn(0, 0, 500, 300, 20, 20));
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
	// cmbLanguage
	// 
	this->cmbLanguage->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(68)));
	this->cmbLanguage->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
	this->cmbLanguage->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->cmbLanguage->ForeColor = System::Drawing::Color::White;
	this->cmbLanguage->FormattingEnabled = true;
	this->cmbLanguage->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Español", L"English" });
	this->cmbLanguage->Location = System::Drawing::Point(750, 25);
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
	this->panelCarga->Size = System::Drawing::Size(900, 700);
	this->panelCarga->TabIndex = 3;
	// 
	// progressBar
	// 
	this->progressBar->Location = System::Drawing::Point(200, 338);
	this->progressBar->MarqueeAnimationSpeed = 30;
	this->progressBar->Name = L"progressBar";
	this->progressBar->Size = System::Drawing::Size(500, 23);
	this->progressBar->Style = System::Windows::Forms::ProgressBarStyle::Marquee;
	this->progressBar->TabIndex = 2;
	// 
	// lblCargando
	// 
	this->lblCargando->Font = (gcnew System::Drawing::Font(L"Segoe UI", 18, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
	this->lblCargando->Location = System::Drawing::Point(50, 288);
	this->lblCargando->Name = L"lblCargando";
	this->lblCargando->Size = System::Drawing::Size(800, 40);
	this->lblCargando->TabIndex = 1;
	this->lblCargando->Text = L"Analizando sitio web, por favor espere...";
	this->lblCargando->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	// 
	// panelResultados
	// 
	this->panelResultados->Controls->Add(this->btnVolverResumen);
	this->panelResultados->Controls->Add(this->panelArbolGrafico);
	this->panelResultados->Controls->Add(this->grpAccionResultado);
	this->panelResultados->Controls->Add(this->grpAcciones);
	this->panelResultados->Controls->Add(this->grpResumen);
	this->panelResultados->Dock = System::Windows::Forms::DockStyle::Fill;
	this->panelResultados->Location = System::Drawing::Point(0, 0);
	this->panelResultados->Name = L"panelResultados";
	this->panelResultados->Size = System::Drawing::Size(900, 700);
	this->panelResultados->TabIndex = 4;
	// 
	// grpResumen
	// 
	this->grpResumen->Controls->Add(this->lblResumenTitulo);
	this->grpResumen->Controls->Add(this->lblUrlAnalizada);
	this->grpResumen->Controls->Add(this->lblProfundidadSolicitada);
	this->grpResumen->Controls->Add(this->lblTotalNodos);
	this->grpResumen->Controls->Add(this->lblEnlacesInternos);
	this->grpResumen->Controls->Add(this->lblEnlacesExternos);
	this->grpResumen->Controls->Add(this->lblProfundidadReal);
	this->grpResumen->ForeColor = System::Drawing::Color::White;
	this->grpResumen->Location = System::Drawing::Point(20, 20);
	this->grpResumen->Name = L"grpResumen";
	this->grpResumen->Size = System::Drawing::Size(420, 240);
	this->grpResumen->TabIndex = 0;
	this->grpResumen->TabStop = false;
	this->grpResumen->Text = L"Resumen del Análisis";
	// 
	// lblResumenTitulo
	// 
	this->lblResumenTitulo->AutoSize = true;
	this->lblResumenTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 14, System::Drawing::FontStyle::Bold));
	this->lblResumenTitulo->Location = System::Drawing::Point(15, 25);
	this->lblResumenTitulo->Name = L"lblResumenTitulo";
	this->lblResumenTitulo->Size = System::Drawing::Size(331, 32);
	this->lblResumenTitulo->TabIndex = 0;
	this->lblResumenTitulo->Text = L">>> RESUMEN DEL ANÁLISIS";
	// 
	// lblUrlAnalizada
	// 
	this->lblUrlAnalizada->AutoSize = true;
	this->lblUrlAnalizada->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
	this->lblUrlAnalizada->Location = System::Drawing::Point(15, 70);
	this->lblUrlAnalizada->Name = L"lblUrlAnalizada";
	this->lblUrlAnalizada->Size = System::Drawing::Size(155, 23);
	this->lblUrlAnalizada->TabIndex = 1;
	this->lblUrlAnalizada->Text = L"URL Raíz Analizada:";
	// 
	// lblProfundidadSolicitada
	// 
	this->lblProfundidadSolicitada->AutoSize = true;
	this->lblProfundidadSolicitada->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
	this->lblProfundidadSolicitada->Location = System::Drawing::Point(15, 100);
	this->lblProfundidadSolicitada->Name = L"lblProfundidadSolicitada";
	this->lblProfundidadSolicitada->Size = System::Drawing::Size(262, 23);
	this->lblProfundidadSolicitada->TabIndex = 2;
	this->lblProfundidadSolicitada->Text = L"Profundidad de Análisis Solicitada:";
	// 
	// lblTotalNodos
	// 
	this->lblTotalNodos->AutoSize = true;
	this->lblTotalNodos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
	this->lblTotalNodos->Location = System::Drawing::Point(15, 130);
	this->lblTotalNodos->Name = L"lblTotalNodos";
	this->lblTotalNodos->Size = System::Drawing::Size(292, 23);
	this->lblTotalNodos->TabIndex = 3;
	this->lblTotalNodos->Text = L"Total de Nodos/Páginas Descubiertas:";
	// 
	// lblEnlacesInternos
	// 
	this->lblEnlacesInternos->AutoSize = true;
	this->lblEnlacesInternos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
	this->lblEnlacesInternos->Location = System::Drawing::Point(15, 160);
	this->lblEnlacesInternos->Name = L"lblEnlacesInternos";
	this->lblEnlacesInternos->Size = System::Drawing::Size(232, 23);
	this->lblEnlacesInternos->TabIndex = 4;
	this->lblEnlacesInternos->Text = L"Enlaces Internos Encontrados:";
	// 
	// lblEnlacesExternos
	// 
	this->lblEnlacesExternos->AutoSize = true;
	this->lblEnlacesExternos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
	this->lblEnlacesExternos->Location = System::Drawing::Point(15, 190);
	this->lblEnlacesExternos->Name = L"lblEnlacesExternos";
	this->lblEnlacesExternos->Size = System::Drawing::Size(236, 23);
	this->lblEnlacesExternos->TabIndex = 5;
	this->lblEnlacesExternos->Text = L"Enlaces Externos Encontrados:";
	// 
	// lblProfundidadReal
	// 
	this->lblProfundidadReal->AutoSize = true;
	this->lblProfundidadReal->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10));
	this->lblProfundidadReal->Location = System::Drawing::Point(15, 220);
	this->lblProfundidadReal->Name = L"lblProfundidadReal";
	this->lblProfundidadReal->Size = System::Drawing::Size(268, 23);
	this->lblProfundidadReal->TabIndex = 6;
	this->lblProfundidadReal->Text = L"Profundidad Máxima Real del Árbol:";
	// 
	// grpAcciones
	// 
	this->grpAcciones->Controls->Add(this->panelBtnVisualizar);
	this->grpAcciones->Controls->Add(this->panelBtnBuscarPalabra);
	this->grpAcciones->Controls->Add(this->panelBtnDetectarRotos);
	this->grpAcciones->Controls->Add(this->panelBtnExportar);
	this->grpAcciones->ForeColor = System::Drawing::Color::White;
	this->grpAcciones->Location = System::Drawing::Point(460, 20);
	this->grpAcciones->Name = L"grpAcciones";
	this->grpAcciones->Size = System::Drawing::Size(420, 120);
	this->grpAcciones->TabIndex = 1;
	this->grpAcciones->TabStop = false;
	this->grpAcciones->Text = L"Acciones";
	// 
	// panelBtnVisualizar
	// 
	this->panelBtnVisualizar->Controls->Add(this->lblBtnVisualizar);
	this->panelBtnVisualizar->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnVisualizar->Location = System::Drawing::Point(220, 70);
	this->panelBtnVisualizar->Name = L"panelBtnVisualizar";
	this->panelBtnVisualizar->Size = System::Drawing::Size(180, 40);
	this->panelBtnVisualizar->TabIndex = 3;
	this->panelBtnVisualizar->Click += gcnew System::EventHandler(this, &MyForm::btnVisualizarArbol_Click);
	this->panelBtnVisualizar->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelAccion_Paint);
	// 
	// lblBtnVisualizar
	// 
	this->lblBtnVisualizar->BackColor = System::Drawing::Color::Transparent;
	this->lblBtnVisualizar->Dock = System::Windows::Forms::DockStyle::Fill;
	this->lblBtnVisualizar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold));
	this->lblBtnVisualizar->ForeColor = System::Drawing::Color::White;
	this->lblBtnVisualizar->Name = L"lblBtnVisualizar";
	this->lblBtnVisualizar->Size = System::Drawing::Size(180, 40);
	this->lblBtnVisualizar->TabIndex = 0;
	this->lblBtnVisualizar->Text = L"VISUALIZAR ÁRBOL";
	this->lblBtnVisualizar->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnVisualizar->Click += gcnew System::EventHandler(this, &MyForm::btnVisualizarArbol_Click);
	// 
	// panelBtnBuscarPalabra
	// 
	this->panelBtnBuscarPalabra->Controls->Add(this->lblBtnBuscarPalabra);
	this->panelBtnBuscarPalabra->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnBuscarPalabra->Location = System::Drawing::Point(220, 25);
	this->panelBtnBuscarPalabra->Name = L"panelBtnBuscarPalabra";
	this->panelBtnBuscarPalabra->Size = System::Drawing::Size(180, 40);
	this->panelBtnBuscarPalabra->TabIndex = 2;
	this->panelBtnBuscarPalabra->Click += gcnew System::EventHandler(this, &MyForm::btnBuscarPalabra_Click);
	this->panelBtnBuscarPalabra->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelAccion_Paint);
	// 
	// lblBtnBuscarPalabra
	// 
	this->lblBtnBuscarPalabra->BackColor = System::Drawing::Color::Transparent;
	this->lblBtnBuscarPalabra->Dock = System::Windows::Forms::DockStyle::Fill;
	this->lblBtnBuscarPalabra->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold));
	this->lblBtnBuscarPalabra->ForeColor = System::Drawing::Color::White;
	this->lblBtnBuscarPalabra->Name = L"lblBtnBuscarPalabra";
	this->lblBtnBuscarPalabra->Size = System::Drawing::Size(180, 40);
	this->lblBtnBuscarPalabra->TabIndex = 0;
	this->lblBtnBuscarPalabra->Text = L"BUSCAR CLAVE";
	this->lblBtnBuscarPalabra->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnBuscarPalabra->Click += gcnew System::EventHandler(this, &MyForm::btnBuscarPalabra_Click);
	// 
	// panelBtnDetectarRotos
	// 
	this->panelBtnDetectarRotos->Controls->Add(this->lblBtnDetectarRotos);
	this->panelBtnDetectarRotos->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnDetectarRotos->Location = System::Drawing::Point(20, 25);
	this->panelBtnDetectarRotos->Name = L"panelBtnDetectarRotos";
	this->panelBtnDetectarRotos->Size = System::Drawing::Size(180, 40);
	this->panelBtnDetectarRotos->TabIndex = 1;
	this->panelBtnDetectarRotos->Click += gcnew System::EventHandler(this, &MyForm::btnDetectarRotos_Click);
	this->panelBtnDetectarRotos->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelAccion_Paint);
	// 
	// lblBtnDetectarRotos
	// 
	this->lblBtnDetectarRotos->BackColor = System::Drawing::Color::Transparent;
	this->lblBtnDetectarRotos->Dock = System::Windows::Forms::DockStyle::Fill;
	this->lblBtnDetectarRotos->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold));
	this->lblBtnDetectarRotos->ForeColor = System::Drawing::Color::White;
	this->lblBtnDetectarRotos->Name = L"lblBtnDetectarRotos";
	this->lblBtnDetectarRotos->Size = System::Drawing::Size(180, 40);
	this->lblBtnDetectarRotos->TabIndex = 0;
	this->lblBtnDetectarRotos->Text = L"ENLACES ROTOS";
	this->lblBtnDetectarRotos->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnDetectarRotos->Click += gcnew System::EventHandler(this, &MyForm::btnDetectarRotos_Click);
	// 
	// panelBtnExportar
	// 
	this->panelBtnExportar->Controls->Add(this->lblBtnExportar);
	this->panelBtnExportar->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnExportar->Location = System::Drawing::Point(20, 70);
	this->panelBtnExportar->Name = L"panelBtnExportar";
	this->panelBtnExportar->Size = System::Drawing::Size(180, 40);
	this->panelBtnExportar->TabIndex = 0;
	this->panelBtnExportar->Click += gcnew System::EventHandler(this, &MyForm::btnExportar_Click);
	this->panelBtnExportar->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelAccion_Paint);
	// 
	// lblBtnExportar
	// 
	this->lblBtnExportar->BackColor = System::Drawing::Color::Transparent;
	this->lblBtnExportar->Dock = System::Windows::Forms::DockStyle::Fill;
	this->lblBtnExportar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold));
	this->lblBtnExportar->ForeColor = System::Drawing::Color::White;
	this->lblBtnExportar->Name = L"lblBtnExportar";
	this->lblBtnExportar->Size = System::Drawing::Size(180, 40);
	this->lblBtnExportar->TabIndex = 0;
	this->lblBtnExportar->Text = L"EXPORTAR ÁRBOL";
	this->lblBtnExportar->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnExportar->Click += gcnew System::EventHandler(this, &MyForm::btnExportar_Click);
	// 
	// grpAccionResultado
	// 
	this->grpAccionResultado->Controls->Add(this->rtbAccionResultado);
	this->grpAccionResultado->Controls->Add(this->lblAccionResultadoTitulo);
	this->grpAccionResultado->Controls->Add(this->txtPalabraClave);
	this->grpAccionResultado->ForeColor = System::Drawing::Color::White;
	this->grpAccionResultado->Location = System::Drawing::Point(460, 150);
	this->grpAccionResultado->Name = L"grpAccionResultado";
	this->grpAccionResultado->Size = System::Drawing::Size(420, 240);
	this->grpAccionResultado->TabIndex = 3;
	this->grpAccionResultado->TabStop = false;
	this->grpAccionResultado->Text = L"Búsqueda y Resultados";
	// 
	// txtPalabraClave
	// 
	this->txtPalabraClave->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
	this->txtPalabraClave->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
	this->txtPalabraClave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
	this->txtPalabraClave->ForeColor = System::Drawing::Color::White;
	this->txtPalabraClave->Location = System::Drawing::Point(15, 30);
	this->txtPalabraClave->Name = L"txtPalabraClave";
	this->txtPalabraClave->Size = System::Drawing::Size(390, 30);
	this->txtPalabraClave->TabIndex = 2;
	this->txtPalabraClave->Text = L"Ingrese palabra clave aquí...";
	// 
	// lblAccionResultadoTitulo
	// 
	this->lblAccionResultadoTitulo->AutoSize = true;
	this->lblAccionResultadoTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F, System::Drawing::FontStyle::Bold));
	this->lblAccionResultadoTitulo->Location = System::Drawing::Point(15, 70);
	this->lblAccionResultadoTitulo->Name = L"lblAccionResultadoTitulo";
	this->lblAccionResultadoTitulo->Size = System::Drawing::Size(201, 23);
	this->lblAccionResultadoTitulo->TabIndex = 1;
	this->lblAccionResultadoTitulo->Text = L"Resultados de la Acción:";
	// 
	// rtbAccionResultado
	// 
	this->rtbAccionResultado->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
	this->rtbAccionResultado->BorderStyle = System::Windows::Forms::BorderStyle::None;
	this->rtbAccionResultado->Font = (gcnew System::Drawing::Font(L"Consolas", 10.2F));
	this->rtbAccionResultado->ForeColor = System::Drawing::Color::Gainsboro;
	this->rtbAccionResultado->Location = System::Drawing::Point(15, 100);
	this->rtbAccionResultado->Name = L"rtbAccionResultado";
	this->rtbAccionResultado->ReadOnly = true;
	this->rtbAccionResultado->Size = System::Drawing::Size(390, 125);
	this->rtbAccionResultado->TabIndex = 3;
	this->rtbAccionResultado->Text = L"";
	// 
	// panelArbolGrafico
	// 
	this->panelArbolGrafico->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left)
		| System::Windows::Forms::AnchorStyles::Right));
	this->panelArbolGrafico->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
	this->panelArbolGrafico->Location = System::Drawing::Point(20, 270);
	this->panelArbolGrafico->Name = L"panelArbolGrafico";
	this->panelArbolGrafico->Size = System::Drawing::Size(860, 400);
	this->panelArbolGrafico->TabIndex = 2;
	this->panelArbolGrafico->Visible = false;
	this->panelArbolGrafico->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelArbolGrafico_Paint);
	// 
	// btnVolverResumen
	// 
	this->btnVolverResumen->ForeColor = System::Drawing::Color::Black;
	this->btnVolverResumen->Location = System::Drawing::Point(20, 630);
	this->btnVolverResumen->Name = L"btnVolverResumen";
	this->btnVolverResumen->Size = System::Drawing::Size(180, 30);
	this->btnVolverResumen->TabIndex = 5;
	this->btnVolverResumen->Text = L"< Volver al Resumen";
	this->btnVolverResumen->Visible = false;
	this->btnVolverResumen->Click += gcnew System::EventHandler(this, &MyForm::btnVolverResumen_Click);
	// 
	// MyForm
	// 
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
	this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
	this->ClientSize = System::Drawing::Size(900, 700);
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
	this->grpResumen->ResumeLayout(false);
	this->grpResumen->PerformLayout();
	this->grpAcciones->ResumeLayout(false);
	this->panelBtnBuscarPalabra->ResumeLayout(false);
	this->panelBtnDetectarRotos->ResumeLayout(false);
	this->panelBtnExportar->ResumeLayout(false);
	this->panelBtnVisualizar->ResumeLayout(false);
	this->grpAccionResultado->ResumeLayout(false);
	this->grpAccionResultado->PerformLayout();
	this->ResumeLayout(false);
	//
	// crawlWorker
	//
	this->crawlWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::crawlWorker_DoWork);
	this->crawlWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::crawlWorker_RunWorkerCompleted);
	//
	// linkCheckWorker
	//
	this->linkCheckWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::linkCheckWorker_DoWork);
	this->linkCheckWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::linkCheckWorker_RunWorkerCompleted);
}
#pragma endregion