#include "svg.h"

namespace svg {

using namespace std::literals;

void OstreamColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}

void OstreamColorPrinter::operator()(const std::string& color) const {
    out << color;
}

void OstreamColorPrinter::operator()(const Rgb& rgb) const {
    out << "rgb("sv << static_cast<short>(rgb.red) << ","sv << static_cast<short>(rgb.green) 
        << ","sv << static_cast<short>(rgb.blue) << ")"sv;
} 

void OstreamColorPrinter::operator()(const Rgba& rgba) const {
    out << "rgba("sv << static_cast<short>(rgba.red) << ","sv << static_cast<short>(rgba.green) << ","sv
        << static_cast<short>(rgba.blue) << ","sv << (rgba.opacity) << ")"sv;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(OstreamColorPrinter{out}, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (const auto& point : points_) {
        if (!is_first) {
            out << " "sv;
        }
        out << point.x << ","sv << point.y;
        is_first = false;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
    pos_x_y_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_dx_dy_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    std::swap(font_family_, font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    std::swap(font_weight_, font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    for (const auto& ch : data) {
        if (ch == '"') {
            data_ += "&quot;"s;
            continue; 
        }
        if (ch == '\'') {
            data_ += "&apos;"s;
            continue;
        }
        if (ch == '<') {
            data_ += "&lt;"s;
            continue;
        }
        if (ch == '>') {
            data_ += "&gt;"s;
            continue;
        }
        if (ch == '&') {
            data_ += "&amp;"s;
            continue;
        }
        data_.push_back(ch);
    }
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(out);
    out << " x=\""sv << pos_x_y_.x << "\" y=\""sv << pos_x_y_.y << "\" dx=\""sv;
    out << offset_dx_dy_.x << "\" dy=\""sv << offset_dx_dy_.y << "\" font-size=\""sv;
    out << font_size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv << data_ << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg