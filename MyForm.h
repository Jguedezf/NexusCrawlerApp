#pragma once

// Inclusión de la API de Windows y enlace con la librería Gdi32 para gráficos.
#include <Windows.h>
#pragma comment(lib, "Gdi32.lib")

// Librería para facilitar la conversión entre tipos de datos de C++ nativo y C++/CLI.
#include <msclr/marshal_cppstd.h>
#include <vector>
#include <string>

// Declaraciones anticipadas (forward declarations) para evitar dependencias circulares
// y reducir los tiempos de compilación. Se usan para tipos nativos de C++.
class NavigationTree;
struct AnalysisResult;
struct PathResult;

namespace NexusCrawlerApp { 
	
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Collections::Generic;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;

    // Estructura para pasar argumentos al BackgroundWorker que realiza el rastreo web.
	public ref struct CrawlArgs {
		String^ Url;
		int Depth;
		bool IncludeSubdomains;
	};

    // Estructura para pasar argumentos al BackgroundWorker que busca palabras clave.
	public ref struct SearchArgs {
		String^ Keyword;
	};

    // Estructura para devolver los resultados de una búsqueda de ruta en un formato gestionado.
	public ref struct PathResultManaged {
		bool found;
		List<String^>^ path;
	};

    // Clase principal del formulario de la aplicación.
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void);
        // Almacena la cultura de la interfaz de usuario actual (ej. "es" o "en").
		static System::String^ currentCulture = "";

	protected:
        // Destructor para liberar recursos no gestionados.
		~MyForm();

        private:  
        // --- Miembros de Lógica de Negocio (Nativos) ---
		NavigationTree* crawler; // Puntero al objeto principal de rastreo (código nativo).

        // --- Componentes para Tareas Asíncronas ---
		BackgroundWorker^ crawlWorker;      // Worker para el rastreo principal.
		BackgroundWorker^ linkCheckWorker;  // Worker para verificar enlaces rotos.
		BackgroundWorker^ searchWorker;     // Worker para buscar palabras clave.

        // --- Componentes del Diseñador de Formularios ---
		System::ComponentModel::Container^ components;

        // --- Controles de la Interfaz de Usuario (Paneles) ---
		Panel^ panelInicio;
		Panel^ panelCarga;
		Panel^ panelResultados;

        // --- Controles de la Interfaz de Usuario (Etiquetas, Cajas de Texto, etc.) ---
		Label^ lblTitulo;
		Label^ lblUrl;
		TextBox^ txtUrl;
		Label^ lblProfundidad;
		NumericUpDown^ numProfundidad;
		CheckBox^ chkIncludeSubdomains;
		Label^ lblEjemploUrl;
		ComboBox^ cmbLanguage;
		Panel^ panelInputContainer;
		Panel^ panelBotonAnalisis;
		Label^ lblBotonAnalisis;
		Label^ lblCargando;
		ProgressBar^ progressBar;
		GroupBox^ grpResumen;
		Label^ lblResumenTitulo;
		Label^ lblUrlAnalizada;
		Label^ lblProfundidadSolicitada;
		Label^ lblTotalNodos;
		Label^ lblEnlacesInternos;
		Label^ lblEnlacesExternos;
		Label^ lblProfundidadReal;
		GroupBox^ grpAcciones;
		Panel^ panelBtnDetectarRotos;
		Label^ lblBtnDetectarRotos;
		Panel^ panelBtnBuscarPalabra;
		Label^ lblBtnBuscarPalabra;
		Panel^ panelBtnExportar;
		Label^ lblBtnExportar;
		GroupBox^ grpAccionResultado;
		TextBox^ txtPalabraClave;
		Label^ lblAccionResultadoTitulo;
		RichTextBox^ rtbAccionResultado;
		Panel^ panelBtnNuevoAnalisis;
		Label^ lblBtnNuevoAnalisis;
		Panel^ panelBotonSalir;
		Label^ lblBotonSalir;
		bool isMouseOverExit; // Flag para el efecto hover del botón de salir.

#pragma region Windows Form Designer generated code
		// Método generado automáticamente por el diseñador de Windows Forms para inicializar los componentes.
		void InitializeComponent(void);
#pragma endregion

	private:
        // --- Métodos de la Lógica de la Interfaz ---
        // Cambia la visibilidad entre los paneles principales de la aplicación.
		void SwitchPanel(Panel^ panelToShow);
        // Aplica los recursos de idioma a una colección de controles.
		System::Void ApplyResourcesToControls(System::Windows::Forms::Control::ControlCollection^ controls, System::ComponentModel::ComponentResourceManager^ resources);
		
        // --- Manejadores de Eventos de Clics en Botones ---
        System::Void btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnExportar_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnDetectarRotos_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnBuscarPalabra_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnNuevoAnalisis_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonSalir_Click(System::Object^ sender, System::EventArgs^ e);

        // --- Manejadores de Eventos de Dibujo (Paint) y Mouse ---
		System::Void panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void panelAccion_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void panelBotonSalir_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void panelBotonSalir_MouseEnter(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonSalir_MouseLeave(System::Object^ sender, System::EventArgs^ e);

        // --- Manejadores de Eventos de BackgroundWorkers ---
		System::Void crawlWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void crawlWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
		System::Void linkCheckWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void linkCheckWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
		System::Void searchWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void searchWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);

        // --- Otros Manejadores de Eventos de Controles ---
		System::Void rtbAccionResultado_LinkClicked(System::Object^ sender, System::Windows::Forms::LinkClickedEventArgs^ e);
		System::Void txtPalabraClave_Enter(System::Object^ sender, System::EventArgs^ e);
		System::Void txtPalabraClave_Leave(System::Object^ sender, System::EventArgs^ e);

        // Manejador para el cambio de selección en el ComboBox de idioma.
		System::Void cmbLanguage_SelectedIndexChanged(System::Object^ sender, System::EventArgs^ e) {
			String^ culture = "";
			if (this->cmbLanguage->SelectedItem->ToString() == "Español") {
				culture = "es-VE";
				currentCulture = "es";
			}
			else if (this->cmbLanguage->SelectedItem->ToString() == "English") {
				culture = "en-US";
				currentCulture = "en";
			}

			// Cambia la "cultura" del hilo actual de la interfaz de usuario.
			System::Threading::Thread::CurrentThread->CurrentUICulture = gcnew System::Globalization::CultureInfo(culture);

			// Carga los recursos del formulario para el nuevo idioma.
			System::ComponentModel::ComponentResourceManager^ resources = gcnew System::ComponentModel::ComponentResourceManager(MyForm::typeid);

			// Itera sobre todos los controles y les aplica el nuevo texto del recurso.
			resources->ApplyResources(this, "$this");
			ApplyResourcesToControls(this->Controls, resources);

			// Actualiza el texto del placeholder del cuadro de búsqueda si es necesario.
			if (String::IsNullOrWhiteSpace(this->txtPalabraClave->Text) || this->txtPalabraClave->Text->Contains("...")) {
				this->txtPalabraClave->Text = resources->GetString("txtPalabraClave.Text");
			}
		}
	};
}