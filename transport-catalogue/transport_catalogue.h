#pragma once

#include <deque>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map> 

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

	class TransportCatalogue {
	public:
	void AddBus(const Bus& bus);
	void AddStop(const Stop& stop);
	const Bus* FindBus(std::string_view name) const;
	const Stop* FindStop(std::string_view name) const;
	const std::unordered_set<const Bus*>* FindBusesByStop(const Stop* stop) const;
	size_t GetDistanceBtwStops(const Stop* from, const Stop* to) const;
	void SetDistanceBtwStops(const Stop* from, const Stop* to, size_t distance);
	const std::map<std::string_view, const Bus*>* GetBuses() const;

	struct Hasher {
		size_t operator()(const std::pair<const Stop*, const Stop*> stops) const;

		private:
			std::hash<const void*> hasher_;
	};

	private: 
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	//std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::unordered_map<std::string_view, std::unordered_set<const Bus*>> stopname_to_buses_;
	std::unordered_map<const std::pair<const Stop*, const Stop*>, size_t, Hasher> distance_btw_stops_;
	};

	int GetUniqueStopsNum(const Bus& bus);
	int GetStopsNum(const Bus& bus);
	double GetGeoRouteLength(const Bus& bus);
	size_t GetRouteLength(const TransportCatalogue& catalogue, const Bus& bus);
}
