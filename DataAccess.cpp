// DataAccess.cpp
#include "DataAccess.h"
#include <ctime>

// <<--- CORRECCIÓN: Firma de la función ahora coincide con la declaración del .h ---
void DataAccess::writeNodeToHtml(std::ofstream& file, WebNode* node) {
	if (!node) return;

	std::string li_class = "node";
	std::string icon_type_class = " doc";
	if (!node->children.empty()) {
		li_class += " expandable";
		icon_type_class = " folder";
	}

	std::string tag = "";
	if (node->type == LinkType::External) {
		tag = "<span class='tag external-tag'>EXTERNO</span>";
	}
	if (node->status == LinkStatus::Broken) {
		tag = "<span class='tag broken-tag'>ROTO</span>";
	}

	file << "<li class='" << li_class << "'>";
	file << "  <div class='node-content'>";
	file << "    <i class='icon" << icon_type_class << "'></i>";
	file << "    <a href='" << node->url << "' target='_blank'>" << node->url << "</a>" << tag;
	file << "  </div>";

	if (!node->children.empty()) {
		file << "  <ul class='nested'>\n";
		for (WebNode* child : node->children) {
			// <<--- CORRECCIÓN: La llamada recursiva ahora usa la firma correcta ---
			writeNodeToHtml(file, child);
		}
		file << "  </ul>\n";
	}
	file << "</li>\n";
}

bool DataAccess::exportTreeToHtml(WebNode* root, const std::string& filePath, const std::string& analysisUrl, int requestedDepth) {
	std::ofstream file(filePath);
	if (!file.is_open()) {
		return false;
	}

	time_t now = time(0);
	char dt[26];
	ctime_s(dt, sizeof dt, &now);

	file << "<!DOCTYPE html>\n";
	file << "<html lang='es'>\n";
	file << "<head>\n";
	file << "  <meta charset='UTF-8'>\n";
	file << "  <title>Reporte de Análisis - NexusCrawler</title>\n";
	file << "  <style>\n";
	file << "    :root { --bg-dark: #1c1b2d; --bg-medium: #2a2940; --accent: #885fff; --text-light: #f0f0f0; --text-medium: #c0c0ff; --line-color: rgba(136, 95, 255, 0.2); --neon-glow: rgba(136, 95, 255, 0.7); --tag-ext: #3b82f6; --tag-brk: #ef4444; }\n";
	file << "    body { font-family: 'Segoe UI', system-ui, sans-serif; background-color: var(--bg-dark); color: var(--text-light); margin: 0; padding: 20px; }\n";
	file << "    .container { max-width: 1200px; margin: 0 auto; background-color: var(--bg-medium); padding: 25px; border-radius: 15px; box-shadow: 0 0 20px rgba(0,0,0,0.5); }\n";
	file << "    h1 { color: white; border-bottom: 2px solid var(--accent); padding-bottom: 10px; text-shadow: 0 0 5px var(--neon-glow); } \n";
	file << "    .summary { background-color: var(--bg-dark); padding: 15px; border-radius: 8px; margin-bottom: 20px; font-size: 1.1em; border-left: 3px solid var(--accent); }\n";
	file << "    .summary p { margin: 8px 0; }\n";
	file << "    .summary strong { color: var(--text-medium); }\n";
	file << "    ul { list-style: none; padding-left: 0; }\n";
	file << "    .node { position: relative; padding-left: 30px; }\n";
	file << "    .node-content { display: flex; align-items: center; padding: 6px; border-radius: 5px; transition: background-color 0.2s; cursor: pointer; }\n";
	file << "    .node-content:hover { background-color: #3c3a54; }\n";
	file << "    .node::before, .node::after { content: ''; position: absolute; left: 12px; background: linear-gradient(to right, var(--line-color), transparent); }\n";
	file << "    .node::before { top: 0; bottom: 0; width: 1px; }\n";
	file << "    li.node:last-child::before { height: 20px; }\n";
	file << "    .node::after { top: 20px; width: 18px; height: 1px; }\n";
	file << "    .icon { width: 1.5em; height: 1.5em; display: inline-block; margin-right: 8px; transition: transform 0.2s ease-in-out; background-size: contain; background-repeat: no-repeat; background-position: center; filter: drop-shadow(0 0 3px var(--neon-glow)); }\n";
	file << "    .icon.folder { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M4 20q-.825 0-1.413-.587T2 18V6q0-.825.588-1.413T4 4h6l2 2h8q.825 0 1.413.588T22 8v10q0 .825-.588 1.413T20 20H4Z\"/%3E%3C/svg%3E'); }\n";
	file << "    .icon.doc { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M6 22q-.825 0-1.413-.587T4 20V4q0-.825.588-1.413T6 2h8l6 6v12q0 .825-.588 1.413T18 22H6Zm7-12V4H6v16h12V9h-5Z\"/%3E%3C/svg%3E'); }\n";
	file << "    .expanded > .node-content > .icon.folder { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M4 20q-.825 0-1.413-.587T2 18V6q0-.825.588-1.413T4 4h6l2 2h8q.825 0 1.413.588T22 8v10q0 .825-.588 1.413T20 20H4Z\"/%3E%3C/svg%3E'); }\n";
	file << "    .nested { max-height: 0; overflow: hidden; transition: max-height 0.3s ease-in-out; }\n";
	file << "    .expanded > .nested { max-height: 10000px; }\n";
	file << "    a { color: var(--text-medium); text-decoration: none; word-break: break-all; }\n";
	file << "    a:hover { text-decoration: underline; }\n";
	file << "    .tag { font-size: 0.7em; font-weight: bold; padding: 2px 8px; border-radius: 10px; margin-left: 10px; color: white; }\n";
	file << "    .external-tag { background-color: var(--tag-ext); }\n";
	file << "    .broken-tag { background-color: var(--tag-brk); }\n";
	file << "    .footer { text-align: center; margin-top: 20px; color: #aaa; font-size: 0.9em; }\n";
	file << "  </style>\n";
	file << "</head>\n";
	file << "<body>\n";
	file << "  <div class='container'>\n";
	file << "    <h1>Reporte de Arbol de Navegacion</h1>\n";
	file << "    <div class='summary'>\n";
	file << "       <p><strong>URL Analizada:</strong> " << analysisUrl << "</p>\n";
	file << "       <p><strong>Profundidad Solicitada:</strong> " << requestedDepth << "</p>\n";
	file << "       <p><strong>Fecha de Generacion:</strong> " << dt << "</p>\n";
	file << "    </div>\n";
	file << "    <ul id='tree-root'>\n";
	writeNodeToHtml(file, root);
	file << "    </ul>\n";
	file << "  </div>\n";
	file << "  <div class='footer'>Generado por NexusCrawler</div>\n";
	file << "  <script>\n";
	file << "    document.addEventListener('DOMContentLoaded', function() {\n";
	file << "      document.querySelectorAll('.expandable').forEach(li => li.classList.add('expanded'));\n";
	file << "      var toggles = document.querySelectorAll('.expandable > .node-content');\n";
	file << "      for (var i = 0; i < toggles.length; i++) {\n";
	file << "        toggles[i].addEventListener('click', function(e) {\n";
	file << "          if (e.target.tagName !== 'A') { this.parentElement.classList.toggle('expanded'); }\n";
	file << "        });\n";
	file << "      }\n";
	file << "    });\n";
	file << "  </script>\n";
	file << "</body>\n";
	file << "</html>\n";

	file.close();
	return true;
}