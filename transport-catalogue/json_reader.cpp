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
            stops_dist = dict.at("road_distances").AsDict();
            for (auto& [key, value] : stops_dist) {
                catalogue.SetDistanceBtwStops(catalogue.FindStop(dict.at("name").AsString()), catalogue.FindStop(key), static_cast<size_t>(value.AsInt()));
            }
        }catch(...) {
            std::cout << "SetStopDistance invalid error"s;
        }
    }

    void JsonReader::ParseNodeBase(TransportCatalogue& catalogue, const Array& array) {
        for (const auto& request : array) {
            Dict dict = request.AsDict();
            if (dict.at("type"s).AsString() == "Stop"s) {
                Stop stop = GetStopFromNode(dict);
                catalogue.AddStop(std::move(stop));
            }
        }
        for (const auto& request : array) {
            Dict dict = request.AsDict();
            if (dict.at("type").AsString() == "Stop") {
                SetStopsDistance(catalogue, dict);
            }
        }
        for (const auto& request : array) {
            Dict dict = request.AsDict();
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
                Dict dict = arr.AsDict();
                StatRequest request;
                request.id = dict.at("id").AsInt();
                request.type = dict.at("type").AsString();
                if (dict.at("type").AsString() == "Map") {
                    stat_requests.emplace_back(request);
                    continue;
                }else if(dict.at("type").AsString() == "Route") {
                    request.stop_from = dict.at("from").AsString();
                    request.stop_to = dict.at("to").AsString();
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

    void JsonReader::FillRoutingSettings(const Dict& dict, TransportRouter& router) {
        int bus_wait_time = dict.at("bus_wait_time"s).AsInt();
        double bus_velocity = dict.at("bus_velocity"s).AsDouble();
        router.SetTimeVelocitySettings(bus_wait_time, bus_velocity);    
    }

    void JsonReader::FillTransportCatalogue(TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, 
                                                RenderSettings& settings, TransportRouter& router) {
        Node first_node = LoadInputNode();
        try {
            ParseNodeBase(catalogue, first_node.AsDict().at("base_requests"s).AsArray());
            FillRenderSettings(first_node.AsDict().at("render_settings"s).AsDict(), settings);
            FillRoutingSettings(first_node.AsDict().at("routing_settings"s).AsDict(), router);
            FillStatRequests(first_node.AsDict().at("stat_requests"s).AsArray(), stat_requests);
        }catch(...) {
            std::cout << "FillTransportCatalogue invalid error"s;
        }
    }

    Node JsonReader::GetStopByRequest(TransportCatalogue& catalogue, const StatRequest& stat_request) {
        Builder builder{};
        builder.StartDict().Key("request_id"s).Value(stat_request.id);
        const auto found_stop = catalogue.FindStop(stat_request.name);
        if (!found_stop) {
            builder.Key("error_message"s).Value("not found"s).EndDict();
            return builder.Build();
        }
        const auto buses_by_stop = catalogue.FindBusesByStop(catalogue.FindStop(found_stop->name));
        builder.Key("buses"s).StartArray();
        if (buses_by_stop) {
            std::set<std::string> buses;
            for (const auto& bus : *buses_by_stop) {
                buses.insert(bus->name);
            }
            for (const auto& bus : buses) {
                builder.Value(bus);
            }
        }
        builder.EndArray().EndDict();
        return builder.Build();
    }

    Node JsonReader::GetBusByRequest(TransportCatalogue& catalogue, const StatRequest& stat_request) {
        Builder builder{};
        builder.StartDict().Key("request_id"s).Value(stat_request.id);
        const auto found_bus = catalogue.FindBus(stat_request.name);
        if (!found_bus) {
            builder.Key("error_message"s).Value("not found"s).EndDict();
            return builder.Build();
        }
        int route_length = static_cast<int>(GetRouteLength(catalogue, *found_bus));
        builder.Key("curvature"s).Value(route_length / GetGeoRouteLength(*found_bus))
                .Key("route_length"s).Value(route_length)
                .Key("stop_count"s).Value(GetStopsNum(*found_bus))
                .Key("unique_stop_count"s).Value(GetUniqueStopsNum(*found_bus))
                .EndDict();
        return builder.Build();
    }

    Node JsonReader::GetMapByRequest(const StatRequest& stat_request, RequestHandler& request_handler) {
        Builder builder{};
        builder.StartDict().Key("request_id"s).Value(stat_request.id);
        svg::Document map = request_handler.RenderMap();
        std::ostringstream strm;
        map.Render(strm);
        builder.Key("map"s).Value(strm.str()).EndDict();
        return builder.Build();
    }

    Node JsonReader::GetRouteByRequest(const StatRequest& stat_request, TransportRouter& router) {
        Builder builder{};
        builder.StartDict();
        const auto found_route = router.FindRoute(stat_request.stop_from, stat_request.stop_to);
        if (!found_route) {
            builder.Key("error_message"s).Value("not found"s)
                    .Key("request_id"s).Value(stat_request.id).EndDict();
            return builder.Build();
        }
        builder.Key("request_id"s).Value(stat_request.id)
                .Key("total_time"s).Value(found_route.value().total_time)
                .Key("items"s).StartArray();
        for (const auto& item : found_route.value().items) {
            builder.StartDict();
            if (item.type == "Wait"s) {
                builder.Key("type"s).Value(item.type)
                        .Key("time"s).Value(item.time)
                        .Key("stop_name"s).Value(item.name);
            }else if(item.type == "Bus"s) {
                builder.Key("type"s).Value(item.type)
                        .Key("bus"s).Value(item.name)
                        .Key("span_count"s).Value(item.span)
                        .Key("time"s).Value(item.time);
            }
            builder.EndDict();
        }
        builder.EndArray().EndDict();
        return builder.Build();
    }

    void JsonReader::PrintStat(TransportCatalogue& catalogue, const std::vector<StatRequest>& stat_requests, 
                                    RequestHandler& request_handler, TransportRouter& router) {
        Builder builder{};
        builder.StartArray();
        for (size_t i = 0; i < stat_requests.size(); ++i) {
            if (stat_requests[i].type == "Stop") {
                builder.Value(GetStopByRequest(catalogue, stat_requests[i]).GetValue());
            }else if(stat_requests[i].type == "Bus") {
                builder.Value(GetBusByRequest(catalogue, stat_requests[i]).GetValue());
            }else if(stat_requests[i].type == "Map") {
                builder.Value(GetMapByRequest(stat_requests[i], request_handler).GetValue());
            }else if (stat_requests[i].type == "Route") {
                builder.Value(GetRouteByRequest(stat_requests[i], router).GetValue());
            }
        }
        builder.EndArray();
        const json::Document doc{builder.Build()};
        Print(doc, output_);
    }
}

