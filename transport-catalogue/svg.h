#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <optional>

namespace svg {

struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r), green(g), blue(b) {
    }

    uint8_t red = 0, green = 0, blue = 0;
};
struct Rgba {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
        : red(r), green(g), blue(b), opacity(a) {
    }

    uint8_t red = 0, green = 0, blue = 0;
    double opacity = 1;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

struct ColorPrinter {
    std::ostream& out;
    void operator()(std::monostate) const;
    void operator()(const std::string& color) const;
    void operator()(const Rgb& color) const;
    void operator()(const Rgba& color) const;
};

std::ostream& operator<<(std::ostream& out, const Color& color);

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& color);

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& color);

struct Point {
    Point() = default;
    Point(double x_, double y_)
        : x(x_)
        , y(y_) {
    }
    double x = 0;
    double y = 0;
};

struct Offset {
    Offset() = default;
    Offset(double x_, double y_)
        : dx(x_)
        , dy(y_) {
    }
    double dx = 0;
    double dy = 0;
};

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

class Object;

class ObjectContainer {
public:
    virtual ~ObjectContainer() = default;

    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }

    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

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

    Owner& SetStrokeLineCap(StrokeLineCap cap) {
        stroke_line_cap_ = cap;
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin join) {
        stroke_line_join_ = join;
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
        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }
        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
    std::optional<double> stroke_width_;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle() = default;

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline() = default;

    Polyline& AddPoint(Point point);

    void RenderObject(const RenderContext& context) const override;

private:
    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text() = default;

    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);

    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);

    Text& SetData(std::string data);

    void RenderObject(const RenderContext& context) const override;

private:
    std::string data_;
    Point pos_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
};

class Document : public ObjectContainer {
public:
    Document() = default;

    void AddPtr(std::unique_ptr<Object>&& obj) override;

    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

}  // namespace svg