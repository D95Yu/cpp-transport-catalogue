#include "input_reader.h"

#include <algorithm>
#include <cassert>
#include <deque>
#include <iterator>
#include <utility>

namespace input_reader {
    RequestType GetRequestType(std::string_view request) {
        if (request == "Stop") {
            return RequestType::STOP;
        }else if (request == "Bus") {
            return RequestType::BUS;
        }
        return RequestType::UNKNOWN;
    }

    namespace detail {
        /**
        * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
        */
        geo::Coordinates ParseCoordinates(std::string_view str) {
            static const double nan = std::nan("");

            auto not_space = str.find_first_not_of(' ');
            auto comma = str.find(',');

            if (comma == str.npos) {
                return {nan, nan};
            }

            auto not_space2 = str.find_first_not_of(' ', comma + 1);

            double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
            double lng = std::stod(std::string(str.substr(not_space2)));

            return {lat, lng};
        }

        /**
        * Удаляет пробелы в начале и конце строки
        */
        std::string_view Trim(std::string_view string) {
            const auto start = string.find_first_not_of(' ');
            if (start == string.npos) {
                return {};
            }
            return string.substr(start, string.find_last_not_of(' ') + 1 - start);
        }

        /**
        * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
        */
        std::vector<std::string_view> Split(std::string_view string, char delim) {
            std::vector<std::string_view> result;

            size_t pos = 0;
            while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
                auto delim_pos = string.find(delim, pos);
                if (delim_pos == string.npos) {
                    delim_pos = string.size();
                }
                if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
                    result.push_back(substr);
                }
                pos = delim_pos + 1;
            }

            return result;
        }

        /**
        * Парсит маршрут.
        * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
        * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
        */
        std::vector<std::string_view> ParseRoute(std::string_view route) {
            if (route.find('>') != route.npos) {
                return Split(route, '>');
            }

            auto stops = Split(route, '-');
            std::vector<std::string_view> results(stops.begin(), stops.end());
            results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

            return results;
        }

        CommandDescription ParseCommandDescription(std::string_view line) {
            auto colon_pos = line.find(':');
            if (colon_pos == line.npos) {
                return {};
            }

            auto space_pos = line.find(' ');
            if (space_pos >= colon_pos) {
                return {};
            }

            auto not_space = line.find_first_not_of(' ', space_pos);
            if (not_space >= colon_pos) {
                return {};
            }

            return {std::string(line.substr(0, space_pos)),
                    std::string(line.substr(not_space, colon_pos - not_space)),
                    std::string(line.substr(colon_pos + 1))};
        }   
    }

    void InputReader::ParseLine(std::string_view line) {
        auto command_description = detail::ParseCommandDescription(line);
        if (command_description) {
            commands_.push_back(std::move(command_description));
        }
    }

    void InputReader::ApplyCommands([[maybe_unused]] transport_catalogue::TransportCatalogue& catalogue) const {
        using namespace transport_catalogue;
        std::deque<CommandDescription> buffer;
        for (const auto& request : commands_) {
            RequestType request_type = GetRequestType(request.command);
            if (request_type == RequestType::BUS) {
                buffer.push_back(std::move(request));
            }else if (request_type == RequestType::STOP) {
                TransportCatalogue::Stop stop;
                stop.name = request.id;
                stop.coordinates = detail::ParseCoordinates(request.description);
                catalogue.AddStop(std::move(stop));
            }else {
                continue;
            }
        }
        for (const auto& request_bus : buffer) {
            TransportCatalogue::Bus bus;
            bus.name = request_bus.id;
            std::vector<std::string_view> stops_names = detail::ParseRoute(request_bus.description);
            for (const auto& stop_name : stops_names) {
                bus.stops.push_back(catalogue.FindStop(stop_name));
            }
            catalogue.AddBus(std::move(bus));
        }
    }
}