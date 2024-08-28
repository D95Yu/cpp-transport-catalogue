#include "request_handler.h"


using namespace transport_catalogue;
using namespace render;

//inline auto comp = [](const Stop* lhs, const Stop* rhs) {return lhs->name < rhs->name;};

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, MapRenderer& renderer) 
    : catalogue_(catalogue), renderer_(renderer) {}

svg::Document RequestHandler::RenderMap() {
    svg::Document doc;

    const auto buses = catalogue_.GetBuses();
    if (!buses) {
        std::cout << "No buses";
    }
    renderer_.FillDocument(doc, buses);
    return doc;
}