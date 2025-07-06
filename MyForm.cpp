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
	this->zoomLevel = 1.0f;
	this->panOffset = PointF(20, 20); // Margen inicial
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


// --- Handlers de Eventos ---

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
	if (crawler->getRoot() == nullptr) {
		MessageBox::Show("Primero debe realizar un análisis.", "Árbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}
	if (String::IsNullOrWhiteSpace(txtPalabraClave->Text) || txtPalabraClave->Text == "Ingrese palabra clave aquí...") {
		MessageBox::Show("Por favor, ingrese una palabra clave para buscar.", "Entrada Inválida", MessageBoxButtons::OK, MessageBoxIcon::Warning);
		return;
	}
	if (searchWorker->IsBusy) return;
	this->grpAcciones->Enabled = false;
	this->lblAccionResultadoTitulo->Text = "Buscando palabra clave...";
	this->rtbAccionResultado->Text = "Por favor espere...";
	SearchArgs^ args = gcnew SearchArgs();
	args->Keyword = this->txtPalabraClave->Text;
	searchWorker->RunWorkerAsync(args);
}

System::Void MyForm::linkCheckWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	std::vector<std::string> brokenLinks = crawler->checkAllLinksStatus();
	List<String^>^ resultList = gcnew List<String^>();
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
	List<String^>^ brokenLinks = safe_cast<List<String^>^>(e->Result);
	this->lblAccionResultadoTitulo->Text = "Resultados de 'Enlaces Rotos':";
	this->rtbAccionResultado->Clear();
	if (brokenLinks->Count == 0) {
		this->rtbAccionResultado->SelectionColor = Color::LightGreen;
		this->rtbAccionResultado->AppendText("[OK] ");
		this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
		this->rtbAccionResultado->AppendText("¡Felicidades! No se encontraron enlaces rotos.");
	}
	else {
		this->rtbAccionResultado->AppendText("Se encontraron " + brokenLinks->Count + " enlaces rotos:\r\n\r\n");
		for each (String ^ link in brokenLinks) {
			rtbAccionResultado->SelectionColor = Color::Tomato;
			rtbAccionResultado->AppendText("[ROTO] ");
			rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			rtbAccionResultado->AppendText(link + "\r\n");
		}
	}
}

System::Void MyForm::searchWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	SearchArgs^ args = static_cast<SearchArgs^>(e->Argument);
	msclr::interop::marshal_context context;
	std::string keyword = context.marshal_as<std::string>(args->Keyword);
	PathResult result_nativa = crawler->findShortestPathToKeyword(keyword);
	PathResultManaged^ result_gestionado = gcnew PathResultManaged();
	result_gestionado->found = result_nativa.found;
	result_gestionado->path = gcnew List<String^>();
	if (result_nativa.found) {
		for (const auto& url : result_nativa.path) {
			result_gestionado->path->Add(gcnew String(url.c_str()));
		}
	}
	e->Result = result_gestionado;
}

System::Void MyForm::searchWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	this->grpAcciones->Enabled = true;
	if (e->Error != nullptr) {
		this->lblAccionResultadoTitulo->Text = "Error en la Búsqueda";
		this->rtbAccionResultado->Text = "Ocurrió un error durante la búsqueda.";
		return;
	}
	PathResultManaged^ result = safe_cast<PathResultManaged^>(e->Result);
	this->lblAccionResultadoTitulo->Text = "Resultados de Búsqueda para '" + txtPalabraClave->Text + "':";
	this->rtbAccionResultado->Clear();
	if (!result->found) {
		this->rtbAccionResultado->SelectionColor = Color::Orange;
		this->rtbAccionResultado->AppendText("[NO ENCONTRADO] ");
		this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
		this->rtbAccionResultado->AppendText("La palabra clave no se encontró en ninguna URL del árbol.");
	}
	else {
		int clicks = result->path->Count - 1;
		this->rtbAccionResultado->SelectionColor = Color::LightGreen;
		this->rtbAccionResultado->AppendText("[ÉXITO] ");
		this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
		this->rtbAccionResultado->AppendText("Encontrado en " + clicks + " clic(s).\r\n\r\n");
		this->rtbAccionResultado->AppendText("Ruta más corta:\r\n");
		for (int i = 0; i < result->path->Count; ++i) {
			rtbAccionResultado->AppendText((gcnew String(L' ', i * 2)) + "-> " + result->path[i] + "\r\n");
		}
	}
}

System::Void MyForm::rtbAccionResultado_LinkClicked(System::Object^ sender, System::Windows::Forms::LinkClickedEventArgs^ e) {
	try {
		System::Diagnostics::Process::Start(e->LinkText);
	}
	catch (Exception^ ex) {
		MessageBox::Show("No se pudo abrir el enlace: " + ex->Message, "Error");
	}
}

System::Void MyForm::txtPalabraClave_Enter(System::Object^ sender, System::EventArgs^ e) {
	if (this->txtPalabraClave->Text == "Ingrese palabra clave aquí...") {
		this->txtPalabraClave->Text = "";
		this->txtPalabraClave->ForeColor = Color::White;
	}
}

System::Void MyForm::txtPalabraClave_Leave(System::Object^ sender, System::EventArgs^ e) {
	if (String::IsNullOrWhiteSpace(this->txtPalabraClave->Text)) {
		this->txtPalabraClave->Text = "Ingrese palabra clave aquí...";
		this->txtPalabraClave->ForeColor = Color::Gray;
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

	if (crawler->getRoot() == nullptr) {
		String^ texto = "No hay árbol para mostrar. Realice un análisis primero.";
		System::Drawing::Font^ font = gcnew System::Drawing::Font("Segoe UI", 12, FontStyle::Italic);
		Brush^ brush = gcnew SolidBrush(Color::Gray);
		StringFormat^ sf = gcnew StringFormat();
		sf->Alignment = StringAlignment::Center;
		sf->LineAlignment = StringAlignment::Center;
		g->DrawString(texto, font, brush, panelArbolGrafico->ClientRectangle, sf);
		return;
	}

	const std::vector<DrawableNodeInfo>& drawableTree = crawler->getDrawableTree();
	if (drawableTree.empty()) return;

	g->TranslateTransform(panOffset.X, panOffset.Y);
	g->ScaleTransform(zoomLevel, zoomLevel);

	Pen^ linePen = gcnew Pen(Color::FromArgb(100, 100, 120), 1.5f);
	Brush^ nodeBrushInternal = gcnew SolidBrush(Color::FromArgb(95, 175, 255));
	Brush^ nodeBrushExternal = gcnew SolidBrush(Color::FromArgb(100, 100, 120));
	Brush^ nodeBrushBroken = gcnew SolidBrush(Color::Tomato);
	Brush^ textBrush = gcnew SolidBrush(Color::White);
	System::Drawing::Font^ nodeFont = gcnew System::Drawing::Font("Segoe UI", 8);
	float nodeWidth = 120.0f;
	float nodeHeight = 40.0f;
	float x_spacing = 140.0f;
	float y_spacing = 100.0f;

	std::map<WebNode*, PointF> positions;
	for (const auto& dNodeInfo : drawableTree) {
		positions[dNodeInfo.nodePtr] = PointF(dNodeInfo.x * x_spacing, dNodeInfo.y * y_spacing);
	}

	for (const auto& dNodeInfo : drawableTree) {
		if (dNodeInfo.nodePtr == crawler->getRoot()) continue;
		WebNode* parent = nullptr;
		for (const auto& pInfo : drawableTree) {
			bool found = false;
			for (WebNode* child : pInfo.nodePtr->children) {
				if (child == dNodeInfo.nodePtr) {
					parent = pInfo.nodePtr;
					found = true;
					break;
				}
			}
			if (found) break;
		}

		if (parent && positions.count(parent) > 0) {
			PointF parentPos = positions[parent];
			PointF childPos = positions[dNodeInfo.nodePtr];
			g->DrawLine(linePen,
				parentPos.X + nodeWidth / 2,
				parentPos.Y + nodeHeight,
				childPos.X + nodeWidth / 2,
				childPos.Y);
		}
	}

	for (const auto& dNodeInfo : drawableTree) {
		PointF currentPos = positions[dNodeInfo.nodePtr];
		RectangleF nodeRect(currentPos.X, currentPos.Y, nodeWidth, nodeHeight);

		Brush^ currentBrush;
		if (dNodeInfo.nodePtr->status == LinkStatus::Broken) currentBrush = nodeBrushBroken;
		else if (dNodeInfo.nodePtr->type == LinkType::Internal) currentBrush = nodeBrushInternal;
		else currentBrush = nodeBrushExternal;

		g->FillEllipse(currentBrush, nodeRect);

		String^ url = gcnew String(dNodeInfo.nodePtr->url.c_str());
		String^ displayText = url;
		try {
			Uri^ uri = gcnew Uri(url);
			displayText = uri->PathAndQuery;
			if (displayText->Length > 15) {
				displayText = "..." + displayText->Substring(displayText->Length - 12);
			}
		}
		catch (Exception^) {
			if (displayText->Length > 18) displayText = displayText->Substring(0, 15) + "...";
		}

		StringFormat^ sf = gcnew StringFormat();
		sf->Alignment = StringAlignment::Center;
		sf->LineAlignment = StringAlignment::Center;
		g->DrawString(displayText, nodeFont, textBrush, nodeRect, sf);
	}
}

System::Void MyForm::panelArbolGrafico_MouseWheel(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
	if (e->Delta > 0)
		zoomLevel *= 1.1f;
	else
		zoomLevel /= 1.1f;
	zoomLevel = Math::Max(0.1f, Math::Min(zoomLevel, 5.0f));
	panelArbolGrafico->Invalidate();
}

System::Void MyForm::panelArbolGrafico_MouseDown(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
	if (e->Button == System::Windows::Forms::MouseButtons::Middle || e->Button == System::Windows::Forms::MouseButtons::Left) {
		lastMousePos = e->Location;
	}
}

System::Void MyForm::panelArbolGrafico_MouseMove(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e) {
	if (e->Button == System::Windows::Forms::MouseButtons::Middle || e->Button == System::Windows::Forms::MouseButtons::Left) {
		panOffset.X += e->X - lastMousePos.X;
		panOffset.Y += e->Y - lastMousePos.Y;
		lastMousePos = e->Location;
		panelArbolGrafico->Invalidate();
	}
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

System::Void MyForm::btnNuevoAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
	System::Windows::Forms::DialogResult confirmResult = MessageBox::Show(
		"¿Está seguro de que desea iniciar un nuevo análisis? Se perderán los resultados actuales.",
		"Confirmar Nuevo Análisis",
		MessageBoxButtons::YesNo,
		MessageBoxIcon::Question);

	if (confirmResult == System::Windows::Forms::DialogResult::Yes) {
		// 2. Limpiar el estado de la capa de negocio
		// Esto es CRUCIAL. Debemos liberar el árbol viejo y resetear el estado.
		// Asumiendo que `startCrawling` ya maneja la limpieza del árbol anterior, 
		// lo cual es una buena práctica y tu código parece hacerlo.
		// Si no, necesitaríamos un método `crawler->reset()`

		// 3. Limpiar la UI de resultados
		this->rtbAccionResultado->Clear();
		this->lblAccionResultadoTitulo->Text = "Resultados de la Acción:";
		this->txtPalabraClave->Text = "Ingrese palabra clave aquí...";
		this->txtPalabraClave->ForeColor = Color::Gray;

		// 4. Volver al panel de inicio
		SwitchPanel(panelInicio);
	}
}

#pragma region Windows Form Designer generated code
void MyForm::InitializeComponent(void)
{
	this->components = (gcnew System::ComponentModel::Container());
	this->crawlWorker = (gcnew System::ComponentModel::BackgroundWorker());
	this->linkCheckWorker = (gcnew System::ComponentModel::BackgroundWorker());
	this->searchWorker = (gcnew System::ComponentModel::BackgroundWorker());
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
	this->btnVolverResumen = (gcnew System::Windows::Forms::Button());
	this->grpAccionResultado = (gcnew System::Windows::Forms::GroupBox());
	this->rtbAccionResultado = (gcnew System::Windows::Forms::RichTextBox());
	this->lblAccionResultadoTitulo = (gcnew System::Windows::Forms::Label());
	this->txtPalabraClave = (gcnew System::Windows::Forms::TextBox());
	this->grpAcciones = (gcnew System::Windows::Forms::GroupBox());
	this->panelBtnVisualizar = (gcnew System::Windows::Forms::Panel());
	this->lblBtnVisualizar = (gcnew System::Windows::Forms::Label());
	this->panelBtnBuscarPalabra = (gcnew System::Windows::Forms::Panel());
	this->lblBtnBuscarPalabra = (gcnew System::Windows::Forms::Label());
	this->panelBtnDetectarRotos = (gcnew System::Windows::Forms::Panel());
	this->lblBtnDetectarRotos = (gcnew System::Windows::Forms::Label());
	this->panelBtnExportar = (gcnew System::Windows::Forms::Panel());
	this->lblBtnExportar = (gcnew System::Windows::Forms::Label());
	this->grpResumen = (gcnew System::Windows::Forms::GroupBox());
	this->lblResumenTitulo = (gcnew System::Windows::Forms::Label());
	this->lblUrlAnalizada = (gcnew System::Windows::Forms::Label());
	this->lblProfundidadSolicitada = (gcnew System::Windows::Forms::Label());
	this->lblTotalNodos = (gcnew System::Windows::Forms::Label());
	this->lblEnlacesInternos = (gcnew System::Windows::Forms::Label());
	this->lblEnlacesExternos = (gcnew System::Windows::Forms::Label());
	this->lblProfundidadReal = (gcnew System::Windows::Forms::Label());
	this->panelArbolGrafico = (gcnew System::Windows::Forms::Panel());
	this->panelBtnNuevoAnalisis = (gcnew System::Windows::Forms::Panel());
	this->lblBtnNuevoAnalisis = (gcnew System::Windows::Forms::Label());
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
	this->panelBtnNuevoAnalisis->SuspendLayout();
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
	this->txtUrl->Text = L""; 
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
	this->panelResultados->Controls->Add(this->grpAccionResultado);
	this->panelResultados->Controls->Add(this->grpAcciones);
	this->panelResultados->Controls->Add(this->grpResumen);
	this->panelResultados->Controls->Add(this->panelArbolGrafico);
	this->panelResultados->Dock = System::Windows::Forms::DockStyle::Fill;
	this->panelResultados->Location = System::Drawing::Point(0, 0);
	this->panelResultados->Name = L"panelResultados";
	this->panelResultados->Size = System::Drawing::Size(900, 700);
	this->panelResultados->TabIndex = 4;
	// 
	// btnVolverResumen
	// 
	this->btnVolverResumen->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
	this->btnVolverResumen->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold));
	this->btnVolverResumen->ForeColor = System::Drawing::Color::Black;
	this->btnVolverResumen->Location = System::Drawing::Point(20, 640);
	this->btnVolverResumen->Name = L"btnVolverResumen";
	this->btnVolverResumen->Size = System::Drawing::Size(180, 30);
	this->btnVolverResumen->TabIndex = 5;
	this->btnVolverResumen->Text = L"< Volver al Resumen";
	this->btnVolverResumen->Visible = false;
	this->btnVolverResumen->Click += gcnew System::EventHandler(this, &MyForm::btnVolverResumen_Click);
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
	// rtbAccionResultado
	// 
	this->rtbAccionResultado->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left));
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
	this->rtbAccionResultado->DetectUrls = true;
	this->rtbAccionResultado->LinkClicked += gcnew System::Windows::Forms::LinkClickedEventHandler(this, &MyForm::rtbAccionResultado_LinkClicked);
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
	// txtPalabraClave
	// 
	this->txtPalabraClave->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
	this->txtPalabraClave->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
	this->txtPalabraClave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
	this->txtPalabraClave->ForeColor = System::Drawing::Color::Gray;
	this->txtPalabraClave->Location = System::Drawing::Point(15, 30);
	this->txtPalabraClave->Name = L"txtPalabraClave";
	this->txtPalabraClave->Size = System::Drawing::Size(390, 30);
	this->txtPalabraClave->TabIndex = 2;
	this->txtPalabraClave->Text = L"Ingrese palabra clave aquí...";
	this->txtPalabraClave->Enter += gcnew System::EventHandler(this, &MyForm::txtPalabraClave_Enter);
	this->txtPalabraClave->Leave += gcnew System::EventHandler(this, &MyForm::txtPalabraClave_Leave);
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
	// grpResumen
	// 
	this->grpResumen->Controls->Add(this->lblResumenTitulo);
	this->grpResumen->Controls->Add(this->lblUrlAnalizada);
	this->grpResumen->Controls->Add(this->lblProfundidadSolicitada);
	this->grpResumen->Controls->Add(this->lblTotalNodos);
	this->grpResumen->Controls->Add(this->lblEnlacesInternos);
	this->grpResumen->Controls->Add(this->lblEnlacesExternos);
	this->grpResumen->Controls->Add(this->lblProfundidadReal);
	this->grpResumen->Controls->Add(this->panelBtnNuevoAnalisis);
	this->grpResumen->ForeColor = System::Drawing::Color::White;
	this->grpResumen->Location = System::Drawing::Point(20, 20);
	this->grpResumen->Name = L"grpResumen";
	this->grpResumen->Size = System::Drawing::Size(420, 320);
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
	// panelArbolGrafico
	// 
	this->panelArbolGrafico->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left)
		| System::Windows::Forms::AnchorStyles::Right));
	this->panelArbolGrafico->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
	this->panelArbolGrafico->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
	this->panelArbolGrafico->Location = System::Drawing::Point(20, 290);
	this->panelArbolGrafico->Name = L"panelArbolGrafico";
	this->panelArbolGrafico->Size = System::Drawing::Size(860, 380);
	this->panelArbolGrafico->TabIndex = 2;
	this->panelArbolGrafico->Visible = false;
	this->panelArbolGrafico->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelArbolGrafico_Paint);
	this->panelArbolGrafico->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::panelArbolGrafico_MouseWheel);
	this->panelArbolGrafico->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::panelArbolGrafico_MouseDown);
	this->panelArbolGrafico->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MyForm::panelArbolGrafico_MouseMove);
	//
	// panelBtnNuevoAnalisis
	// 
	this->panelBtnNuevoAnalisis->Controls->Add(this->lblBtnNuevoAnalisis);
	this->panelBtnNuevoAnalisis->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnNuevoAnalisis->Location = System::Drawing::Point(20, 260);
	this->panelBtnNuevoAnalisis->Name = L"panelBtnNuevoAnalisis";
	this->panelBtnNuevoAnalisis->Size = System::Drawing::Size(180, 40);
	this->panelBtnNuevoAnalisis->TabIndex = 7;
	this->panelBtnNuevoAnalisis->Click += gcnew System::EventHandler(this, &MyForm::btnNuevoAnalisis_Click);
	this->panelBtnNuevoAnalisis->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelAccion_Paint);
	// 
	// lblBtnNuevoAnalisis
	// 
	this->lblBtnNuevoAnalisis->BackColor = System::Drawing::Color::Transparent;
	this->lblBtnNuevoAnalisis->Dock = System::Windows::Forms::DockStyle::Fill;
	this->lblBtnNuevoAnalisis->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold));
	this->lblBtnNuevoAnalisis->ForeColor = System::Drawing::Color::White;
	this->lblBtnNuevoAnalisis->Location = System::Drawing::Point(0, 0);
	this->lblBtnNuevoAnalisis->Name = L"lblBtnNuevoAnalisis";
	this->lblBtnNuevoAnalisis->Size = System::Drawing::Size(180, 40);
	this->lblBtnNuevoAnalisis->TabIndex = 0;
	this->lblBtnNuevoAnalisis->Text = L"NUEVO ANÁLISIS";
	this->lblBtnNuevoAnalisis->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnNuevoAnalisis->Click += gcnew System::EventHandler(this, &MyForm::btnNuevoAnalisis_Click);
	// 
	// MyForm
	// 
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
	this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
	this->ClientSize = System::Drawing::Size(900, 700);
	this->Controls->Add(this->panelCarga);
	this->Controls->Add(this->panelInicio);
	this->Controls->Add(this->panelResultados);
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
	this->panelBtnVisualizar->ResumeLayout(false);
	this->lblBtnVisualizar->ResumeLayout(false);
	this->panelBtnBuscarPalabra->ResumeLayout(false);
	this->lblBtnBuscarPalabra->ResumeLayout(false);
	this->panelBtnDetectarRotos->ResumeLayout(false);
	this->lblBtnDetectarRotos->ResumeLayout(false);
	this->panelBtnExportar->ResumeLayout(false);
	this->lblBtnExportar->ResumeLayout(false);
	this->grpAccionResultado->ResumeLayout(false);
	this->grpAccionResultado->PerformLayout();
	this->panelBtnNuevoAnalisis->ResumeLayout(false);
	this->lblBtnNuevoAnalisis->ResumeLayout(false);
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
	// 
	// searchWorker
	// 
	this->searchWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::searchWorker_DoWork);
	this->searchWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::searchWorker_RunWorkerCompleted);
}
#pragma endregion