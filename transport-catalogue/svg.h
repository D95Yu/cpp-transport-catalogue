#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <variant>

namespace svg {

// Объявив в заголовочном файле константу со спецификатором inline,
// мы сделаем так, что она будет одной на все единицы трансляции,
// которые подключают этот заголовок.
// В противном случае каждая единица трансляции будет использовать свою копию этой константы

struct Rgb {
    Rgb() = default;

    Rgb(uint8_t r, uint8_t g, uint8_t b) 
        : red(r), green(g), blue(b) {
    }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba() = default;

    Rgba(uint8_t r, uint8_t g, uint8_t b, double o) 
        : red(r), green(g), blue(b), opacity(o) {
        }

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

struct OstreamColorPrinter {
    std::ostream& out;

    void operator()(std::monostate) const;
    void operator()(const std::string& color) const;
    void operator()(const Rgb& rgb) const;
    void operator()(const Rgba& rgba) const;
};



using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{"none"};

std::ostream& operator<<(std::ostream& out, const Color& color);


enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

inline std::ostream& operator<<(std::ostream& output, StrokeLineCap line_cap) {
    using namespace std::literals;
    switch(line_cap) {
        case StrokeLineCap::BUTT:
            output << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            output << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            output << "square"s;
            break;
    }
    return output;
}

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& output, StrokeLineJoin line_join) {
    using namespace std::literals;
    switch(line_join) {
        case StrokeLineJoin::ARCS:
            output << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            output << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            output << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            output << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            output << "round"s;
            break;
    }
    return output;
}

template <typename Owner>
class PathProps {
public: 
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }
protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;
        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }
        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }
private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

struct Point {
    Point() = default;
    Point(double x_, double y_)
        : x(x_)
        , y(y_) {
    }
    double x = 0;
    double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out_)
        : out(out_) {
    }

    RenderContext(std::ostream& out_, int indent_step_, int indent_ = 0)
        : out(out_)
        , indent_step(indent_step_)
        , indent(indent_) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    // Прочие данные и методы, необходимые для реализации элемента <text>
private: 
    void RenderObject(const RenderContext& context) const override;

    Point pos_x_y_;
    Point offset_dx_dy_;
    uint32_t font_size_ = 1;
    std::string font_weight_;
    std::string font_family_;
    std::string data_;

};

class ObjectContainer {
public:
    template <typename Obj>
    void Add(Obj obj) {
        //objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
        AddPtr(std::make_unique<Obj>(obj));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
protected:
     ~ObjectContainer() = default;

//private:
    //std::vector<std::unique_ptr<Object>> objects_;
};  

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};



class Document : public ObjectContainer {
public:
    /*
     Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
     Пример использования:
     Document doc;
     doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
    */
    Document() {}


    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document
private:
    std::vector<std::unique_ptr<Object>> objects_;
};



}  // namespace svg