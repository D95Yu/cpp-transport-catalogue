#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <set>
#include <sstream>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "request_handler.h"
#include "json.h"
#include "json_builder.h"
#include "geo.h"
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace json_reader {
    struct StatRequest {
        int id;
        std::string type;
        std::string name;
        std::string stop_to;
        std::string stop_from;
    };

    class JsonReader {
    public: 
        JsonReader(std::istream& input, std::ostream& output);
        void FillTransportCatalogue(transport_catalogue::TransportCatalogue& catalogue, std::vector<StatRequest>& stat_requests, 
                                        render::RenderSettings& settings, TransportRouter& router);
        void PrintStat(transport_catalogue::TransportCatalogue& catalogue, const std::vector<StatRequest>& stat_requests, 
                        RequestHandler& request_handler, TransportRouter& router);
        
    private:
        std::istream& input_;
        std::ostream& output_;

        json::Node GetRouteByRequest(const StatRequest& stat_request, TransportRouter& router);
        json::Node GetMapByRequest(const StatRequest& stat_request, RequestHandler& request_handler);
        svg::Color GetNodeColor(const json::Node& node);
        void FillRenderSettings(const json::Dict& dict, render::RenderSettings& render_settings);
        void FillRoutingSettings(const json::Dict& dict, TransportRouter& router);
        void FillStatRequests(const json::Array& array, std::vector<StatRequest>& stat_requests);
        json::Node GetStopByRequest(transport_catalogue::TransportCatalogue& catalogue, const StatRequest& stat_request);
        json::Node GetBusByRequest(transport_catalogue::TransportCatalogue& catalogue, const StatRequest& stat_request);
        Stop GetStopFromNode(const json::Dict& dict);
        Bus GetBusFromNode(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& dict);
        void SetStopsDistance(transport_catalogue::TransportCatalogue& catalogue, const json::Dict& dict);
        void ParseNodeBase(transport_catalogue::TransportCatalogue& catalogue, const json::Array& array);
        json::Node LoadInputNode();


    };
}

