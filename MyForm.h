#pragma once

#include <Windows.h>
#pragma comment(lib, "Gdi32.lib")
#include <msclr/marshal_cppstd.h>
#include <vector>

class NavigationTree;
struct AnalysisResult;

namespace NexusCrawlerApp {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Windows::Forms;
	using namespace System::Drawing;
	using namespace System::Drawing::Drawing2D;

	public ref struct CrawlArgs {
		String^ Url;
		int Depth;
	};

	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		MyForm(void);
	protected:
		~MyForm();

	private:
		NavigationTree* crawler;
		BackgroundWorker^ crawlWorker;
		BackgroundWorker^ linkCheckWorker;
		System::ComponentModel::Container^ components;

		Panel^ panelInicio;
		Panel^ panelCarga;
		Panel^ panelResultados;
		Label^ lblTitulo;
		Label^ lblUrl;
		TextBox^ txtUrl;
		Label^ lblProfundidad;
		NumericUpDown^ numProfundidad;
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
		Panel^ panelBtnVisualizar;
		Label^ lblBtnVisualizar;

		GroupBox^ grpAccionResultado;
		TextBox^ txtPalabraClave;
		Label^ lblAccionResultadoTitulo;
		RichTextBox^ rtbAccionResultado;

		Panel^ panelArbolGrafico;
		Button^ btnVolverResumen;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void);
#pragma endregion

	private:
		void SwitchPanel(Panel^ panelToShow);
		System::Void btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void panelArbolGrafico_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void btnExportar_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnDetectarRotos_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnBuscarPalabra_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelAccion_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void btnVisualizarArbol_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnVolverResumen_Click(System::Object^ sender, System::EventArgs^ e);

		System::Void crawlWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void crawlWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
		System::Void linkCheckWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void linkCheckWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
	};
}