#include "svg.h"

namespace svg {

using namespace std::literals;

void ColorPrinter::operator()(std::monostate) const {
    out << "none";
}
void ColorPrinter::operator()(const std::string& color) const {
    out << color;
}
void ColorPrinter::operator()(const Rgb& color) const {
    out << "rgb("
        << static_cast<int>(color.red) << ","
        << static_cast<int>(color.green) << ","
        << static_cast<int>(color.blue) << ")";
}
void ColorPrinter::operator()(const Rgba& color) const {
    out << "rgba("
        << static_cast<int>(color.red) << ","
        << static_cast<int>(color.green) << ","
        << static_cast<int>(color.blue) << ","
        << color.opacity << ")";
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(ColorPrinter{out}, color);
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& cap) {
    switch (cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& join) {
    switch (join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
    }
    return out;
}

void ObjectContainer::AddPtr(std::unique_ptr<Object>&& object) {
    objects_.push_back(std::move(object));
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
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point)  {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool is_first = true;
    for (auto& point : points_) {
        if (!is_first) {
            out << " ";
        }
        out << point.x << ","sv << point.y;
        is_first = false;
    }
    out << "\"";

    RenderAttrs(out);

    out << " />"sv;
}

// ---------- Text --------------------

std::string escapeForSVG(const std::string& input) {
    std::string output;
    for (const char& c : input) {
        switch (c) {
            case '"':
                output += "&quot;";
                break;
            case '\'':
                output += "&apos;";
                break;
            case '<':
                output += "&lt;";
                break;
            case '>':
                output += "&gt;";
                break;
            case '&':
                output += "&amp;";
                break;
            default:
                output += c;
        }
    }
    return output;
}

Text& Text::SetData(std::string text)  {
    data_ = std::move(escapeForSVG(text));
    return *this;
}

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    
    out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" ";
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" ";
    out << "font-size=\""sv << font_size_ << "\" ";
    if (!font_family_.empty())
        out << "font-family=\""sv << font_family_ << "\" ";
    if (!font_weight_.empty())
        out << "font-weight=\""sv << font_weight_ << "\"";
    RenderAttrs(out);
    out << ">" << data_ << "</text>"sv;
}

// ---------- Document ----------------

void Document::AddPtr(std::unique_ptr<Object>&& object) {
    objects_.push_back(std::move(object));
}

void Document::Render(std::ostream& out) const {
    RenderContext context(out, 0, 1);
    
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for (auto& object : objects_) {
        context.RenderIndent();
        object->Render(context);
    }
    out << "</svg>"sv;
}

}  // namespace svg