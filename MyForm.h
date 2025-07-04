#pragma once

#include <Windows.h>
#pragma comment(lib, "Gdi32.lib")
#include <msclr/marshal_cppstd.h>

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
		System::ComponentModel::BackgroundWorker^ backgroundCrawler;
		System::ComponentModel::Container^ components;

		// Componentes de UI
		MenuStrip^ menuStrip; // <-- AÑADIDO
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
		Label^ lblResumenTitulo;
		Label^ lblUrlAnalizada;
		Label^ lblProfundidadSolicitada;
		Label^ lblTotalNodos;
		Label^ lblEnlacesInternos;
		Label^ lblEnlacesExternos;
		Label^ lblProfundidadReal;
		Button^ btnVolver;

#pragma region Windows Form Designer generated code
		void InitializeComponent(void);
#pragma endregion

	private:
		// Manejadores de eventos y funciones
		void SwitchPanel(Panel^ panelToShow);
		System::Void btnIniciarAnalisis_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void backgroundCrawler_DoWork(System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e);
		System::Void backgroundCrawler_RunWorkerCompleted(System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e);
		System::Void btnVolver_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void panelBotonAnalisis_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e);

		// --- NUEVOS MANEJADORES PARA EL MENÚ ---
		System::Void nuevoAnalisisToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void exportarArbolToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void salirToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
		System::Void acercaDeToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e);
	};
}