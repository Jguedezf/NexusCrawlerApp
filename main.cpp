#include "MyForm.h" 

using namespace System;
using namespace System::Windows::Forms;

// Atributo STAThread: Indica que el modelo de subprocesos COM de la aplicaci�n es
// Single-Threaded Apartment. Es un requisito para las aplicaciones de Windows Forms.
[STAThreadAttribute]
int main(array<String^>^ args)
{
    // Habilita los estilos visuales modernos de Windows para los controles.
    Application::EnableVisualStyles();
    // Establece el motor de renderizado de texto predeterminado a GDI+ (m�s moderno).
    Application::SetCompatibleTextRenderingDefault(false);

    // Crea una instancia del formulario principal y lo ejecuta,
    // iniciando as� el bucle de mensajes de la aplicaci�n.
    Application::Run(gcnew NexusCrawlerApp::MyForm());

    return 0;
}
