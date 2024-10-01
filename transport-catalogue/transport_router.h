#pragma once

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"


class TransportRouter {
public:
    struct Item {
        std::string type;
        std::string name;
        double time;
        int span;
    };

    struct RouteItems {
        double total_time;
        std::vector<Item> items;
    };

    TransportRouter() = default;

    TransportRouter(const transport_catalogue::TransportCatalogue& catalogue) : catalogue_(catalogue) {
    }

    void BuildRouter();

    //метод задания настроек 
    void SetTimeVelocitySettings(int time, double velocity) {
        bus_wait_time_ = time;
        bus_velocity_  = velocity;
    }

    std::optional<RouteItems> FindRoute(std::string_view from, std::string_view to);

private:
    const transport_catalogue::TransportCatalogue& catalogue_;

    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.;
    std::unique_ptr<graph::DirectedWeightedGraph<double>> graph_;
    std::unique_ptr<graph::Router<double>> router_;
    std::map<const Stop*, std::pair<graph::VertexId, graph::VertexId>> vetrexes_to_stop_;
    std::map<graph::EdgeId, Item> edge_id_to_item_;
    
    std::pair<graph::VertexId, graph::VertexId> GetStopVertexes(const Stop* stop) {
        return vetrexes_to_stop_.at(stop);
    }

    void AddStops();

    void AddBusEdge(const Stop* from, const Stop* to, const std::string& busname, int span_count, size_t distance);

    void AddBus(const Bus* bus);

    
};