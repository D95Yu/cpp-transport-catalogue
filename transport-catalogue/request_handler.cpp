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

svg::Polyline RequestHandler::GetPolyline(std::vector<const Stop*> stops, SphereProjector& proj, size_t& color_index) const {
    
    svg::Polyline polyline;
    for (const auto& stop : stops) {
        polyline.AddPoint(proj(stop->coordinates));
    }
    renderer_.SetLineProperties(polyline, color_index);
    return polyline;
}

svg::Document RequestHandler::RenderMap() {
    svg::Document doc;

    const auto buses = catalogue_.GetBuses();
    if (!buses) {
        std::cout << "No buses";
    }
    std::set<const Stop*, decltype(comp)> stops_(comp);
    for (const auto& [busname, bus] : *buses) {
        for (const auto& stop : bus->stops) {
            stops_.insert(stop);
        }
    }
    SphereProjector proj = renderer_.GetSphereProjector(stops_);
    size_t color_index = 0;
    std::vector<svg::Text> text_;
    for (const auto& [busname, bus] : *buses) {
        if (bus->stops.empty()) {
            continue;
        }
        text_.emplace_back(renderer_.GetBaseText(bus->stops[0], proj, color_index, bus->name, true));
        text_.emplace_back(renderer_.GetBaseText(bus->stops[0], proj, color_index, bus->name, false));
        if (!(bus->is_roundtrip)) {
            if (bus->stops[0] != bus->stops[(bus->stops.size() / 2)]) {
                text_.emplace_back(renderer_.GetBaseText(bus->stops[(bus->stops.size() / 2)], proj, color_index, bus->name, true));
                text_.emplace_back(renderer_.GetBaseText(bus->stops[(bus->stops.size() / 2)], proj, color_index, bus->name, false));
            }
        }
        doc.Add(GetPolyline(bus->stops, proj, color_index));
        ++color_index;
    }
    for (const auto& text : text_) {
        doc.Add(text);
    }
    for (const auto& stop : stops_) {
        doc.Add(renderer_.GetCircle(stop, proj));
    }
    for (const auto& stop : stops_) {
        doc.Add(renderer_.GetStopText(stop, proj, true));
        doc.Add(renderer_.GetStopText(stop, proj, false));
    }
    return doc;
}