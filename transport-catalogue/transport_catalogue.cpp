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

    size_t TransportCatalogue::GetDistanceBtwStops(const Stop* from, const Stop* to) const{
        auto founded_dist = distance_btw_stops_.find({from, to});
        if (founded_dist == distance_btw_stops_.end()) {
            founded_dist = distance_btw_stops_.find({to, from});
        }
        return founded_dist != distance_btw_stops_.end() ? founded_dist->second : 0;
    }

    const std::map<std::string_view, const Bus*>* TransportCatalogue::GetBuses() const {
        return &busname_to_bus_;
    }

    const std::unordered_map<std::string_view, const Stop*>* TransportCatalogue::GetStops() const {
        return &stopname_to_stop_;
    }

    size_t TransportCatalogue::GetStopsCount() const {
        return stops_.size();
    }

	void TransportCatalogue::SetDistanceBtwStops(const Stop* from, const Stop* to, size_t distance) {
        distance_btw_stops_.insert({{from, to}, distance});
    }

    size_t TransportCatalogue::Hasher::operator()(const std::pair<const Stop*, const Stop*> stops) const{
        size_t h_first = hasher_(stops.first);
        size_t h_second = hasher_(stops.second);
        return h_first * 53 + h_second * (53 * 53);
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

    double GetGeoRouteLength(const Bus& bus) {
        double route_length = 0.;
        for (size_t i = 0; i + 1 < bus.stops.size(); ++i) {
            route_length += geo::ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
        }
        return route_length;
    }

    size_t GetRouteLength(const TransportCatalogue& catalogue, const Bus& bus) {
        size_t route_length = 0.;
        for (size_t i = 0; i + 1 < bus.stops.size(); ++i) {
            route_length += catalogue.GetDistanceBtwStops(bus.stops[i], bus.stops[i + 1]);
        }
        return route_length;
    }
}
