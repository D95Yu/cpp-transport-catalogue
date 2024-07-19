#include "transport_catalogue.h"

#include <algorithm>
#include <utility>

namespace transport_catalogue {
    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(std::move(bus));
        busname_to_bus_[buses_.back().name] = &buses_.back();
        for (const auto& stop : buses_.back().stops) {
            stopname_to_buses_[stop->name].insert(&buses_.back());
        }
    }

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(std::move(stop));
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    const TransportCatalogue::Bus* TransportCatalogue::FindBus(std::string_view name) const {
        return busname_to_bus_.count(name) ? busname_to_bus_.at(name) : nullptr;
    }

    const TransportCatalogue::Stop* TransportCatalogue::FindStop(std::string_view name) const {
        return stopname_to_stop_.count(name) ? stopname_to_stop_.at(name) : nullptr;
    }

    const std::unordered_set<const TransportCatalogue::Bus*> TransportCatalogue::FindBusesByStop(const TransportCatalogue::Stop* stop) const {
        std::unordered_set<const TransportCatalogue::Bus*> no_bus;
        return stopname_to_buses_.count(stop->name) ? stopname_to_buses_.at(stop->name) : no_bus;
    }
    
    namespace get {
        int UniqueStopsNum(const TransportCatalogue::Bus& bus) {
            std::unordered_set<std::string_view> unique_stops;
            for (const auto& stop : bus.stops) {
                unique_stops.insert(stop->name);
            }
            return static_cast<int>(unique_stops.size());
        }

        int StopsNum(const TransportCatalogue::Bus& bus) {
            return static_cast<int>(bus.stops.size());
        }

        double RouteLength(const TransportCatalogue::Bus& bus) {
            double route_length = 0.;
            for (size_t i = 0; i + 1 < bus.stops.size(); ++i) {
                route_length += geo::ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
            }
            return route_length;
        }
    }
}
