#include "transport_router.h"

void TransportRouter::BuildRouter() {
    graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(catalogue_.GetStopsCount() * 2);
    AddStops();
    const auto* buses = catalogue_.GetBuses();
    if (buses) {
        for (const auto [_ , bus] : *buses) {
            AddBus(bus);
        }
        router_ = std::make_unique<graph::Router<double>>(*graph_);
    }
}

std::optional<TransportRouter::RouteItems> TransportRouter::FindRoute(std::string_view from, 
                                                std::string_view to) {
    const Stop* stop_from = catalogue_.FindStop(from);
    const Stop* stop_to = catalogue_.FindStop(to);
    RouteItems items_info;
    auto router_info = router_->BuildRoute(GetStopVertexes(stop_from).first, GetStopVertexes(stop_to).first);
    if (router_info.has_value()) {
        items_info.total_time = router_info->weight;
        for (const auto& edge_id : router_info->edges) {
            items_info.items.push_back(edge_id_to_item_.at(edge_id));
        }
        return items_info;
    }
    return std::nullopt;
}

void TransportRouter::AddStops() {
    const auto* stops = catalogue_.GetStops();
    if (stops) {
        graph::VertexId vertex_id = 0;
        for (const auto& [stop_name, stop] : *stops) {
            vetrexes_to_stop_[stop] = {vertex_id, vertex_id + 1};
            graph::EdgeId edge_id = graph_->AddEdge({vertex_id, vertex_id + 1, 
                                                    static_cast<double>(bus_wait_time_)});
            Item item{"Wait", stop->name, static_cast<double>(bus_wait_time_), 1};
            edge_id_to_item_[edge_id] = std::move(item);
            vertex_id += 2;
        }
    }
}

void TransportRouter::AddBusEdge(const Stop* from, const Stop* to, const std::string& busname, 
                                    int span_count, size_t distance) {
    Item item;
    item.type = "Bus";
    item.name = busname;
    item.time = static_cast<double>(distance) / (bus_velocity_ * 100.0 / 6.0);
    item.span = span_count;
    graph::EdgeId edge_id = graph_->AddEdge({GetStopVertexes(from).second, GetStopVertexes(to).first, item.time});
    edge_id_to_item_[edge_id] = std::move(item);
}

void TransportRouter::AddBus(const Bus* bus) {
    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        size_t distance = 0;
        size_t distance_inverse = 0;
        for (size_t j = i; j < bus->stops.size() - 1; ++j) {
            distance += catalogue_.GetDistanceBtwStops(bus->stops[j], bus->stops[j + 1]);
            AddBusEdge(bus->stops[i], bus->stops[j + 1], bus->name, static_cast<int>(j - i + 1), distance);
            if (!bus->is_roundtrip) {
                distance_inverse += catalogue_.GetDistanceBtwStops(bus->stops[j + 1], bus->stops[j]);
                AddBusEdge(bus->stops[j + 1], bus->stops[i], bus->name, static_cast<int>(j - i + 1), distance_inverse);
            }
        }
    }
}