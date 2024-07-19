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
	class TransportCatalogue {
		public:

	struct Stop {
		std::string name;
		geo::Coordinates coordinates;
	};

	struct Bus {
		std::string name;
		std::vector<const Stop*> stops;
	};

	void AddBus(const Bus& bus);
	void AddStop(const Stop& stop);
	const Bus* FindBus(std::string_view name) const;
	const Stop* FindStop(std::string_view name) const;
	const std::unordered_set<const Bus*> FindBusesByStop(const Stop*) const;

	private: 
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stopname_to_buses_;
};

	namespace get {
		int UniqueStopsNum(const TransportCatalogue::Bus& bus);
		int StopsNum(const TransportCatalogue::Bus& bus);
		double RouteLength(const TransportCatalogue::Bus& bus);
	}
}

