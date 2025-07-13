#include "MyForm.h"
#include "BusinessLogic.h"
#include "DataAccess.h"
#include <vector>
#include <string>
#include <map>
#include <ctime>

using namespace NexusCrawlerApp;

// Constructor del formulario.
MyForm::MyForm(void)
{
	InitializeComponent(); // Inicializa los componentes del diseñador de Windows Forms.
	crawler = new NavigationTree(); // Crea una instancia del objeto de lógica de negocio principal.
	this->isMouseOverExit = false; // Flag para el efecto hover del botón de salir.

	// Crea una región circular para el botón de salir, dándole una forma redonda.
	System::Drawing::Drawing2D::GraphicsPath^ path = gcnew System::Drawing::Drawing2D::GraphicsPath();
	path->AddEllipse(0, 0, this->panelBotonSalir->Width, this->panelBotonSalir->Height);
	this->panelBotonSalir->Region = gcnew System::Drawing::Region(path);

	// Muestra el panel de inicio al arrancar la aplicación.
	SwitchPanel(panelInicio);
}

// Destructor del formulario.
MyForm::~MyForm()
{
	// Libera la memoria de los componentes y del objeto crawler.
	if (components) {
		delete components;
	}
	delete crawler;
}

// --- Manejadores de Eventos de la Interfaz de Usuario ---

// Se ejecuta al hacer clic en el botón "Iniciar Análisis".
System::Void MyForm::btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
	// Valida que se haya ingresado una URL.
	if (String::IsNullOrWhiteSpace(this->txtUrl->Text)) {
		if (currentCulture == "" || currentCulture == "es") {
			MessageBox::Show(L"Por favor, ingrese una URL.", L"Error");
		}
		else {
			MessageBox::Show(L"Please enter a URL.", L"Error");
		}
		return;
	}
	// Evita iniciar un nuevo análisis si ya hay uno en curso.
	if (this->crawlWorker->IsBusy) return;

	// Cambia al panel de carga y prepara los argumentos para el worker.
	SwitchPanel(panelCarga);
	CrawlArgs^ args = gcnew CrawlArgs();
	args->Url = this->txtUrl->Text;
	args->Depth = static_cast<int>(this->numProfundidad->Value);
	args->IncludeSubdomains = this->chkIncludeSubdomains->Checked;

	// Inicia el análisis en un hilo separado para no bloquear la UI.
	this->crawlWorker->RunWorkerAsync(args);
}

// Tarea que se ejecuta en segundo plano para realizar el rastreo web.
System::Void MyForm::crawlWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	CrawlArgs^ args = static_cast<CrawlArgs^>(e->Argument);
	msclr::interop::marshal_context context;
	std::string stdUrl = context.marshal_as<std::string>(args->Url);

	// Llama al método de la lógica de negocio que realiza el rastreo.
	crawler->startCrawling(stdUrl, args->Depth, args->IncludeSubdomains);
}

// Se ejecuta cuando el worker de rastreo ha completado su tarea.
System::Void MyForm::crawlWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	// Maneja cualquier error que haya ocurrido durante el análisis.
	if (e->Error != nullptr) {
		if (currentCulture == "" || currentCulture == "es") {
			MessageBox::Show(L"Ocurrió un error durante el análisis: " + e->Error->Message, L"Error");
		}
		else {
			MessageBox::Show(L"An error occurred during the analysis: " + e->Error->Message, L"Error");
		}
		SwitchPanel(panelInicio); // Regresa al panel de inicio.
	}
	else {
		// Si el análisis fue exitoso, obtiene los resultados.
		AnalysisResult result = crawler->getAnalysisResult();
		// Actualiza las etiquetas en el panel de resultados con las estadísticas del análisis.
		if (currentCulture == "" || currentCulture == "es") {
			this->lblUrlAnalizada->Text = "URL Raíz Analizada: " + this->txtUrl->Text;
			this->lblProfundidadSolicitada->Text = L"Profundidad de Análisis Solicitada: " + this->numProfundidad->Value.ToString();
			this->lblTotalNodos->Text = L"Total de Nodos/Páginas Descubiertas: " + result.totalNodes;
			this->lblEnlacesInternos->Text = L"Enlaces Internos Encontrados: " + result.internalLinks;
			this->lblEnlacesExternos->Text = L"Enlaces Externos Encontrados: " + result.externalLinks;
			this->lblProfundidadReal->Text = L"Profundidad Máxima Real del Árbol: " + result.maxDepth;
		}
		else {
			this->lblUrlAnalizada->Text = "Root URL Analyzed: " + this->txtUrl->Text;
			this->lblProfundidadSolicitada->Text = L"Requested Analysis Depth: " + this->numProfundidad->Value.ToString();
			this->lblTotalNodos->Text = L"Total Nodes/Pages Discovered: " + result.totalNodes;
			this->lblEnlacesInternos->Text = L"Internal Links Found: " + result.internalLinks;
			this->lblEnlacesExternos->Text = L"External Links Found: " + result.externalLinks;
			this->lblProfundidadReal->Text = L"Maximum Tree Depth Reached: " + result.maxDepth;
		}
		// Muestra el panel de resultados.
		SwitchPanel(panelResultados);
	}
}

// Se ejecuta al hacer clic en el botón "Exportar HTML".
System::Void MyForm::btnExportar_Click(System::Object^ sender, System::EventArgs^ e) {
	// Verifica si existe un árbol de análisis para exportar.
	if (crawler->getRoot() == nullptr) {
		if (currentCulture == "" || currentCulture == "es") {
			MessageBox::Show(L"Primero debe realizar un análisis.", L"Árbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
		else {
			MessageBox::Show(L"You must first perform an analysis.", L"Tree not available", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
		return;
	}

	// Configura y muestra un diálogo para guardar el archivo.
	SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
	if (currentCulture == "" || currentCulture == "es") {
		saveFileDialog->Filter = "HTML File (*.html)|*.html|All files (*.*)|*.*";
		saveFileDialog->Title = L"Exportar Árbol de Navegación como HTML";
	}
	else {
		saveFileDialog->Filter = "HTML file (*.html)|*.html|All files (*.*)|*.*";
		saveFileDialog->Title = L"Export Navigation Tree as HTML";
	}

	// Genera un nombre de archivo sugerido basado en la URL analizada.
	try {
		Uri^ uri = gcnew Uri(this->txtUrl->Text);
		String^ host = uri->Host->Replace("www.", "");
		saveFileDialog->FileName = "Reporte_NexusCrawler_" + host + ".html";
	}
	catch (Exception^) {
		saveFileDialog->FileName = "Reporte_NexusCrawler.html";
	}

	// Si el usuario selecciona una ubicación y guarda...
	if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
		msclr::interop::marshal_context context;
		std::string filePath = context.marshal_as<std::string>(saveFileDialog->FileName);
		std::string analysisUrl = context.marshal_as<std::string>(this->txtUrl->Text);
		int requestedDepth = static_cast<int>(this->numProfundidad->Value);

		// Llama a la función de DataAccess para generar y guardar el archivo HTML.
		if (DataAccess::exportTreeToHtml(crawler->getRoot(), filePath, analysisUrl, requestedDepth)) {
			// Pregunta al usuario si desea abrir el archivo recién creado.
			if (currentCulture == "" || currentCulture == "es") {
				System::Windows::Forms::DialogResult openResult = MessageBox::Show(
					L"El árbol se ha exportado exitosamente como HTML.\n\n¿Desea abrir el archivo ahora?",
					L"Exportación Exitosa",
					MessageBoxButtons::YesNo,
					MessageBoxIcon::Information);

				if (openResult == System::Windows::Forms::DialogResult::Yes) {
					try {
						System::Diagnostics::Process::Start(saveFileDialog->FileName);
					}
					catch (Exception^ ex) {
						MessageBox::Show(L"No se pudo abrir el archivo: " + ex->Message, L"Error al abrir");
					}
				}

				else {
					MessageBox::Show(L"Ocurrió un error al guardar el archivo.", L"Error de Exportación", MessageBoxButtons::OK, MessageBoxIcon::Error);
				}
			}
			else {
				System::Windows::Forms::DialogResult openResult = MessageBox::Show(
					L"The tree has been successfully exported as HTML.\n\n Do you want to open the file now?",
					L"Successful Export",
					MessageBoxButtons::YesNo,
					MessageBoxIcon::Information);

				if (openResult == System::Windows::Forms::DialogResult::Yes) {
					try {
						System::Diagnostics::Process::Start(saveFileDialog->FileName);
					}
					catch (Exception^ ex) {
						MessageBox::Show(L"The file could not be opened: " + ex->Message, L"Error opening");
					}
				}
			}
		}
		else {
			// Muestra un error si la exportación falla.
			if (currentCulture == "" || currentCulture == "es") {
				MessageBox::Show(L"Ocurrió un error al guardar el archivo.", L"Error de Exportación", MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
			else {
				MessageBox::Show(L"An error occurred while saving the file.", L"Export Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}
	}
}

// Se ejecuta al hacer clic en el botón "Detectar Enlaces Rotos".
System::Void MyForm::btnDetectarRotos_Click(System::Object^ sender, System::EventArgs^ e) {
	// Verifica que exista un análisis previo.
	if (crawler->getRoot() == nullptr) {
		if (currentCulture == "" || currentCulture == "es") {
			MessageBox::Show(L"Primero debe realizar un análisis.", L"Árbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
		else {
			MessageBox::Show(L"You must first perform an analysis.", L"Tree not available", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
		return;
	}
	// Evita iniciar una nueva verificación si ya hay una en curso.
	if (linkCheckWorker->IsBusy) return;
	this->grpAcciones->Enabled = false; // Deshabilita los botones de acción.

	// Actualiza la UI para indicar que la verificación está en progreso.
	if (currentCulture == "" || currentCulture == "es") {
		this->lblAccionResultadoTitulo->Text = L"Verificando enlaces rotos...";
		this->rtbAccionResultado->Text = L"Por favor espere...";
	}
	else {
		this->lblAccionResultadoTitulo->Text = L"Checking broken links...";
		this->rtbAccionResultado->Text = L"Please wait...";
	}
	// Inicia la verificación en un hilo separado.
	linkCheckWorker->RunWorkerAsync();
}

// Se ejecuta al hacer clic en el botón "Buscar Palabra Clave".
System::Void MyForm::btnBuscarPalabra_Click(System::Object^ sender, System::EventArgs^ e) {
	if (currentCulture == "" || currentCulture == "es") {
		// Verifica que exista un análisis previo.
		if (crawler->getRoot() == nullptr) {
			MessageBox::Show(L"Primero debe realizar un análisis.", L"Árbol no disponible", MessageBoxButtons::OK, MessageBoxIcon::Information);
			return;
		}
		// Valida que se haya ingresado una palabra clave.
		if (String::IsNullOrWhiteSpace(txtPalabraClave->Text) || txtPalabraClave->Text == L"Ingrese palabra clave aquí...") {
			MessageBox::Show(L"Por favor, ingrese una palabra clave para buscar.", L"Entrada Inválida", MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}
		// Evita iniciar una nueva búsqueda si ya hay una en curso.
		if (searchWorker->IsBusy) return;
		this->grpAcciones->Enabled = false; // Deshabilita los botones de acción.
		this->lblAccionResultadoTitulo->Text = L"Buscando palabra clave...";
		this->rtbAccionResultado->Text = L"Por favor espere...";
		// Prepara los argumentos y ejecuta el worker de búsqueda.
		SearchArgs^ args = gcnew SearchArgs();
		args->Keyword = this->txtPalabraClave->Text;
		searchWorker->RunWorkerAsync(args);
	}
	else {
		if (crawler->getRoot() == nullptr) {
			MessageBox::Show(L"You must first perform an analysis.", L"Tree not available", MessageBoxButtons::OK, MessageBoxIcon::Information);
			return;
		}
		if (String::IsNullOrWhiteSpace(txtPalabraClave->Text) || txtPalabraClave->Text == L"Enter keyword here...") {
			MessageBox::Show(L"Please enter a keyword to search.", L"Invalid Input", MessageBoxButtons::OK, MessageBoxIcon::Warning);
			return;
		}
		if (searchWorker->IsBusy) return;
		this->grpAcciones->Enabled = false;
		this->lblAccionResultadoTitulo->Text = L"Searching for keyword...";
		this->rtbAccionResultado->Text = L"Please wait...";
		SearchArgs^ args = gcnew SearchArgs();
		args->Keyword = this->txtPalabraClave->Text;
		searchWorker->RunWorkerAsync(args);
	}
}

// Tarea en segundo plano para verificar el estado de todos los enlaces.
System::Void MyForm::linkCheckWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	// Llama al método de la lógica de negocio y convierte el resultado a un tipo gestionado.
	std::vector<std::string> brokenLinks = crawler->checkAllLinksStatus();
	List<String^>^ resultList = gcnew List<String^>();
	for (const auto& link : brokenLinks) {
		resultList->Add(gcnew String(link.c_str()));
	}
	e->Result = resultList;
}

// Se ejecuta cuando el worker de verificación de enlaces ha completado su tarea.
System::Void MyForm::linkCheckWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	if (currentCulture == "" || currentCulture == "es") {
		this->grpAcciones->Enabled = true; // Rehabilita los botones.
		// Maneja cualquier error.
		if (e->Error != nullptr) {
			this->lblAccionResultadoTitulo->Text = L"Error en la Verificación";
			this->rtbAccionResultado->Text = L"Ocurrió un error al verificar los enlaces.";
			return;
		}
		// Muestra los resultados en el RichTextBox.
		List<String^>^ brokenLinks = safe_cast<List<String^>^>(e->Result);
		this->lblAccionResultadoTitulo->Text = L"Resultados de 'Enlaces Rotos':";
		this->rtbAccionResultado->Clear();
		if (brokenLinks->Count == 0) {
			this->rtbAccionResultado->SelectionColor = Color::LightGreen;
			this->rtbAccionResultado->AppendText("[OK] ");
			this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			this->rtbAccionResultado->AppendText(L"¡Felicidades! No se encontraron enlaces rotos.");
		}
		else {
			this->rtbAccionResultado->AppendText("Se encontraron " + brokenLinks->Count + " enlaces rotos:\r\n\r\n");
			for each(String ^ link in brokenLinks) {
				rtbAccionResultado->SelectionColor = Color::Tomato;
				rtbAccionResultado->AppendText("[ROTO] ");
				rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
				rtbAccionResultado->AppendText(link + "\r\n");
			}
		}
	}
	else {
		this->grpAcciones->Enabled = true;
		if (e->Error != nullptr) {
			this->lblAccionResultadoTitulo->Text = L"Verification Error";
			this->rtbAccionResultado->Text = L"An error occurred while verifying the links.";
			return;
		}
		List<String^>^ brokenLinks = safe_cast<List<String^>^>(e->Result);
		this->lblAccionResultadoTitulo->Text = L"Results for 'Broken Links'";
		this->rtbAccionResultado->Clear();
		if (brokenLinks->Count == 0) {
			this->rtbAccionResultado->SelectionColor = Color::LightGreen;
			this->rtbAccionResultado->AppendText("[OK] ");
			this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			this->rtbAccionResultado->AppendText(L"Congratulations! No broken links found.");
		}
		else {
			this->rtbAccionResultado->AppendText("Found " + brokenLinks->Count + " broken links:\r\n\r\n");
			for each(String ^ link in brokenLinks) {
				rtbAccionResultado->SelectionColor = Color::Tomato;
				rtbAccionResultado->AppendText("[BROKEN] ");
				rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
				rtbAccionResultado->AppendText(link + "\r\n");
			}
		}
	}
}

// Tarea en segundo plano para buscar la ruta más corta a una palabra clave.
System::Void MyForm::searchWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e) {
	SearchArgs^ args = static_cast<SearchArgs^>(e->Argument);
	msclr::interop::marshal_context context;
	std::string keyword = context.marshal_as<std::string>(args->Keyword);
	// Llama al método de la lógica de negocio.
	PathResult result_nativa = crawler->findShortestPathToKeyword(keyword);
	// Convierte el resultado nativo a un tipo gestionado para pasarlo a la UI.
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

// Se ejecuta cuando el worker de búsqueda ha completado su tarea.
System::Void MyForm::searchWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e) {
	this->grpAcciones->Enabled = true; // Rehabilita los botones.
	// Maneja cualquier error.
	if (e->Error != nullptr) {
		if (currentCulture == "" || currentCulture == "es") {
			this->lblAccionResultadoTitulo->Text = L"Error en la Búsqueda";
			this->rtbAccionResultado->Text = L"Ocurrió un error durante la búsqueda.";
		}
		else {
			this->lblAccionResultadoTitulo->Text = L"Search Error";
			this->rtbAccionResultado->Text = L"An error occurred during the search.";
		}
		return;
	}
	// Muestra los resultados de la búsqueda en el RichTextBox.
	PathResultManaged^ result = safe_cast<PathResultManaged^>(e->Result);
	if (currentCulture == "" || currentCulture == "es") {
		this->lblAccionResultadoTitulo->Text = "Resultados de Búsqueda para '" + txtPalabraClave->Text + "':";
	}
	else
	{
		this->lblAccionResultadoTitulo->Text = "Search results for '" + txtPalabraClave->Text + "':";
	}

	this->rtbAccionResultado->Clear();
	if (!result->found) {
		// Informa si la palabra clave no fue encontrada.
		if (currentCulture == "" || currentCulture == "es") {
			this->rtbAccionResultado->SelectionColor = Color::Orange;
			this->rtbAccionResultado->AppendText("[NO ENCONTRADO] ");
			this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			this->rtbAccionResultado->AppendText(L"La palabra clave no se encontró en ninguna URL del árbol.");
		}
		else {
			this->rtbAccionResultado->SelectionColor = Color::Orange;
			this->rtbAccionResultado->AppendText("[NOT FOUND] ");
			this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			this->rtbAccionResultado->AppendText(L"The keyword was not found in any URL in the tree.");
		}
	}
	else {
		// Muestra la ruta más corta si se encontró la palabra clave.
		int clicks = result->path->Count - 1;
		this->rtbAccionResultado->SelectionColor = Color::LightGreen;
		if (currentCulture == "" || currentCulture == "es") {
			this->rtbAccionResultado->AppendText(L"[ÉXITO] ");
			this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			this->rtbAccionResultado->AppendText("Encontrado en " + clicks + " clic(s).\r\n\r\n");
			this->rtbAccionResultado->AppendText(L"Ruta más corta:\r\n");
		}
		else {
			this->rtbAccionResultado->AppendText(L"[SUCCESS] ");
			this->rtbAccionResultado->SelectionColor = rtbAccionResultado->ForeColor;
			this->rtbAccionResultado->AppendText("Found in " + clicks + " clic(s).\r\n\r\n");
			this->rtbAccionResultado->AppendText(L"Shorter route:\r\n");
		}
		// Imprime la ruta con indentación para mayor claridad.
		for (int i = 0; i < result->path->Count; ++i) {
			rtbAccionResultado->AppendText((gcnew String(L' ', i * 2)) + "-> " + result->path[i] + "\r\n");
		}
	}
}

// Permite abrir los enlaces del RichTextBox en el navegador por defecto.
System::Void MyForm::rtbAccionResultado_LinkClicked(System::Object^ sender, System::Windows::Forms::LinkClickedEventArgs^ e) {
	try {
		System::Diagnostics::Process::Start(e->LinkText);
	}
	catch (Exception^ ex) {
		MessageBox::Show(L"No se pudo abrir el enlace: " + ex->Message, L"Error");
	}
}

// --- Manejadores para el Placeholder del TextBox de Palabra Clave ---

// Limpia el texto del placeholder cuando el usuario hace foco en el cuadro de texto.
System::Void MyForm::txtPalabraClave_Enter(System::Object^ sender, System::EventArgs^ e) {
	if (currentCulture == "" || currentCulture == "es") {
		if (this->txtPalabraClave->Text == L"Ingrese palabra clave aquí...") {
			this->txtPalabraClave->Text = "";
			this->txtPalabraClave->ForeColor = Color::White;
		}
	}
	else {
		if (this->txtPalabraClave->Text == L"Enter keyword here...") {
			this->txtPalabraClave->Text = "";
			this->txtPalabraClave->ForeColor = Color::White;
		}
	}
}

// Restaura el texto del placeholder si el cuadro de texto queda vacío.
System::Void MyForm::txtPalabraClave_Leave(System::Object^ sender, System::EventArgs^ e) {
	if (currentCulture == "" || currentCulture == "es") {
		if (String::IsNullOrWhiteSpace(this->txtPalabraClave->Text)) {
			this->txtPalabraClave->Text = L"Ingrese palabra clave aquí...";
			this->txtPalabraClave->ForeColor = Color::Gray;
		}
	}
	else {
		if (String::IsNullOrWhiteSpace(this->txtPalabraClave->Text)) {
			this->txtPalabraClave->Text = L"Enter keyword here...";
			this->txtPalabraClave->ForeColor = Color::Gray;
		}
	}
}

// --- Métodos Auxiliares de la UI ---

// Gestiona la visibilidad de los paneles principales (Inicio, Carga, Resultados).
void MyForm::SwitchPanel(Panel^ panelToShow) {
	this->panelInicio->Visible = false;
	this->panelCarga->Visible = false;
	this->panelResultados->Visible = false;
	panelToShow->Visible = true;
	panelToShow->BringToFront();

	// Mueve el botón de salir para que sea visible en los paneles de inicio y resultados.
	if (panelToShow == panelInicio || panelToShow == panelResultados) {
		this->panelBotonSalir->Parent = panelToShow;
		this->panelBotonSalir->Visible = true;
		this->panelBotonSalir->BringToFront();
	}
	else {
		this->panelBotonSalir->Visible = false;
	}
}

// Aplica recursivamente los recursos de idioma a todos los controles del formulario.
System::Void NexusCrawlerApp::MyForm::ApplyResourcesToControls(System::Windows::Forms::Control::ControlCollection^ controls, System::ComponentModel::ComponentResourceManager^ resources)
{
	for each(System::Windows::Forms::Control ^ control in controls) {
		resources->ApplyResources(control, control->Name);
		if (control->HasChildren) {
			ApplyResourcesToControls(control->Controls, resources);
		}
	}
}

// --- Manejadores de Eventos de Dibujo Personalizado (Paint) ---

// Dibuja el fondo con gradiente para el botón principal de análisis.
System::Void MyForm::panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	System::Drawing::Rectangle rect = (cli::safe_cast<Panel^>(sender))->ClientRectangle;
	LinearGradientBrush^ brush = gcnew LinearGradientBrush(rect, Color::FromArgb(136, 95, 255), Color::FromArgb(95, 175, 255), LinearGradientMode::Horizontal);
	g->FillRectangle(brush, rect);
	delete brush;
}

// Dibuja el fondo con gradiente y bordes redondeados para los botones de acción.
System::Void MyForm::panelAccion_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	g->SmoothingMode = SmoothingMode::AntiAlias;
	Panel^ panel = cli::safe_cast<Panel^>(sender);
	System::Drawing::Rectangle rect = panel->ClientRectangle;
	GraphicsPath^ path = gcnew GraphicsPath();
	int cornerRadius = 15;
	// Crea una ruta con esquinas redondeadas.
	path->AddArc(rect.X, rect.Y, cornerRadius, cornerRadius, 180, 90);
	path->AddArc(rect.Right - cornerRadius, rect.Y, cornerRadius, cornerRadius, 270, 90);
	path->AddArc(rect.Right - cornerRadius, rect.Bottom - cornerRadius, cornerRadius, cornerRadius, 0, 90);
	path->AddArc(rect.X, rect.Bottom - cornerRadius, cornerRadius, cornerRadius, 90, 90);
	path->CloseFigure();
	panel->Region = gcnew System::Drawing::Region(path); // Aplica la región redondeada.
	// Rellena el panel con un gradiente.
	LinearGradientBrush^ brush = gcnew LinearGradientBrush(rect, Color::FromArgb(136, 95, 255), Color::FromArgb(95, 50, 230), LinearGradientMode::Vertical);
	g->FillPath(brush, path);
	delete brush;
	delete path;
}

// Se ejecuta al hacer clic en el botón "Nuevo Análisis".
System::Void MyForm::btnNuevoAnalisis_Click(System::Object^ sender, System::EventArgs^ e) {
	if (currentCulture == "" || currentCulture == "es") {
		// Pide confirmación al usuario antes de borrar los resultados actuales.
		System::Windows::Forms::DialogResult confirmResult = MessageBox::Show(
			L"¿Está seguro de que desea iniciar un nuevo análisis? Se perderán los resultados actuales.",
			L"Confirmar Nuevo Análisis",
			MessageBoxButtons::YesNo,
			MessageBoxIcon::Question);

		if (confirmResult == System::Windows::Forms::DialogResult::Yes) {
			// Limpia los campos de resultados y regresa al panel de inicio.
			this->rtbAccionResultado->Clear();
			this->lblAccionResultadoTitulo->Text = L"Resultados de la Acción:";
			this->txtPalabraClave->Text = L"Ingrese palabra clave aquí...";
			this->txtPalabraClave->ForeColor = Color::Gray;
			SwitchPanel(panelInicio);
		}
	}
	else {
		System::Windows::Forms::DialogResult confirmResult = MessageBox::Show(
			L"Are you sure you want to start a new analysis? Your current results will be lost.",
			L"Confirm New Analysis",
			MessageBoxButtons::YesNo,
			MessageBoxIcon::Question);

		if (confirmResult == System::Windows::Forms::DialogResult::Yes) {
			this->rtbAccionResultado->Clear();
			this->lblAccionResultadoTitulo->Text = L"Action Results:";
			this->txtPalabraClave->Text = L"Enter keyword here...";
			this->txtPalabraClave->ForeColor = Color::Gray;
			SwitchPanel(panelInicio);
		}
	}
}

// Cierra la aplicación al hacer clic en el botón de salir.
System::Void MyForm::panelBotonSalir_Click(System::Object^ sender, System::EventArgs^ e) {
	this->Close();
}

// Dibuja el botón de salir con un efecto de portal y un icono.
System::Void MyForm::panelBotonSalir_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
	Graphics^ g = e->Graphics;
	g->SmoothingMode = SmoothingMode::AntiAlias;
	Panel^ panel = cli::safe_cast<Panel^>(sender);
	System::Drawing::Rectangle rect = panel->ClientRectangle;

	GraphicsPath^ portalPath = gcnew GraphicsPath();
	portalPath->AddEllipse(rect);

	// Cambia los colores del gradiente según si el mouse está sobre el botón.
	Color centerColor = isMouseOverExit ? Color::FromArgb(220, 200, 255) : Color::FromArgb(136, 95, 255);
	Color outerColor = isMouseOverExit ? Color::FromArgb(136, 95, 255) : Color::FromArgb(28, 27, 45);

	PathGradientBrush^ portalBrush = gcnew PathGradientBrush(portalPath);
	portalBrush->CenterPoint = PointF(rect.Width / 2.0f, rect.Height / 2.0f);
	portalBrush->CenterColor = centerColor;
	array<Color>^ surroundColors = { outerColor };
	portalBrush->SurroundColors = surroundColors;

	g->FillEllipse(portalBrush, rect);

	// Dibuja el icono de "apagado" en el centro del botón.
	Pen^ iconPen = gcnew Pen(Color::White, 2);
	float iconSize = 20.0f;
	RectangleF iconRect = RectangleF((rect.Width - iconSize) / 2.0f, (rect.Height - iconSize) / 2.0f, iconSize, iconSize);
	g->DrawArc(iconPen, iconRect, -45, 270);
	g->DrawLine(iconPen, rect.Width / 2.0f, iconRect.Top + 2, rect.Width / 2.0f, iconRect.Top + 10);


	delete portalBrush;
	delete portalPath;
	delete iconPen;
}

// --- Manejadores de Eventos de Mouse para el Botón de Salir ---

// Activa el flag de hover y redibuja el botón cuando el mouse entra.
System::Void MyForm::panelBotonSalir_MouseEnter(System::Object^ sender, System::EventArgs^ e) {
	isMouseOverExit = true;
	(cli::safe_cast<Panel^>(sender))->Invalidate();
}

// Desactiva el flag de hover y redibuja el botón cuando el mouse sale.
System::Void MyForm::panelBotonSalir_MouseLeave(System::Object^ sender, System::EventArgs^ e) {
	isMouseOverExit = false;
	(cli::safe_cast<Panel^>(sender))->Invalidate();
}

// Función auxiliar para aplicar los recursos a los controles anidados (dentro de GroupBox, Panels, etc.)
void ApplyResourcesToControls(System::Windows::Forms::Control::ControlCollection^ controls, System::ComponentModel::ComponentResourceManager^ resources) {
	for each(System::Windows::Forms::Control ^ control in controls) {
		resources->ApplyResources(control, control->Name);
		if (control->HasChildren) {
			ApplyResourcesToControls(control->Controls, resources);
		}
	}
}

#pragma region Windows Form Designer generated code
// Método generado por el diseñador de Windows Forms para inicializar todos los componentes de la UI.
// Contiene la creación, configuración de propiedades y disposición de todos los controles.
void MyForm::InitializeComponent(void)
{
	try
	{
		System::Resources::ResourceManager^ resources =
			gcnew System::Resources::ResourceManager("NexusCrawlerApp.MyForm", MyForm::typeid->Assembly);

		this->Text = resources->GetString("$this.Text");
	}
	catch (System::Exception^ ex)
	{
		System::Windows::Forms::MessageBox::Show(ex->ToString(), "Error al Cargar Recursos");
	}
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
	this->grpAccionResultado = (gcnew System::Windows::Forms::GroupBox());
	this->rtbAccionResultado = (gcnew System::Windows::Forms::RichTextBox());
	this->lblAccionResultadoTitulo = (gcnew System::Windows::Forms::Label());
	this->txtPalabraClave = (gcnew System::Windows::Forms::TextBox());
	this->grpAcciones = (gcnew System::Windows::Forms::GroupBox());
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
	this->panelBtnNuevoAnalisis = (gcnew System::Windows::Forms::Panel());
	this->lblBtnNuevoAnalisis = (gcnew System::Windows::Forms::Label());
	this->panelBotonSalir = (gcnew System::Windows::Forms::Panel());
	this->lblBotonSalir = (gcnew System::Windows::Forms::Label());
	this->panelBtnBuscarPalabra = (gcnew System::Windows::Forms::Panel());
	this->lblBtnBuscarPalabra = (gcnew System::Windows::Forms::Label());
	this->chkIncludeSubdomains = (gcnew System::Windows::Forms::CheckBox());
	this->panelInicio->SuspendLayout();
	this->panelInputContainer->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->BeginInit();
	this->panelCarga->SuspendLayout();
	this->panelResultados->SuspendLayout();
	this->grpAccionResultado->SuspendLayout();
	this->grpAcciones->SuspendLayout();
	this->panelBtnDetectarRotos->SuspendLayout();
	this->panelBtnExportar->SuspendLayout();
	this->grpResumen->SuspendLayout();
	this->panelBtnNuevoAnalisis->SuspendLayout();
	this->panelBotonSalir->SuspendLayout();
	this->panelBtnBuscarPalabra->SuspendLayout();
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
	this->panelInputContainer->Anchor = System::Windows::Forms::AnchorStyles::None;
	this->panelInputContainer->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
	this->panelInputContainer->Controls->Add(this->lblUrl);
	this->panelInputContainer->Controls->Add(this->txtUrl);
	this->panelInputContainer->Controls->Add(this->lblEjemploUrl);
	this->panelInputContainer->Controls->Add(this->lblProfundidad);
	this->panelInputContainer->Controls->Add(this->numProfundidad);
	this->panelInputContainer->Controls->Add(this->chkIncludeSubdomains);
	this->panelInputContainer->Controls->Add(this->panelBotonAnalisis);
	this->panelInputContainer->Location = System::Drawing::Point(200, 200);
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
	this->txtUrl->Text = L"https://www.uneg.edu.ve";
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
	this->numProfundidad->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
	// 
	// chkIncludeSubdomains
	// 
	this->chkIncludeSubdomains->AutoSize = true;
	this->chkIncludeSubdomains->Checked = true; // Por defecto activado
	this->chkIncludeSubdomains->CheckState = System::Windows::Forms::CheckState::Checked;
	this->chkIncludeSubdomains->ForeColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(200)), static_cast<System::Int32>(static_cast<System::Byte>(220)));
	this->chkIncludeSubdomains->Location = System::Drawing::Point(44, 200);
	this->chkIncludeSubdomains->Name = L"chkIncludeSubdomains";
	this->chkIncludeSubdomains->Size = System::Drawing::Size(180, 24);
	this->chkIncludeSubdomains->TabIndex = 5;
	this->chkIncludeSubdomains->Text = L"Incluir subdominios en el análisis";
	this->chkIncludeSubdomains->UseVisualStyleBackColor = true;
	this->chkIncludeSubdomains->BackColor = System::Drawing::Color::Transparent;
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
	this->lblTitulo->Anchor = System::Windows::Forms::AnchorStyles::Top;
	this->lblTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 22.2F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
	this->lblTitulo->Location = System::Drawing::Point(12, 58);
	this->lblTitulo->Name = L"lblTitulo";
	this->lblTitulo->Size = System::Drawing::Size(876, 50);
	this->lblTitulo->TabIndex = 1;
	this->lblTitulo->Text = L"NexusCrawler - Analizador de Sitios Web";
	this->lblTitulo->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	// 
	// cmbLanguage
	// 
	this->cmbLanguage->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
	this->cmbLanguage->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(68)));
	this->cmbLanguage->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
	this->cmbLanguage->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->cmbLanguage->ForeColor = System::Drawing::Color::White;
	this->cmbLanguage->FormattingEnabled = true;
	this->cmbLanguage->Items->AddRange(gcnew cli::array< System::Object^  >(2) { L"Español", L"English" });
	this->cmbLanguage->Location = System::Drawing::Point(759, 20);
	this->cmbLanguage->Name = L"cmbLanguage";
	this->cmbLanguage->Size = System::Drawing::Size(121, 31);
	this->cmbLanguage->TabIndex = 0;
	this->cmbLanguage->SelectedIndex = 0;
	this->cmbLanguage->SelectedIndexChanged +=
		gcnew System::EventHandler(this, &MyForm::cmbLanguage_SelectedIndexChanged);
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
	this->progressBar->Anchor = System::Windows::Forms::AnchorStyles::None;
	this->progressBar->Location = System::Drawing::Point(200, 338);
	this->progressBar->MarqueeAnimationSpeed = 30;
	this->progressBar->Name = L"progressBar";
	this->progressBar->Size = System::Drawing::Size(500, 23);
	this->progressBar->Style = System::Windows::Forms::ProgressBarStyle::Marquee;
	this->progressBar->TabIndex = 2;
	// 
	// lblCargando
	// 
	this->lblCargando->Anchor = System::Windows::Forms::AnchorStyles::None;
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
	this->panelResultados->Controls->Add(this->grpAccionResultado);
	this->panelResultados->Controls->Add(this->grpAcciones);
	this->panelResultados->Controls->Add(this->grpResumen);
	this->panelResultados->Dock = System::Windows::Forms::DockStyle::Fill;
	this->panelResultados->Location = System::Drawing::Point(0, 0);
	this->panelResultados->Name = L"panelResultados";
	this->panelResultados->Padding = System::Windows::Forms::Padding(10);
	this->panelResultados->Size = System::Drawing::Size(900, 700);
	this->panelResultados->TabIndex = 4;
	// 
	// grpAccionResultado
	// 
	this->grpAccionResultado->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left)
		| System::Windows::Forms::AnchorStyles::Right));
	this->grpAccionResultado->Controls->Add(this->rtbAccionResultado);
	this->grpAccionResultado->Controls->Add(this->lblAccionResultadoTitulo);
	this->grpAccionResultado->Controls->Add(this->txtPalabraClave);
	this->grpAccionResultado->Controls->Add(this->panelBtnBuscarPalabra);
	this->grpAccionResultado->ForeColor = System::Drawing::Color::White;
	this->grpAccionResultado->Location = System::Drawing::Point(460, 110);
	this->grpAccionResultado->Name = L"grpAccionResultado";
	this->grpAccionResultado->Size = System::Drawing::Size(420, 560);
	this->grpAccionResultado->TabIndex = 3;
	this->grpAccionResultado->TabStop = false;
	this->grpAccionResultado->Text = L"Búsqueda y Resultados";
	// 
	// rtbAccionResultado
	// 
	this->rtbAccionResultado->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left)
		| System::Windows::Forms::AnchorStyles::Right));
	this->rtbAccionResultado->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(42)), static_cast<System::Int32>(static_cast<System::Byte>(41)), static_cast<System::Int32>(static_cast<System::Byte>(64)));
	this->rtbAccionResultado->BorderStyle = System::Windows::Forms::BorderStyle::None;
	this->rtbAccionResultado->Font = (gcnew System::Drawing::Font(L"Consolas", 10.2F));
	this->rtbAccionResultado->ForeColor = System::Drawing::Color::Gainsboro;
	this->rtbAccionResultado->Location = System::Drawing::Point(15, 110);
	this->rtbAccionResultado->Name = L"rtbAccionResultado";
	this->rtbAccionResultado->ReadOnly = true;
	this->rtbAccionResultado->Size = System::Drawing::Size(390, 435);
	this->rtbAccionResultado->TabIndex = 3;
	this->rtbAccionResultado->Text = L"";
	this->rtbAccionResultado->DetectUrls = true;
	this->rtbAccionResultado->LinkClicked += gcnew System::Windows::Forms::LinkClickedEventHandler(this, &MyForm::rtbAccionResultado_LinkClicked);
	// 
	// lblAccionResultadoTitulo
	// 
	this->lblAccionResultadoTitulo->AutoSize = true;
	this->lblAccionResultadoTitulo->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F, System::Drawing::FontStyle::Bold));
	this->lblAccionResultadoTitulo->Location = System::Drawing::Point(15, 80);
	this->lblAccionResultadoTitulo->Name = L"lblAccionResultadoTitulo";
	this->lblAccionResultadoTitulo->Size = System::Drawing::Size(201, 23);
	this->lblAccionResultadoTitulo->TabIndex = 1;
	this->lblAccionResultadoTitulo->Text = L"Resultados de la Acción:";
	// 
	// txtPalabraClave
	// 
	this->txtPalabraClave->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
		| System::Windows::Forms::AnchorStyles::Right));
	this->txtPalabraClave->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
	this->txtPalabraClave->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
	this->txtPalabraClave->Font = (gcnew System::Drawing::Font(L"Segoe UI", 10.2F));
	this->txtPalabraClave->ForeColor = System::Drawing::Color::Gray;
	this->txtPalabraClave->Location = System::Drawing::Point(15, 30);
	this->txtPalabraClave->Name = L"txtPalabraClave";
	this->txtPalabraClave->Size = System::Drawing::Size(210, 30);
	this->txtPalabraClave->TabIndex = 2;
	this->txtPalabraClave->Text = L"Ingrese palabra clave aquí...";
	this->txtPalabraClave->Enter += gcnew System::EventHandler(this, &MyForm::txtPalabraClave_Enter);
	this->txtPalabraClave->Leave += gcnew System::EventHandler(this, &MyForm::txtPalabraClave_Leave);
	// 
	// grpAcciones
	// 
	this->grpAcciones->Controls->Add(this->panelBtnDetectarRotos);
	this->grpAcciones->Controls->Add(this->panelBtnExportar);
	this->grpAcciones->ForeColor = System::Drawing::Color::White;
	this->grpAcciones->Location = System::Drawing::Point(460, 20);
	this->grpAcciones->Name = L"grpAcciones";
	this->grpAcciones->Size = System::Drawing::Size(420, 80);
	this->grpAcciones->TabIndex = 1;
	this->grpAcciones->TabStop = false;
	this->grpAcciones->Text = L"Acciones Generales";
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
	this->lblBtnDetectarRotos->Location = System::Drawing::Point(0, 0);
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
	this->panelBtnExportar->Location = System::Drawing::Point(220, 25);
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
	this->lblBtnExportar->Location = System::Drawing::Point(0, 0);
	this->lblBtnExportar->Name = L"lblBtnExportar";
	this->lblBtnExportar->Size = System::Drawing::Size(180, 40);
	this->lblBtnExportar->TabIndex = 0;
	this->lblBtnExportar->Text = L"EXPORTAR HTML";
	this->lblBtnExportar->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnExportar->Click += gcnew System::EventHandler(this, &MyForm::btnExportar_Click);
	// 
	// grpResumen
	// 
	this->grpResumen->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left));
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
	this->grpResumen->Size = System::Drawing::Size(420, 650);
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
	// panelBtnNuevoAnalisis
	// 
	this->panelBtnNuevoAnalisis->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
	this->panelBtnNuevoAnalisis->Controls->Add(this->lblBtnNuevoAnalisis);
	this->panelBtnNuevoAnalisis->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnNuevoAnalisis->Location = System::Drawing::Point(20, 590);
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
	// panelBotonSalir
	// 
	this->panelBotonSalir->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
	this->panelBotonSalir->BackColor = System::Drawing::Color::Transparent;
	this->panelBotonSalir->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBotonSalir->Location = System::Drawing::Point(840, 640);
	this->panelBotonSalir->Name = L"panelBotonSalir";
	this->panelBotonSalir->Size = System::Drawing::Size(40, 40);
	this->panelBotonSalir->TabIndex = 6;
	this->panelBotonSalir->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MyForm::panelBotonSalir_Paint);
	this->panelBotonSalir->Click += gcnew System::EventHandler(this, &MyForm::panelBotonSalir_Click);
	this->panelBotonSalir->MouseEnter += gcnew System::EventHandler(this, &MyForm::panelBotonSalir_MouseEnter);
	this->panelBotonSalir->MouseLeave += gcnew System::EventHandler(this, &MyForm::panelBotonSalir_MouseLeave);
	// 
	// lblBotonSalir
	// 
	this->lblBotonSalir->Location = System::Drawing::Point(0, 0);
	this->lblBotonSalir->Name = L"lblBotonSalir";
	this->lblBotonSalir->Size = System::Drawing::Size(100, 23);
	this->lblBotonSalir->TabIndex = 0;
	// 
	// panelBtnBuscarPalabra
	// 
	this->panelBtnBuscarPalabra->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
	this->panelBtnBuscarPalabra->Controls->Add(this->lblBtnBuscarPalabra);
	this->panelBtnBuscarPalabra->Cursor = System::Windows::Forms::Cursors::Hand;
	this->panelBtnBuscarPalabra->Location = System::Drawing::Point(230, 25);
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
	this->lblBtnBuscarPalabra->Location = System::Drawing::Point(0, 0);
	this->lblBtnBuscarPalabra->Name = L"lblBtnBuscarPalabra";
	this->lblBtnBuscarPalabra->Size = System::Drawing::Size(180, 40);
	this->lblBtnBuscarPalabra->TabIndex = 0;
	this->lblBtnBuscarPalabra->Text = L"BUSCAR CLAVE";
	this->lblBtnBuscarPalabra->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
	this->lblBtnBuscarPalabra->Click += gcnew System::EventHandler(this, &MyForm::btnBuscarPalabra_Click);
	// 
	// MyForm
	// 
	this->Icon = gcnew System::Drawing::Icon(Application::StartupPath + L"\\icono\\grafoicono.ico");
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::None;
	this->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(28)), static_cast<System::Int32>(static_cast<System::Byte>(27)), static_cast<System::Int32>(static_cast<System::Byte>(45)));
	this->ClientSize = System::Drawing::Size(900, 700);
	this->Controls->Add(this->panelResultados);
	this->Controls->Add(this->panelCarga);
	this->Controls->Add(this->panelInicio);
	this->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9.75F));
	this->ForeColor = System::Drawing::Color::White;
	this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
	this->MaximizeBox = true;
	this->MinimumSize = System::Drawing::Size(920, 720);
	this->Name = L"MyForm";
	this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
	this->Text = L"NexusCrawlerApp";
	this->panelInicio->ResumeLayout(false);
	this->panelInputContainer->ResumeLayout(false);
	this->panelInputContainer->PerformLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->numProfundidad))->EndInit();
	this->panelCarga->ResumeLayout(false);
	this->panelResultados->ResumeLayout(false);
	this->grpAccionResultado->ResumeLayout(false);
	this->grpAccionResultado->PerformLayout();
	this->grpAcciones->ResumeLayout(false);
	this->panelBtnDetectarRotos->ResumeLayout(false);
	this->panelBtnExportar->ResumeLayout(false);
	this->grpResumen->ResumeLayout(false);
	this->grpResumen->PerformLayout();
	this->panelBtnNuevoAnalisis->ResumeLayout(false);
	this->panelBotonSalir->ResumeLayout(false);
	this->panelBtnBuscarPalabra->ResumeLayout(false);
	this->ResumeLayout(false);

	// Agregar los eventos de los Workers
	this->crawlWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::crawlWorker_DoWork);
	this->crawlWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::crawlWorker_RunWorkerCompleted);
	this->linkCheckWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::linkCheckWorker_DoWork);
	this->linkCheckWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::linkCheckWorker_RunWorkerCompleted);
	this->searchWorker->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::searchWorker_DoWork);
	this->searchWorker->RunWorkerCompleted += gcnew System::ComponentModel::RunWorkerCompletedEventHandler(this, &MyForm::searchWorker_RunWorkerCompleted);
	System::ComponentModel::ComponentResourceManager^ resources = gcnew System::ComponentModel::ComponentResourceManager(MyForm::typeid);
}
#pragma endregion