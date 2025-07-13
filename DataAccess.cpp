#include "DataAccess.h"
#include <ctime>
#include <functional>
#include "MyForm.h"

// Escribe los datos del nodo en formato de lista HTML
void DataAccess::writeNodeToList(std::ofstream& file, WebNode* node) {
    if (!node) return;

    std::string li_class = "node";
    std::string icon_type_class = "doc";
    if (!node->children.empty()) {
        li_class += " expandable";
        icon_type_class = "folder";
    }
    
    std::string tag = "";
	// Determina el tipo de enlace y etiqueta
    if (node->type == LinkType::External) {
        if (NexusCrawlerApp::MyForm::currentCulture == "" || NexusCrawlerApp::MyForm::currentCulture == "es") {
            tag = "<span class='tag external-tag'>EXTERNO</span>";
        } else {
            tag = "<span class='tag external-tag'>EXTERNAL</span>";
		}
    }
    // Si el nodo es roto, se muestra etiqueta de error
    if (node->status == LinkStatus::Broken) {
        if (NexusCrawlerApp::MyForm::currentCulture == "" || NexusCrawlerApp::MyForm::currentCulture == "es") {
            tag = "<span class='tag broken-tag'>ROTO</span>";
        }
        else {
            tag = "<span class='tag broken-tag'>BROKEN</span>";
        }
    }
    // Si el nodo es interno, no se muestra etiqueta
    file << "<li class='" << li_class << "'>\n";
    file << "  <div class='node-content'>\n";
    file << "    <i class='icon " << icon_type_class << "'></i>\n"; 
    file << "    <a href='" << node->url << "' target='_blank'>" << node->url << "</a>\n";
    file << "    " << tag << "\n";
    file << "  </div>\n";

    if (!node->children.empty()) {
        file << "  <ul class='nested'>\n";
        for (WebNode* child : node->children) {
            writeNodeToList(file, child);
        }
        file << "  </ul>\n";
    }
    file << "</li>\n";
}

// Escribe los datos de los nodos en formato JSON para visualización en JavaScript
void DataAccess::writeNodeDataForJs(std::ofstream& file, WebNode* node, int& nodeIdCounter) {
	if (!node) return;

	// Asignar un ID único a cada nodo
	node->js_id = nodeIdCounter++;

	// Escribir los datos del nodo en formato JSON
	file << "{ \"id\": " << node->js_id << ", \"url\": \"" << node->url << "\", \"type\": \"" << (node->type == LinkType::Internal ? "internal" : "external") << "\", \"status\": \"" << (node->status == LinkStatus::Broken ? "broken" : "ok") << "\" },\n";

	// Escribir las conexiones (enlaces) a sus hijos
	for (WebNode* child : node->children) {
		writeNodeDataForJs(file, child, nodeIdCounter);
	}
}

// Exporta el árbol de navegación a un archivo HTML
bool DataAccess::exportTreeToHtml(WebNode* root, const std::string& filePath,
    const std::string& analysisUrl, int requestedDepth) {
    if (NexusCrawlerApp::MyForm::currentCulture == "" || NexusCrawlerApp::MyForm::currentCulture == "es") {

        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        time_t now = time(0);
        char dt[26];
        ctime_s(dt, sizeof dt, &now);

		// --- Cabecera HTML ---
        file << "<!DOCTYPE html>\n<html lang='es'>\n<head>\n";
        file << "  <meta charset='UTF-8'>\n";
        file << "  <title>Reporte de Analisis - NexusCrawler</title>\n";
        file << "  <script src='https://d3js.org/d3.v7.min.js'></script>\n";

        file << "<style>\n"
            "  :root { --bg-dark: #1c1b2d; --bg-medium: #2a2940; --accent: #885fff; --text-light: #f0f0f0; --text-medium: #c0c0ff; --line-color: rgba(136, 95, 255, 0.2); --neon-glow: rgba(136, 95, 255, 0.7); --tag-ext: #3b82f6; --tag-brk: #ef4444; }\n"
            "  body { font-family: 'Segoe UI', system-ui, sans-serif; background-color: var(--bg-dark); color: var(--text-light); margin: 0; padding: 20px; }\n"
            "  .container { max-width: 1200px; margin: 0 auto; background-color: var(--bg-medium); padding: 25px; border-radius: 15px; box-shadow: 0 0 20px rgba(0,0,0,0.5); }\n"
            "  h1 { color: white; border-bottom: 2px solid var(--accent); padding-bottom: 10px; text-shadow: 0 0 5px var(--neon-glow); } \n"
            "  .summary { background-color: var(--bg-dark); padding: 15px; border-radius: 8px; margin-bottom: 20px; font-size: 1.1em; border-left: 3px solid var(--accent); }\n"
            "  .summary p { margin: 8px 0; }\n .summary strong { color: var(--text-medium); }\n .footer { text-align: center; margin-top: 20px; color: #aaa; font-size: 0.9em; }\n"
            "  .view-toggle { margin-bottom: 20px; }\n"
            "  .view-toggle button { background-color: #444; border: 1px solid #666; color: white; padding: 10px 15px; cursor: pointer; border-radius: 5px; margin-right: 10px; font-size: 14px; }\n"
            "  .view-toggle button.active { background-color: #885fff; border-color: #a07fff; }\n"
            "  #list-view-container, #tree-view-container { display: none; }\n"
            "  #list-view-container.active, #tree-view-container.active { display: block; }\n"
            "  #list-view-container ul { list-style: none; padding-left: 0; }\n"
            "  #list-view-container .node { position: relative; padding-left: 30px; }\n"
            "  #list-view-container .node-content { display: flex; align-items: center; padding: 6px; border-radius: 5px; transition: background-color 0.2s; cursor: pointer; }\n"
            "  #list-view-container .node-content:hover { background-color: #3c3a54; }\n"
            "  #list-view-container .node::before, #list-view-container .node::after { content: ''; position: absolute; left: 12px; background: linear-gradient(to right, var(--line-color), transparent); }\n"
            "  #list-view-container .node::before { top: 0; bottom: 0; width: 1px; }\n"
            "  #list-view-container li.node:last-child::before { height: 20px; }\n"
            "  #list-view-container .node::after { top: 20px; width: 18px; height: 1px; }\n"
            "  #list-view-container .icon { width: 1.5em; height: 1.5em; display: inline-block; margin-right: 8px; transition: transform 0.2s ease-in-out; background-size: contain; background-repeat: no-repeat; background-position: center; filter: drop-shadow(0 0 3px var(--neon-glow)); }\n"
            "  #list-view-container .icon.folder { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M4 20q-.825 0-1.413-.587T2 18V6q0-.825.588-1.413T4 4h6l2 2h8q.825 0 1.413.588T22 8v10q0 .825-.588 1.413T20 20H4Z\"/%3E%3C/svg%3E'); }\n"
            "  #list-view-container .icon.doc { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M6 22q-.825 0-1.413-.587T4 20V4q0-.825.588-1.413T6 2h8l6 6v12q0 .825-.588 1.413T18 22H6Zm7-12V4H6v16h12V9h-5Z\"/%3E%3C/svg%3E'); }\n"
            "  #list-view-container .expanded > .node-content > .icon.folder { transform: rotate(0deg); } /* Original state */\n"
            "  #list-view-container .node-content > .icon.folder { transform: rotate(-90deg); }\n"
            "  #list-view-container .nested { max-height: 0; overflow: hidden; transition: max-height 0.3s ease-in-out; }\n"
            "  #list-view-container .expanded > .nested { max-height: 5000px; }\n"
            "  #list-view-container a { color: var(--text-medium); text-decoration: none; word-break: break-all; }\n"
            "  #list-view-container a:hover { text-decoration: underline; }\n"
            "  #list-view-container .tag { font-size: 0.7em; font-weight: bold; padding: 2px 8px; border-radius: 10px; margin-left: 10px; color: white; }\n"
            "  #list-view-container .external-tag { background-color: var(--tag-ext); }\n"
            "  #list-view-container .broken-tag { background-color: var(--tag-brk); }\n"
            "  #tree-view-container { width: 100%; height: 80vh; border: 1px solid #444; border-radius: 8px; overflow: hidden; background-color: #1c1b2d; cursor: move; }\n"
            "  .tree-node circle { stroke-width: 2px; cursor: pointer; }\n"
            "  .tree-node circle.internal { fill: #2a2940; stroke: #885fff; }\n"
            "  .tree-node circle.external { fill: #4a4960; stroke: #aaa; }\n"
            "  .tree-node circle.broken { stroke: #ef4444; stroke-dasharray: 4,4; }\n"
            "  .tree-node text { fill: #f0f0f0; font-size: 14px; pointer-events: none; text-anchor: middle; dominant-baseline: central; font-weight: bold; }\n"
            "  .tree-link { fill: none; stroke: #555; stroke-width: 1.5px; }\n"
            "  .tooltip { position: absolute; text-align: center; max-width: 300px; padding: 8px; font: 12px sans-serif; background: #f0f0f0; color: #1c1b2d; border: 0px; border-radius: 8px; pointer-events: none; opacity: 0; word-wrap: break-word; }\n"
            "</style>\n</head>\n<body>\n<div class='container'>\n";

        // --- Cabecera, Resumen y Botones ---
        file << "<h1>Reporte de Arbol de Navegacion</h1>\n"
            "<div class='summary'>"
            "<p><strong>URL Analizada:</strong> " << analysisUrl << "</p>"
            "<p><strong>Profundidad Solicitada:</strong> " << requestedDepth << "</p>"
            "<p><strong>Fecha de Generacion:</strong> " << dt << "</p>"
            "</div>\n";
        file << "<div class='view-toggle'>"
            "<button id='btn-list-view' class='active'>Vista de Lista</button>"
            "<button id='btn-tree-view'>Vista de Arbol</button>"
            "</div>\n";

        // --- Contenedores de Vistas ---
        file << "<div id='list-view-container' class='active'>\n<ul id='tree-root'>\n";
        writeNodeToList(file, root);
        file << "</ul>\n</div>\n";
        file << "<div id='tree-view-container'></div>\n";

        // --- Generación de Datos para el Árbol ---
        file << "<script>\n"
            "const flatTreeData = [\n";

        std::vector<WebNode*> allNodes;
        std::function<void(WebNode*)> collectNodes =
            [&](WebNode* n) {
            if (!n) return;
            allNodes.push_back(n);
            for (WebNode* child : n->children) {
                collectNodes(child);
            }
            };

        int id_counter = 0;
        if (root) collectNodes(root);

        for (WebNode* node : allNodes) {
            node->js_id = id_counter++;
            file << "{ \"id\": " << node->js_id
                << ", \"parentId\": " << (node->parent ? std::to_string(node->parent->js_id) : "null")
                << ", \"url\": \"" << node->url
                << "\", \"type\": \"" << (node->type == LinkType::Internal ? "internal" : "external")
                << "\", \"status\": \"" << (node->status == LinkStatus::Broken ? "broken" : "ok") << "\" },\n";
        }

        file << "];\n";

        // --- SCRIPT DE D3.JS CON EL AJUSTE DE ESPACIADO ---
        file << R"JS(
        document.addEventListener('DOMContentLoaded', function() {
            document.querySelectorAll('#list-view-container .expandable').forEach(li => li.classList.add('expanded'));
            var toggles = document.querySelectorAll('#list-view-container .expandable > .node-content');
            for (var i = 0; i < toggles.length; i++) {
                toggles[i].addEventListener('click', function(e) {
                    if (e.target.tagName !== 'A') this.parentElement.classList.toggle('expanded');
                });
            }
            const btnList = document.getElementById('btn-list-view');
            const btnTree = document.getElementById('btn-tree-view');
            const listView = document.getElementById('list-view-container');
            const treeView = document.getElementById('tree-view-container');
            let isTreeDrawn = false;
            btnList.addEventListener('click', () => { listView.classList.add('active'); treeView.classList.remove('active'); btnList.classList.add('active'); btnTree.classList.remove('active'); });
            btnTree.addEventListener('click', () => { treeView.classList.add('active'); listView.classList.remove('active'); btnTree.classList.add('active'); btnList.classList.remove('active'); if (!isTreeDrawn) { drawHierarchicalTree(); isTreeDrawn = true; } });
        });

        // --- FUNCIÓN DE DIBUJO CON EL ESPACIADO CORREGIDO ---
        function drawHierarchicalTree() {
            if (flatTreeData.length === 0) return;

            const container = document.getElementById('tree-view-container');
            const width = container.clientWidth;
            const height = container.clientHeight;

            const root = d3.stratify()
                .id(d => d.id)
                .parentId(d => d.parentId)
                (flatTreeData);

            const treeLayout = d3.tree().nodeSize([50, 150]); // [Separación Horizontal, Separación Vertical]

            treeLayout(root);
            
            // Calculamos el tamaño real que necesita el árbol después de aplicar el layout
            let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
            root.each(d => {
                if (d.x < minX) minX = d.x;
                if (d.x > maxX) maxX = d.x;
                if (d.y < minY) minY = d.y;
                if (d.y > maxY) maxY = d.y;
            });
            const treeWidth = maxX - minX;
            const treeHeight = maxY - minY;
            
            const tooltip = d3.select("body").append("div").attr("class", "tooltip");

            const svg = d3.select(container).append("svg")
                .attr("width", width)
                .attr("height", height);
            
            const g = svg.append("g");

            const zoom = d3.zoom()
                .scaleExtent([0.1, 8])
                .on("zoom", (event) => {
                    g.attr("transform", event.transform);
                });
            svg.call(zoom);

            g.selectAll('.tree-link')
                .data(root.links())
                .enter()
                .append('path')
                .attr('class', 'tree-link')
                .attr('d', d3.linkVertical()
                    .x(d => d.x)
                    .y(d => d.y));

            const node = g.selectAll('.tree-node')
                .data(root.descendants())
                .enter()
                .append('g')
                .attr('class', 'tree-node')
                .attr('transform', d => `translate(${d.x},${d.y})`)
                .on("mouseover", (event, d) => {
                    tooltip.transition().duration(200).style("opacity", .9);
                    tooltip.html(d.data.url).style("left", (event.pageX + 15) + "px").style("top", (event.pageY - 15) + "px");
                })
                .on("mouseout", () => tooltip.transition().duration(500).style("opacity", 0))
                .on("click", (event, d) => window.open(d.data.url, '_blank'));

            node.append('circle')
                .attr('r', 15)
                .attr('class', d => `${d.data.type} ${d.data.status}`);

            node.append('text')
                .attr('dy', '0.31em')
                .text(d => d.data.id);
            
            // Centrar el árbol en la vista inicial
            const initialScale = Math.min(width / (treeWidth + 100), height / (treeHeight + 100));
            const initialX = width / 2 - ((minX + maxX) / 2) * initialScale;
            const initialY = height / 2 - ((minY + maxY) / 2) * initialScale;

            svg.call(zoom.transform, d3.zoomIdentity.translate(initialX, initialY).scale(initialScale));
        }
    )JS";

        file << "</script>\n</div>\n<div class='footer'>Generado por NexusCrawler</div>\n</body>\n</html>\n";
        file.close();
        return true;
    }
 else { // Si el idioma no es español, generamos el HTML en inglés

     std::ofstream file(filePath);
     if (!file.is_open()) return false;

     time_t now = time(0);
     char dt[26];
     ctime_s(dt, sizeof dt, &now);

	 // --- HTML Header ---
     file << "<!DOCTYPE html>\n<html lang='en'>\n<head>\n";
     file << "  <meta charset='UTF-8'>\n";
     file << "  <title>Analysis Report - NexusCrawler</title>\n";
     file << "  <script src='https://d3js.org/d3.v7.min.js'></script>\n";

     // --- CSS ---
     file << "<style>\n"
         "  :root { --bg-dark: #1c1b2d; --bg-medium: #2a2940; --accent: #885fff; --text-light: #f0f0f0; --text-medium: #c0c0ff; --line-color: rgba(136, 95, 255, 0.2); --neon-glow: rgba(136, 95, 255, 0.7); --tag-ext: #3b82f6; --tag-brk: #ef4444; }\n"
         "  body { font-family: 'Segoe UI', system-ui, sans-serif; background-color: var(--bg-dark); color: var(--text-light); margin: 0; padding: 20px; }\n"
         "  .container { max-width: 1200px; margin: 0 auto; background-color: var(--bg-medium); padding: 25px; border-radius: 15px; box-shadow: 0 0 20px rgba(0,0,0,0.5); }\n"
         "  h1 { color: white; border-bottom: 2px solid var(--accent); padding-bottom: 10px; text-shadow: 0 0 5px var(--neon-glow); } \n"
         "  .summary { background-color: var(--bg-dark); padding: 15px; border-radius: 8px; margin-bottom: 20px; font-size: 1.1em; border-left: 3px solid var(--accent); }\n"
         "  .summary p { margin: 8px 0; }\n .summary strong { color: var(--text-medium); }\n .footer { text-align: center; margin-top: 20px; color: #aaa; font-size: 0.9em; }\n"
         "  .view-toggle { margin-bottom: 20px; }\n"
         "  .view-toggle button { background-color: #444; border: 1px solid #666; color: white; padding: 10px 15px; cursor: pointer; border-radius: 5px; margin-right: 10px; font-size: 14px; }\n"
         "  .view-toggle button.active { background-color: #885fff; border-color: #a07fff; }\n"
         "  #list-view-container, #tree-view-container { display: none; }\n"
         "  #list-view-container.active, #tree-view-container.active { display: block; }\n"
         "  #list-view-container ul { list-style: none; padding-left: 0; }\n"
         "  #list-view-container .node { position: relative; padding-left: 30px; }\n"
         "  #list-view-container .node-content { display: flex; align-items: center; padding: 6px; border-radius: 5px; transition: background-color 0.2s; cursor: pointer; }\n"
         "  #list-view-container .node-content:hover { background-color: #3c3a54; }\n"
         "  #list-view-container .node::before, #list-view-container .node::after { content: ''; position: absolute; left: 12px; background: linear-gradient(to right, var(--line-color), transparent); }\n"
         "  #list-view-container .node::before { top: 0; bottom: 0; width: 1px; }\n"
         "  #list-view-container li.node:last-child::before { height: 20px; }\n"
         "  #list-view-container .node::after { top: 20px; width: 18px; height: 1px; }\n"
         "  #list-view-container .icon { width: 1.5em; height: 1.5em; display: inline-block; margin-right: 8px; transition: transform 0.2s ease-in-out; background-size: contain; background-repeat: no-repeat; background-position: center; filter: drop-shadow(0 0 3px var(--neon-glow)); }\n"
         "  #list-view-container .icon.folder { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M4 20q-.825 0-1.413-.587T2 18V6q0-.825.588-1.413T4 4h6l2 2h8q.825 0 1.413.588T22 8v10q0 .825-.588 1.413T20 20H4Z\"/%3E%3C/svg%3E'); }\n"
         "  #list-view-container .icon.doc { background-image: url('data:image/svg+xml,%3Csvg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\" fill=\"%23c0c0ff\"%3E%3Cpath d=\"M6 22q-.825 0-1.413-.587T4 20V4q0-.825.588-1.413T6 2h8l6 6v12q0 .825-.588 1.413T18 22H6Zm7-12V4H6v16h12V9h-5Z\"/%3E%3C/svg%3E'); }\n"
         "  #list-view-container .expanded > .node-content > .icon.folder { transform: rotate(0deg); } /* Original state */\n"
         "  #list-view-container .node-content > .icon.folder { transform: rotate(-90deg); }\n"
         "  #list-view-container .nested { max-height: 0; overflow: hidden; transition: max-height 0.3s ease-in-out; }\n"
         "  #list-view-container .expanded > .nested { max-height: 5000px; }\n"
         "  #list-view-container a { color: var(--text-medium); text-decoration: none; word-break: break-all; }\n"
         "  #list-view-container a:hover { text-decoration: underline; }\n"
         "  #list-view-container .tag { font-size: 0.7em; font-weight: bold; padding: 2px 8px; border-radius: 10px; margin-left: 10px; color: white; }\n"
         "  #list-view-container .external-tag { background-color: var(--tag-ext); }\n"
         "  #list-view-container .broken-tag { background-color: var(--tag-brk); }\n"
         "  #tree-view-container { width: 100%; height: 80vh; border: 1px solid #444; border-radius: 8px; overflow: hidden; background-color: #1c1b2d; cursor: move; }\n"
         "  .tree-node circle { stroke-width: 2px; cursor: pointer; }\n"
         "  .tree-node circle.internal { fill: #2a2940; stroke: #885fff; }\n"
         "  .tree-node circle.external { fill: #4a4960; stroke: #aaa; }\n"
         "  .tree-node circle.broken { stroke: #ef4444; stroke-dasharray: 4,4; }\n"
         "  .tree-node text { fill: #f0f0f0; font-size: 14px; pointer-events: none; text-anchor: middle; dominant-baseline: central; font-weight: bold; }\n"
         "  .tree-link { fill: none; stroke: #555; stroke-width: 1.5px; }\n"
         "  .tooltip { position: absolute; text-align: center; max-width: 300px; padding: 8px; font: 12px sans-serif; background: #f0f0f0; color: #1c1b2d; border: 0px; border-radius: 8px; pointer-events: none; opacity: 0; word-wrap: break-word; }\n"
         "</style>\n</head>\n<body>\n<div class='container'>\n";

     // --- Cabecera, Resumen y Botones  ---
     file << "<h1>Navigation Tree Report</h1>\n"
         "<div class='summary'>"
         "<p><strong>URL Parsed:</strong> " << analysisUrl << "</p>"
         "<p><strong>Requested Depth:</strong> " << requestedDepth << "</p>"
         "<p><strong>Generation Date:</strong> " << dt << "</p>"
         "</div>\n";
     file << "<div class='view-toggle'>"
         "<button id='btn-list-view' class='active'>List View</button>"
         "<button id='btn-tree-view'>Tree View</button>"
         "</div>\n";

     // --- Contenedores de Vistas ---
     file << "<div id='list-view-container' class='active'>\n<ul id='tree-root'>\n";
     writeNodeToList(file, root);
     file << "</ul>\n</div>\n";
     file << "<div id='tree-view-container'></div>\n";

     // --- Generación de Datos para el Árbol ---
     file << "<script>\n"
         "const flatTreeData = [\n";

     std::vector<WebNode*> allNodes;
     std::function<void(WebNode*)> collectNodes =
         [&](WebNode* n) {
         if (!n) return;
         allNodes.push_back(n);
         for (WebNode* child : n->children) {
             collectNodes(child);
         }
         };

     int id_counter = 0;
     if (root) collectNodes(root);

     for (WebNode* node : allNodes) {
         node->js_id = id_counter++;
         file << "{ \"id\": " << node->js_id
             << ", \"parentId\": " << (node->parent ? std::to_string(node->parent->js_id) : "null")
             << ", \"url\": \"" << node->url
             << "\", \"type\": \"" << (node->type == LinkType::Internal ? "internal" : "external")
             << "\", \"status\": \"" << (node->status == LinkStatus::Broken ? "broken" : "ok") << "\" },\n";
     }

     file << "];\n";

     // --- SCRIPT DE D3.JS CON EL AJUSTE DE ESPACIADO ---
     file << R"JS(
        document.addEventListener('DOMContentLoaded', function() {
            document.querySelectorAll('#list-view-container .expandable').forEach(li => li.classList.add('expanded'));
            var toggles = document.querySelectorAll('#list-view-container .expandable > .node-content');
            for (var i = 0; i < toggles.length; i++) {
                toggles[i].addEventListener('click', function(e) {
                    if (e.target.tagName !== 'A') this.parentElement.classList.toggle('expanded');
                });
            }
            const btnList = document.getElementById('btn-list-view');
            const btnTree = document.getElementById('btn-tree-view');
            const listView = document.getElementById('list-view-container');
            const treeView = document.getElementById('tree-view-container');
            let isTreeDrawn = false;
            btnList.addEventListener('click', () => { listView.classList.add('active'); treeView.classList.remove('active'); btnList.classList.add('active'); btnTree.classList.remove('active'); });
            btnTree.addEventListener('click', () => { treeView.classList.add('active'); listView.classList.remove('active'); btnTree.classList.add('active'); btnList.classList.remove('active'); if (!isTreeDrawn) { drawHierarchicalTree(); isTreeDrawn = true; } });
        });

        function drawHierarchicalTree() {
            if (flatTreeData.length === 0) return;

            const container = document.getElementById('tree-view-container');
            const width = container.clientWidth;
            const height = container.clientHeight;

            const root = d3.stratify()
                .id(d => d.id)
                .parentId(d => d.parentId)
                (flatTreeData);

            const treeLayout = d3.tree().nodeSize([50, 150]); // [Separación Horizontal, Separación Vertical]

            treeLayout(root);
            
            // Calculamos el tamaño real que necesita el árbol después de aplicar el layout
            let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
            root.each(d => {
                if (d.x < minX) minX = d.x;
                if (d.x > maxX) maxX = d.x;
                if (d.y < minY) minY = d.y;
                if (d.y > maxY) maxY = d.y;
            });
            const treeWidth = maxX - minX;
            const treeHeight = maxY - minY;
            
            const tooltip = d3.select("body").append("div").attr("class", "tooltip");

            const svg = d3.select(container).append("svg")
                .attr("width", width)
                .attr("height", height);
            
            const g = svg.append("g");

            const zoom = d3.zoom()
                .scaleExtent([0.1, 8])
                .on("zoom", (event) => {
                    g.attr("transform", event.transform);
                });
            svg.call(zoom);

            g.selectAll('.tree-link')
                .data(root.links())
                .enter()
                .append('path')
                .attr('class', 'tree-link')
                .attr('d', d3.linkVertical()
                    .x(d => d.x)
                    .y(d => d.y));

            const node = g.selectAll('.tree-node')
                .data(root.descendants())
                .enter()
                .append('g')
                .attr('class', 'tree-node')
                .attr('transform', d => `translate(${d.x},${d.y})`)
                .on("mouseover", (event, d) => {
                    tooltip.transition().duration(200).style("opacity", .9);
                    tooltip.html(d.data.url).style("left", (event.pageX + 15) + "px").style("top", (event.pageY - 15) + "px");
                })
                .on("mouseout", () => tooltip.transition().duration(500).style("opacity", 0))
                .on("click", (event, d) => window.open(d.data.url, '_blank'));

            node.append('circle')
                .attr('r', 15)
                .attr('class', d => `${d.data.type} ${d.data.status}`);

            node.append('text')
                .attr('dy', '0.31em')
                .text(d => d.data.id);
            
            // Centrar el árbol en la vista inicial
            const initialScale = Math.min(width / (treeWidth + 100), height / (treeHeight + 100));
            const initialX = width / 2 - ((minX + maxX) / 2) * initialScale;
            const initialY = height / 2 - ((minY + maxY) / 2) * initialScale;

            svg.call(zoom.transform, d3.zoomIdentity.translate(initialX, initialY).scale(initialScale));
        }
    )JS";

     file << "</script>\n</div>\n<div class='footer'>Generated by NexusCrawler</div>\n</body>\n</html>\n";
     file.close();
     return true;

    }
}