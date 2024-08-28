#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршртутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace svg;

namespace render {
    MapRenderer::MapRenderer(const RenderSettings& settings) 
        : settings_(settings) {}

    svg::Polyline MapRenderer::GetPolyline(std::vector<const Stop*> stops) {
        svg::Polyline polyline;
        for (const auto& stop : stops) {
            polyline.AddPoint(proj_(stop->coordinates));
        }
        SetLineProperties(polyline);
        return polyline;
    }

    SphereProjector MapRenderer::GetSphereProjector(const std::set<const Stop*, decltype(comp)> stops) {
        return SphereProjector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
    }

    Color MapRenderer::GetPaletteColor() {
        if (color_index_ >= settings_.color_palette.size()) {
            color_index_ = 0;
        }
        return settings_.color_palette[color_index_];
    }

    Circle MapRenderer::GetCircle(const Stop* stop) {
        Circle circle;
        circle.SetCenter(proj_(stop->coordinates));
        circle.SetRadius(settings_.stop_radius);
        circle.SetFillColor("white");
        return circle;
    }

    Text MapRenderer::GetBaseText(const Stop* stop, const std::string& busname, bool is_under_layer) {
        Text text;
        text.SetPosition(proj_(stop->coordinates));
        text.SetOffset({settings_.bus_label_offset.first, settings_.bus_label_offset.second});
        text.SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size));
        text.SetFontFamily("Verdana");
        text.SetData(busname);
        text.SetFontWeight("bold");
        if (is_under_layer) {
            SetUnderLayerText(text);
            return text;
        }
        text.SetFillColor(GetPaletteColor());
        return text;
    }

    Text MapRenderer::GetStopText(const Stop* stop, bool is_under_layer) {
        Text text;
        
        text.SetPosition(proj_(stop->coordinates));
        text.SetOffset({settings_.stop_label_offset.first, settings_.stop_label_offset.second});
        text.SetFontSize(static_cast<uint32_t>(settings_.stop_label_font_size));
        text.SetFontFamily("Verdana");
        text.SetData(stop->name);
        if (is_under_layer) {
            SetUnderLayerText(text);
            return text;
        }
        text.SetFillColor("black");
        return text;
    }

    void MapRenderer::SetUnderLayerText(Text& text) {
        text.SetFillColor(settings_.underlayer_color);
        text.SetStrokeColor(settings_.underlayer_color);
        text.SetStrokeWidth(settings_.underlayer_width);
        text.SetStrokeLineCap(StrokeLineCap::ROUND);
        text.SetStrokeLineJoin(StrokeLineJoin::ROUND);
    }

    void MapRenderer::SetLineProperties(Polyline& polyline) {
        polyline.SetFillColor("none");
        polyline.SetStrokeWidth(settings_.line_width);
        polyline.SetStrokeLineCap(StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        polyline.SetStrokeColor(GetPaletteColor());
    }

    void MapRenderer::FillDocument(svg::Document& doc, const std::map<std::string_view, const Bus*>* buses) {
    
        std::set<const Stop*, decltype(comp)> stops_(comp);
        for (const auto& [busname, bus] : *buses) {
            for (const auto& stop : bus->stops) {
                stops_.insert(stop);
            }
        }
        proj_ = GetSphereProjector(stops_);
        std::vector<svg::Text> text_;
        for (const auto& [busname, bus] : *buses) {
            if (bus->stops.empty()) {
                continue;
            }
            text_.emplace_back(GetBaseText(bus->stops[0], bus->name, true));
            text_.emplace_back(GetBaseText(bus->stops[0], bus->name, false));
            if (!(bus->is_roundtrip)) {
                if (bus->stops[0] != bus->stops[(bus->stops.size() / 2)]) {
                    text_.emplace_back(GetBaseText(bus->stops[(bus->stops.size() / 2)], bus->name, true));
                    text_.emplace_back(GetBaseText(bus->stops[(bus->stops.size() / 2)], bus->name, false));
                }
            }
            doc.Add(GetPolyline(bus->stops));
            ++color_index_;
        }
        for (const auto& text : text_) {
            doc.Add(text);
        }
        for (const auto& stop : stops_) {
            doc.Add(GetCircle(stop));
        }
        for (const auto& stop : stops_) {
            doc.Add(GetStopText(stop, true));
            doc.Add(GetStopText(stop, false));
        }
    }    
}