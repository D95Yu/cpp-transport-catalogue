#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */
using namespace json;
using namespace transport_catalogue;
using namespace std::literals;
using namespace render;

namespace json_reader {
    JsonReader::JsonReader(std::istream& input, std::ostream& output) 
        : input_(input), output_(output) {}

    Node JsonReader::LoadInputNode() {
        json::Document doc = Load(input_);
        return doc.GetRoot();
    }

    Stop JsonReader::GetStopFromNode(const Dict& dict) {
        Stop stop;
        stop.name = dict.at("name").AsString();
        geo::Coordinates coordinates;
        coordinates.lat = dict.at("latitude").AsDouble();
        coordinates.lng = dict.at("longitude").AsDouble();
        stop.coordinates = coordinates;
        return stop;
    }

    Bus JsonReader::GetBusFromNode(TransportCatalogue& catalogue, const Dict& dict) {
        Bus bus;
        bus.name = dict.at("name").AsString();
        bus.is_roundtrip = dict.at("is_roundtrip").AsBool();

        Array array;
        try {
            array = dict.at("stops").AsArray();
            for (const auto& node : array) {
                bus.stops.push_back(catalogue.FindStop(node.AsString()));
            }
            if (!bus.is_roundtrip) {
                size_t size = bus.stops.size() - 1;
                for (size_t i = size; i > 0; --i) {
                    bus.stops.push_back(bus.stops[i - 1]);
                }
            }
            
        }catch(...) {
            std::cout << "GetBusFromNode invalid error"s;
        }
        return bus;
    }

    void JsonReader::SetStopsDistance(TransportCatalogue& catalogue, const Dict& dict) {
        Dict stops_dist;
        try {
            stops_dist = dict.at("road_distances").AsMap();
            for (auto& [key, value] : stops_dist) {
                catalogue.SetDistanceBtwStops(catalogue.FindStop(dict.at("name").AsString()), catalogue.FindStop(key), static_cast<size_t>(value.AsInt()));
            }
        }catch(...) {
            std::cout << "SetStopDistance invalid error"s;
        }
    }

    void JsonReader::ParseNodeBase(TransportCatalogue& catalogue, const Array& array) {
        for (const auto& request : array) {
            Dict dict = request.AsMap();
            if (dict.at("type"s).AsString() == "Stop"s) {
                Stop stop = GetStopFromNode(dict);
                catalogue.AddStop(std::move(stop));
            }
        }
        for (const auto& request : array) {
            Dict dict = request.AsMap();
            if (dict.at("type").AsString() == "Stop") {
                SetStopsDistance(catalogue, dict);
            }
        }
        for (const auto& request : array) {
            Dict dict = request.AsMap();
            if (dict.at("type").AsString() == "Bus") {
                Bus bus = GetBusFromNode(catalogue, dict);
                catalogue.AddBus(std::move(bus));
            }
        }

    }

    svg::Color JsonReader::GetNodeColor(const Node& node) {
        if (node.IsString()) {
            return node.AsString();
        }
        Array array = node.AsArray();
            if (array.size() > 3) {
                svg::Rgba rgba(static_cast<uint8_t>(array[0].AsInt()), static_cast<uint8_t>(array[1].AsInt()), static_cast<uint8_t>(array[2].AsInt()), array[3].AsDouble());
                return rgba;
            }
        svg::Rgb rgb(static_cast<uint8_t>(array[0].AsInt()), static_cast<uint8_t>(array[1].AsInt()), static_cast<uint8_t>(array[2].AsInt()));
        return rgb;  
    }

    void JsonReader::FillStatRequests(const Array& array, std::vector<StatRequest>& stat_requests) {
        for (auto& arr : array) {
            try {
                Dict dict = arr.AsMap();
                StatRequest request;
                request.id = dict.at("id").AsInt();
                request.type = dict.at("type").AsString();
                if (dict.at("type").AsString() == "Map") {
                    stat_requests.emplace_back(request);
                    continue;
                }
                request.name = dict.at("name").AsString();
                stat_requests.emplace_back(request);
            }catch(...) {
                std::cout << "FillStatRequests invalid error"s;
            }
        }
        
    }

    void JsonReader::FillRenderSettings(const Dict& dict, RenderSettings& settings) {
        try {
            settings.width = dict.at("width"s).AsDouble();
            settings.height = dict.at("height"s).AsDouble();
            settings.padding = dict.at("padding"s).AsDouble();
            settings.line_width = dict.at("line_width"s).AsDouble();
            settings.stop_radius = dict.at("stop_radius"s).AsDouble();
            settings.bus_label_font_size = dict.at("bus_label_font_size"s).AsInt();
            settings.stop_label_font_size = dict.at("stop_label_font_size"s).AsInt();
            settings.underlayer_width = dict.at("underlayer_width"s).AsDouble();

            Array bus_label_offset = dict.at("bus_label_offset"s).AsArray();
            settings.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};

            Array stop_label_offset = dict.at("stop_label_offset"s).AsArray();
            settings.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

            settings.underlayer_color = GetNodeColor(dict.at("underlayer_color"s));

            Array color_palette = dict.at("color_palette"s).AsArray();
            for (const auto& color : color_palette) {
                settings.color_palette.emplace_back(GetNodeColor(color));
            }

        }catch (...) {
            std::cout << "FillRenderSettings invalid error"s;
        }
    }

    void JsonReader::FillTransportCatalogue(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, RenderSettings& settings) {
        Node first_node = LoadInputNode();
        try {
            ParseNodeBase(catalogue, first_node.AsMap().at("base_requests"s).AsArray());
            FillRenderSettings(first_node.AsMap().at("render_settings"s).AsMap(), settings);
            FillStatRequests(first_node.AsMap().at("stat_requests"s).AsArray(), stat_requests);
        }catch(...) {
            std::cout << "FillTransportCatalogue invalid error"s;
        }
    }

    Dict JsonReader::GetStopByRequest(TransportCatalogue& catalogue, const StatRequest& stat_request) {
        Dict dict;
        const auto found_stop = catalogue.FindStop(stat_request.name);
        dict["request_id"] = stat_request.id;
        if (!found_stop) {
            dict["error_message"] = Node("not found"s);
            return dict;
        }
        const auto buses_by_stop = catalogue.FindBusesByStop(catalogue.FindStop(found_stop->name));
        Array array;
        if (buses_by_stop) {
            std::set<std::string> buses;
            for (const auto& bus : *buses_by_stop) {
                buses.insert(bus->name);
            }
            for (const auto& bus : buses) {
                array.emplace_back(Node(bus));
            }
        }
        dict["buses"] = Node(array);
        return dict;
    }

    Dict JsonReader::GetBusByRequest(TransportCatalogue& catalogue, const StatRequest& stat_request) {
        Dict dict;
        dict["request_id"] = stat_request.id;
        const auto found_bus = catalogue.FindBus(stat_request.name);
        if (!found_bus) {
            dict["error_message"] = Node("not found"s);
            return dict;
        }
        int route_length = static_cast<int>(GetRouteLength(catalogue, *found_bus));
        dict["curvature"] = Node(route_length / GetGeoRouteLength(*found_bus));
        dict["route_length"] = Node(route_length);
        dict["stop_count"] = Node(GetStopsNum(*found_bus));
        dict["unique_stop_count"] = Node(GetUniqueStopsNum(*found_bus));
        return dict;
    }

    Dict JsonReader::GetMapByRequest(const StatRequest& stat_request, RequestHandler& request_handler) {
        Dict dict;
        dict["request_id"] = stat_request.id;
        svg::Document map = request_handler.RenderMap();
        std::ostringstream strm;
        map.Render(strm);
        dict["map"] = strm.str();
        return dict;
    }

    void JsonReader::PrintStat(TransportCatalogue& catalogue, const std::vector<StatRequest>& stat_requests, RequestHandler& request_handler) {
        Array result;
        for (size_t i = 0; i < stat_requests.size(); ++i) {
            if (stat_requests[i].type == "Stop") {
                Dict dict = GetStopByRequest(catalogue, stat_requests[i]);
               result.emplace_back(Node(dict));
            }else if(stat_requests[i].type == "Bus") {
                Dict dict = GetBusByRequest(catalogue, stat_requests[i]);
                result.emplace_back(Node(dict));
            }else if(stat_requests[i].type == "Map") {
                Dict dict = GetMapByRequest(stat_requests[i], request_handler);
                result.emplace_back(Node(dict));
            }
        }
        const json::Document doc{Node(result)};
        Print(doc, output_);
    }
}

