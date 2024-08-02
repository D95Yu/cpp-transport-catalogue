#include "stat_reader.h"
#include "input_reader.h"

#include <iostream>
#include <iomanip>  
#include <set>
#include <unordered_set>

namespace stat_reader {
    void ParseAndPrintStat(const transport_catalogue::TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
        using namespace transport_catalogue;
        using namespace input_reader;
        output << request;
        std::string_view request_type = request.substr(0, request.find(' '));
        if (GetRequestType(request_type) == RequestType::BUS) {
            std::string_view bus_num = request.substr(request.find(' ') + 1);
            const Bus* found_bus = transport_catalogue.FindBus(bus_num);
            if (found_bus == nullptr) {
                output << ": not found" << std::endl; 
            }else {
                output << ": " << GetStopsNum(*found_bus)
                << " stops on route, " << GetUniqueStopsNum(*found_bus) 
                << " unique stops, ";
                double route_length = GetRouteLength(transport_catalogue, *found_bus);
                output << std::setprecision(6) << route_length 
                << " route length, " << std::setprecision(6) 
                << route_length / GetGeoRouteLength(*found_bus) 
                << " curvature" << std::endl;
            }
        }else if(GetRequestType(request_type) == RequestType::STOP) {
            std::string_view stop_name = request.substr(request.find(' ') + 1);
            const Stop* found_stop = transport_catalogue.FindStop(stop_name);
            if (found_stop == nullptr) {
                output << ": not found" << std::endl;
            }else{
                const std::unordered_set<const Bus*>* buses = transport_catalogue.FindBusesByStop(found_stop);
                if (!buses) {
                    output << ": no buses" << std::endl;
                }else {
                    output << ": buses";
                    std::set<std::string_view> buses_name;
                    for (const auto& bus : *buses) {
                        buses_name.insert(bus->name);
                    }
                    for (const auto& name : buses_name) {
                        output << " " << name;
                    }
                    output << std::endl;
                }
            }
        }
    }
}