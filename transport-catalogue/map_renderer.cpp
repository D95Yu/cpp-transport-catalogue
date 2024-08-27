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
    
    /*RenderSettings MapRenderer::GetRenderSettings() const {
        return settings_;
    } */

    //auto comp = [](const Stop* lhs, const Stop* rhs) {return lhs->name < rhs->name;};
    SphereProjector MapRenderer::GetSphereProjector(const std::set<const Stop*, decltype(comp)> stops) {
        return SphereProjector(stops.begin(), stops.end(), settings_.width, settings_.height, settings_.padding);
    }

    Color MapRenderer::GetPaletteColor(size_t& color_index) {
        if (color_index >= settings_.color_palette.size()) {
            color_index = 0;
        }
        return settings_.color_palette[color_index];
    }

    Circle MapRenderer::GetCircle(const Stop* stop, SphereProjector& proj) {
        Circle circle;
        circle.SetCenter(proj(stop->coordinates));
        circle.SetRadius(settings_.stop_radius);
        circle.SetFillColor("white");
        return circle;
    }

    Text MapRenderer::GetBaseText(const Stop* stop, SphereProjector& proj,[[maybe_unused]] size_t& color_index, const std::string& busname, bool is_under_layer) {
        Text text;
        text.SetPosition(proj(stop->coordinates));
        text.SetOffset({settings_.bus_label_offset.first, settings_.bus_label_offset.second});
        text.SetFontSize(static_cast<uint32_t>(settings_.bus_label_font_size));
        text.SetFontFamily("Verdana");
        text.SetData(busname);
        text.SetFontWeight("bold");
        if (is_under_layer) {
            SetUnderLayerText(text);
            return text;
        }
        text.SetFillColor(GetPaletteColor(color_index));
        return text;
    }

    Text MapRenderer::GetStopText(const Stop* stop, SphereProjector& proj, bool is_under_layer) {
        Text text;
        
        text.SetPosition(proj(stop->coordinates));
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

    void MapRenderer::SetLineProperties(Polyline& polyline, size_t& color_index) {
        polyline.SetFillColor("none");
        polyline.SetStrokeWidth(settings_.line_width);
        polyline.SetStrokeLineCap(StrokeLineCap::ROUND);
        polyline.SetStrokeLineJoin(StrokeLineJoin::ROUND);
        polyline.SetStrokeColor(GetPaletteColor(color_index));
    }


}