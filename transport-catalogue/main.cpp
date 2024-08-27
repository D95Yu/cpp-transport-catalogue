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

    JsonReader reader(std::cin, std::cout);

    std::vector<StatRequest> stat_requests;
    RenderSettings settings;
    reader.FillTransportCatalogue(catalogue, stat_requests, settings);

    MapRenderer renderer(settings);
    RequestHandler request_handler(catalogue, renderer);
    
    reader.PrintStat(catalogue, stat_requests, request_handler);

}