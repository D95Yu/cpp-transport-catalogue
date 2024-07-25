#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo.h"
namespace transport_catalogue {

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
	};

	class TransportCatalogue {
	public:
	void AddBus(const Bus& bus);
	void AddStop(const Stop& stop);
	const Bus* FindBus(std::string_view name) const;
	const Stop* FindStop(std::string_view name) const;
	const std::unordered_set<const Bus*>* FindBusesByStop(const Stop* stop) const;

	private: 
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stopname_to_buses_;
};

	int GetUniqueStopsNum(const Bus& bus);
	int GetStopsNum(const Bus& bus);
	double GetRouteLength(const Bus& bus);
}

