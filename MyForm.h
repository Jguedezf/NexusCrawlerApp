#pragma once

#include <Windows.h>
#pragma comment(lib, "Gdi32.lib")
#include <msclr/marshal_cppstd.h>
#include <vector>

// Forward declarations para tipos nativos
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

	public ref struct CrawlArgs {
		String^ Url;
		int Depth;
		bool IncludeSubdomains;
	};

	public ref struct SearchArgs {
		String^ Keyword;
	};

	public ref struct PathResultManaged {
		bool found;
		List<String^>^ path;
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
		BackgroundWorker^ searchWorker;
		System::ComponentModel::Container^ components;

		Panel^ panelInicio;
		Panel^ panelCarga;
		Panel^ panelResultados;
		Label^ lblTitulo;
		Label^ lblUrl;
		TextBox^ txtUrl;
		Label^ lblProfundidad;
		NumericUpDown^ numProfundidad;
		CheckBox^ chkIncludeSubdomains; // <--- AÑADIR ESTA LÍNEA
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
		bool isMouseOverExit;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void);
#pragma endregion

	private:
		void SwitchPanel(Panel^ panelToShow);
		System::Void btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void btnExportar_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnDetectarRotos_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void btnBuscarPalabra_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelAccion_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void btnNuevoAnalisis_Click(System::Object^ sender, System::EventArgs^ e);

		System::Void panelBotonSalir_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);
		System::Void panelBotonSalir_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonSalir_MouseEnter(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonSalir_MouseLeave(System::Object^ sender, System::EventArgs^ e);

		System::Void crawlWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void crawlWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
		System::Void linkCheckWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void linkCheckWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
		System::Void searchWorker_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void searchWorker_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);

		System::Void rtbAccionResultado_LinkClicked(System::Object^ sender, System::Windows::Forms::LinkClickedEventArgs^ e);
		System::Void txtPalabraClave_Enter(System::Object^ sender, System::EventArgs^ e);
		System::Void txtPalabraClave_Leave(System::Object^ sender, System::EventArgs^ e);
	};
}