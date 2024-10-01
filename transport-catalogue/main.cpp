    #include <iostream>
    #include <sstream>
    #include <string>

    #include "transport_catalogue.h"
    #include "transport_router.h"
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

        //std::istringstream a{"{ \n \"base_requests\": [ \n  { \n  \"is_roundtrip\": true, \n  \"name\": \"297\", \n   \"stops\": [ \n     \"Biryulyovo Zapadnoye\", \n   \"Biryulyovo Tovarnaya\", \n     \"Universam\", \n   \"Biryulyovo Zapadnoye\" \n   ], \n   \"type\": \"Bus\" \n  }, \n  { \n   \"is_roundtrip\": false, \n   \"name\": \"635\", \n    \"stops\": [ \n    \"Biryulyovo Tovarnaya\", \n     \"Universam\", \n    \"Prazhskaya\" \n   ], \n   \"type\": \"Bus\" \n  }, \n  { \n    \"latitude\": 55.574371, \n   \"longitude\": 37.6517, \n     \"name\": \"Biryulyovo Zapadnoye\", \n      \"road_distances\": { \n         \"Biryulyovo Tovarnaya\": 2600 \n     }, \n      \"type\": \"Stop\" \n   }, \n    { \n     \"latitude\": 55.587655, \n     \"longitude\": 37.645687, \n     \"name\": \"Universam\", \n     \"road_distances\": { \n          \"Biryulyovo Tovarnaya\": 1380, \n       \"Biryulyovo Zapadnoye\": 2500, \n       \"Prazhskaya\": 4650 \n     }, \n    \"type\": \"Stop\" \n }, \n { \n    \"latitude\": 55.592028, \n   \"longitude\": 37.653656, \n    \"name\": \"Biryulyovo Tovarnaya\", \n   \"road_distances\": { \n        \"Universam\": 890 \n    }, \n    \"type\": \"Stop\" \n  }, \n   { \n    \"latitude\": 55.611717, \n   \"longitude\": 37.603938, \n   \"name\": \"Prazhskaya\", \n     \"road_distances\": {}, \n     \"type\": \"Stop\" \n   } \n ], \n    \"render_settings\": { \n  \"bus_label_font_size\": 20, \n  \"bus_label_offset\": [ \n     7, \n     15 \n    ], \n   \"color_palette\": [ \n    \"green\", \n    [ \n     255, \n      160, \n      0 \n    ], \n   \"red\" \n  ], \n  \"height\": 200, \n \"line_width\": 14, \n \"padding\": 30, \n  \"stop_label_font_size\": 20, \n  \"stop_label_offset\": [ \n      7, \n      -3 \n  ], \n  \"stop_radius\": 5, \n  \"underlayer_color\": [ \n      255, \n      255, \n    255, \n     0.85 \n   ], \n  \"underlayer_width\": 3, \n   \"width\": 200 \n  }, \n  \"routing_settings\": { \n   \"bus_velocity\": 40, \n   \"bus_wait_time\": 6 \n   }, \n \"stat_requests\": [ \n    { \n       \"id\": 1, \n    \"name\": \"297\", \n     \"type\": \"Bus\" \n   }, \n   { \n     \"id\": 2, \n     \"name\": \"635\", \n    \"type\": \"Bus\" \n  }, \n    { \n   \"id\": 3, \n \"name\": \"Universam\", \n   \"type\": \"Stop\" \n  }, \n  { \n  \"from\": \"Biryulyovo Zapadnoye\", \n    \"id\": 4, \n    \"to\": \"Universam\", \n   \"type\": \"Route\" \n  }, \n { \n  \"from\": \"Biryulyovo Zapadnoye\", \n  \"id\": 5, \n \"to\": \"Prazhskaya\", \n \"type\": \"Route\" \n} \n] \n} \n "};

        //std::istringstream aa(a);

        JsonReader reader(std::cin, std::cout);

        std::vector<StatRequest> stat_requests;
        RenderSettings settings;
        TransportRouter router(catalogue);
        reader.FillTransportCatalogue(catalogue, stat_requests, settings, router);
        router.BuildRouter();
        MapRenderer renderer(settings);
        RequestHandler request_handler(catalogue, renderer);
        
        reader.PrintStat(catalogue, stat_requests, request_handler, router);

    }