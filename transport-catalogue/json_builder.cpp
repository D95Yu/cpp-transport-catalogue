#include "json_builder.h"

using namespace std::literals;

namespace json {

    Builder::Builder() {
        Node* root_ptr = &root_;
        nodes_stack_.emplace_back(root_ptr);
    }

    KeyContext Builder::Key(std::string key) {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsDict() && !key_) {
            key_ = std::move(key);
        }else {
            throw std::logic_error("Key() invalid error"s);
        }
        return KeyContext(*this);
    }

    Node Builder::GetNode(Node::Value value) {
        if (std::holds_alternative<std::nullptr_t>(value)) {
            return Node(std::get<std::nullptr_t>(value));
        }
        if (std::holds_alternative<Array>(value)) {
            return Node(std::get<Array>(value));
        }
        if (std::holds_alternative<Dict>(value)) {
            return Node(std::get<Dict>(value));
        }
        if (std::holds_alternative<bool>(value)) {
            return Node(std::get<bool>(value));
        }
        if (std::holds_alternative<int>(value)) {
            return Node(std::get<int>(value));
        }
        if (std::holds_alternative<double>(value)) {
            return Node(std::get<double>(value));
        }
        if (std::holds_alternative<std::string>(value)) {
            return Node(std::get<std::string>(value));
        }
        return {};
    }

    Builder& Builder::Value(Node::Value value) {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsNull()) {
            root_.GetValue() = std::move(value);
        }else if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("Value()-IsDict() invalid error"s);
            }
            auto& dict = std::get<Dict>(top_node->GetValue());
            auto [it, _] = dict.emplace(std::move(*key_), Node{});
            key_ = std::nullopt;
            top_node = &it->second;
            top_node->GetValue() = std::move(value);
        }else if (top_node->IsArray()) {
            auto& array = std::get<Array>(top_node->GetValue());
            array.emplace_back(GetNode(value));
            top_node = &array.back();
        } else {
            throw std::logic_error("Value() invalid error"s);
        }
        return *this;
    }

    DictItemContext Builder::StartDict() {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsNull()) {
            top_node->GetValue() = Dict();
        }else if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("StartDict()-IsDict() invalid error"s);
            }
            auto& dict = std::get<Dict>(top_node->GetValue());
            auto [it, _] = dict.emplace(std::move(*key_), Dict());   //почему не Node(Dict())?
            key_ = std::nullopt;
            nodes_stack_.emplace_back(&it->second);
        }else if (top_node->IsArray()) {
            auto& array = std::get<Array>(top_node->GetValue());
            array.emplace_back(Dict());
            nodes_stack_.emplace_back(&array.back());
        }else {
            throw std::logic_error("StartDict() invalid error"s);
        }
        return DictItemContext(*this);
    }

    ArrayItemContext Builder::StartArray() {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsNull()) {
            root_.GetValue() = Array();
        }else if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("StartArray()-IsDict() invalid error"s);
            }
            auto& dict = std::get<Dict>(top_node->GetValue());
            auto [it, _] = dict.emplace(std::move(*key_), Array());
            key_ = std::nullopt;
            nodes_stack_.emplace_back(&it->second);
        }else if (top_node->IsArray()) {
            auto& array = std::get<Array>(top_node->GetValue());
            array.emplace_back(Array());
            nodes_stack_.emplace_back(&array.back());
        }else {
            throw std::logic_error("StartArray() invalid error"s);
        }
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndDict() {
        auto* top_node = nodes_stack_.back();
        if (!top_node->IsDict()) {
            throw std::logic_error("EndDict() invalid error"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Builder& Builder::EndArray() {
        auto* top_node = nodes_stack_.back();
        if (!top_node->IsArray()) {
            throw std::logic_error("EndArray() invalid error"s);
        }
        nodes_stack_.pop_back();
        return *this;
    }

    Node Builder::Build() {
        if (root_.IsNull() || nodes_stack_.size() > 1) {
            throw std::logic_error("Build() invalid error"s);
        }
        return root_;
    }

    ArrayItemContext KeyContext::StartArray() {
        return builder_.StartArray();
    }

    DictItemContext KeyContext::StartDict() {
        return builder_.StartDict();
    }

    DictItemContext KeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(std::move(value)));
    }

    Builder& ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    ArrayItemContext ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(std::move(value)));
    }

    ArrayItemContext ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    DictItemContext ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& DictItemContext::EndDict() {
        return builder_.EndDict();
    }

    KeyContext DictItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    DictItemContext::DictItemContext(Builder& builder) 
        : builder_(builder) {}

    KeyContext::KeyContext(Builder& builder) 
        : builder_(builder) {}

    ArrayItemContext::ArrayItemContext(Builder& builder) 
        : builder_(builder) {}

    


}