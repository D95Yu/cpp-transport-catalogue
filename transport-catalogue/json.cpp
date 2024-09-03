#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input) {
        throw ParsingError("can't parse array"s);
    }

    return Node(move(result));
}

Node LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(istream& input) {    
    auto it = istreambuf_iterator<char>(input);
    auto end = istreambuf_iterator<char>();
    string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(s);
}

Node LoadDict(istream& input) {
    Dict result;
 
    for (char ch; input >> ch && ch != '}';) {
        
        if (ch == '"') {
            std::string key = LoadString(input).AsString();
 
            if (input >> ch && ch == ':') {
                
                if (result.find(key) != result.end()) {
                    throw ParsingError("duplicate key '"s + key + "'found");
                }
 
                result.emplace(std::move(key), LoadNode(input));
                
            } else {
                throw ParsingError(": expected. but '"s + ch + "' found"s);
            }
            
        } else if (ch != ',') {
            throw ParsingError("',' expected. but '"s + ch + "' found"s);
        }
    }
 
    if (!input) {
        throw ParsingError("unable to parse dictionary"s);
    }
        return Node(result);
}

Node LoadBool(istream& input) {
    string str;
    while (isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    if (str != "true"sv && str != "false"sv) {
        throw ParsingError(str + "is not bool");
    }else if (str == "false"sv) {
        return Node(false);
    }
    return Node(true);
}

Node LoadNull(istream& input) {
    string str;
    while(isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    if (str != "null"sv) {
        throw ParsingError(str + "is not null");
    }
    return Node(nullptr);
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    }else if (c == '{') {
        return LoadDict(input);
    }else if (c == '"') {
        return LoadString(input);
    }else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }else if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    input.putback(c);
    return LoadNumber(input);
}

}  // namespace

Node::Node(std::nullptr_t) 
    : Node() {}

bool Node::IsInt() const {
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const {
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const {
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const {
    return std::holds_alternative<string>(value_);
}

bool Node::IsNull() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsArray() const {
    return std::holds_alternative<Array>(value_);
}

bool Node::IsDict() const {
    return std::holds_alternative<Dict>(value_);
}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("value is not int");
    }
    return std::get<int>(value_);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("value is not bool");
    }
    return std::get<bool>(value_);
}

double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("value is not double or int");
    }
    if (IsInt()) {
        return static_cast<double>(std::get<int>(value_));
    }
    return std::get<double>(value_);
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("value is not string");
    }
    return std::get<string>(value_);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("value is not array");
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsDict() const {
    if (!IsDict()) {
        throw std::logic_error("value is not map");
    }
    return std::get<Dict>(value_);
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& node, const PrintContext& ctx);

void PrintString(const std::string& value, std::ostream& out) {
    using namespace std::literals;
    out.put('"');
    for (const char c : value) {
        if (c == '\r') {
            out << "\\r"sv;
        }else if (c == '"') {
            out << "\\\""sv;
        }else if (c == '\\') {
            out << "\\\\"sv;
        }else if (c == '\t') {
            out << "\\t"sv;
        }else if (c == '\n') {
            out << "\\n"sv;
        }else {
            out.put(c);
        }
    }
    out.put('"');
}

// Шаблон, подходящий для вывода double и int
template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

// Перегрузка функции PrintValue для вывода значений null
void PrintValue(std::nullptr_t, const PrintContext& ctx) {
    ctx.out << "null"sv;
}
// Другие перегрузки функции PrintValue пишутся аналогично
//template<>
void PrintValue(const std::string& value, const PrintContext& ctx) {
    PrintString(value, ctx.out);
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << boolalpha << value;
}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
    using namespace std::literals;
    ctx.out << "{\n"sv;
    bool is_first = true;
    auto ctx_indent = ctx.Indented();

    for (const auto& [key, value] : dict) {
        if (!is_first) {
            ctx.out << ",\n"sv;
        }else {
            is_first = false;
        }
        ctx_indent.PrintIndent();
        PrintString(key, ctx_indent.out);
        ctx.out << ": "sv;
        PrintNode(value, ctx_indent);
    }

    ctx.out.put('\n');
    ctx.PrintIndent();
    ctx.out.put('}');
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    using namespace std::literals;
    ctx.out << "[\n"sv;
    bool is_first = true;
    auto ctx_indent = ctx.Indented();

    for (const auto& value : array) {
        if (!is_first) {
            ctx.out << ",\n"sv;
        }else {
            is_first = false;
        }
        ctx_indent.PrintIndent();
        PrintNode(value, ctx_indent);
    }

    ctx.out.put('\n');
    ctx.PrintIndent();
    ctx.out.put(']');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){ PrintValue(value, ctx); },
        node.GetValue());
} 

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});

    // Реализуйте функцию самостоятельно
}

}  // namespace json