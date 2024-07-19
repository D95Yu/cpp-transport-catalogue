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
            const TransportCatalogue::Bus* finded_bus = transport_catalogue.FindBus(bus_num);
            if (finded_bus == nullptr) {
                output << ": not found" << std::endl; 
            }else {
                output << ": " << get::StopsNum(*finded_bus)
                << " stops on route, " << get::UniqueStopsNum(*finded_bus) 
                << " unique stops, " << std::setprecision(6) << get::RouteLength(*finded_bus) 
                << " route length" << std::endl;
            }
        }else if(GetRequestType(request_type) == RequestType::STOP) {
            std::string_view stop_name = request.substr(request.find(' ') + 1);
            const TransportCatalogue::Stop* finded_stop = transport_catalogue.FindStop(stop_name);
            if (finded_stop == nullptr) {
                output << ": not found" << std::endl;
            }else{
                const std::unordered_set<const TransportCatalogue::Bus*> buses = transport_catalogue.FindBusesByStop(finded_stop);
                if (buses.empty()) {
                    output << ": no buses" << std::endl;
                }else {
                    output << ": buses";
                    std::set<std::string_view> buses_name;
                    for (const auto& bus : buses) {
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