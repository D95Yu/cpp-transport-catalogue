#include <iostream>
#include <sstream>
#include <string>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"

using namespace transport_catalogue;
using namespace json_reader;
using namespace render;

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами Вывести в stdout ответы в виде JSON
     */


    TransportCatalogue catalogue;
    //std::string s;
    //std::getline(std::cin, s);
    //std::istringstream input(s);
    JsonReader reader(std::cin, std::cout);

    std::vector<StatRequest> stat_requests;
    RenderSettings settings;
    reader.FillTransportCatalogue(catalogue, stat_requests, settings);
    MapRenderer renderer(settings);
    RequestHandler request_handler(catalogue, renderer);
    //request_handler.RenderMap(std::cout);
    reader.PrintStat(catalogue, stat_requests, request_handler);


    
    /*std::vector<StatRequest> stat_request;    
    map_renderer::RenderSettings render_settings;
    TransportCatalogue catalogue;   
    Document doc_in;
    
    doc_in = json::load(std::cin);  
    json::parse(doc_in, catalogue, stat_request, render_settings);
    
    map_renderer::MapRenderer map_render(render_settings);
    
    request_handler::RequestHandler request_handler(catalogue, map_render);
    request_handler.render_map(std::cout);*/

    //json_reader.PrintStat(std::cout);

    /*std::vector<StatRequest> stat_request;
    
    TransportCatalogue catalogue;
    
    Document doc_in;
    Document doc_out;
    
    doc_in = json::Load(std::cin);
    
    json::Parse(doc_in, catalogue, stat_request);
    doc_out = json::ExecuteQueries(catalogue, stat_request);
    Print(doc_out, std::cout);*/

}