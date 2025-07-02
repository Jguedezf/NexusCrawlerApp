#include "MyForm.h" // Incluimos el dise�o de nuestra ventana

using namespace System;
using namespace System::Windows::Forms;

// La directiva [STAThread] es necesaria para que los componentes de Windows Forms funcionen correctamente.
[STAThreadAttribute]
int main(array<String^>^ args)
{
    // Habilita los estilos visuales de Windows para que la aplicaci�n no se vea como de Windows 98.
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // Esta es la l�nea clave:
    // 1. Creamos una instancia de nuestro formulario (NexusCrawlerApp::MyForm).
    // 2. Le pasamos el control a Application::Run(), que la muestra en pantalla y espera eventos (clics, etc.).
    Application::Run(gcnew NexusCrawlerApp::MyForm());

    return 0;
}