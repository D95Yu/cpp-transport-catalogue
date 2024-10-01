#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <optional>
#include <string_view>
#include <string>
#include <set>

#include "transport_catalogue.h"
#include "transport_router.h"
#include "svg.h"
#include "geo.h"
#include "map_renderer.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)


class RequestHandler {
public:

RequestHandler(const transport_catalogue::TransportCatalogue& catalogue,  render::MapRenderer& renderer);

svg::Document RenderMap();
    
private:
const transport_catalogue::TransportCatalogue& catalogue_;
render::MapRenderer& renderer_;
};