#include "transport_catalogue.h"

#include <algorithm>
#include <utility>

namespace transport_catalogue {
    void TransportCatalogue::AddBus(const Bus& bus) {
        buses_.push_back(bus);
        busname_to_bus_[buses_.back().name] = &buses_.back();
        for (const auto& stop : buses_.back().stops) {
            stopname_to_buses_[stop->name].insert(&buses_.back());
        }
    }

    void TransportCatalogue::AddStop(const Stop& stop) {
        stops_.push_back(stop);
        stopname_to_stop_[stops_.back().name] = &stops_.back();
    }

    const Bus* TransportCatalogue::FindBus(std::string_view name) const {
        auto founded_bus = busname_to_bus_.find(name);
        return founded_bus != busname_to_bus_.end() ? founded_bus->second : nullptr;
    }

    const Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto founded_stop = stopname_to_stop_.find(name);
        return founded_stop != stopname_to_stop_.end() ? founded_stop->second : nullptr;
    }

    const std::unordered_set<const Bus*>* TransportCatalogue::FindBusesByStop(const Stop* stop) const {
        auto founded_buses = stopname_to_buses_.find(stop->name);
        return founded_buses != stopname_to_buses_.end() ? &founded_buses->second : nullptr;
    }
    int GetUniqueStopsNum(const Bus& bus) {
        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop : bus.stops) {
            unique_stops.insert(stop->name);
        }
        return static_cast<int>(unique_stops.size());
    }

    int GetStopsNum(const Bus& bus) {
            return static_cast<int>(bus.stops.size());
    }

    double GetRouteLength(const Bus& bus) {
        double route_length = 0.;
        for (size_t i = 0; i + 1 < bus.stops.size(); ++i) {
            route_length += geo::ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
        }
        return route_length;
    }
}

