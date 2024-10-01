#include "json_builder.h"

using namespace std::literals;

namespace json {

    Builder::Builder() {
        Node* root_ptr = &root_;
        nodes_stack_.emplace_back(root_ptr);
    }

    Builder::KeyContext Builder::Key(std::string key) {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsDict() && !key_) {
            key_ = std::move(key);
        }else {
            throw std::logic_error("Key() invalid error"s);
        }
        return KeyContext(*this);
    }

    Node Builder::GetNode(Node::Value value) {
        return Node(std::move(value));
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
            dict.emplace(std::move(*key_), Node(std::move(value)));
            //auto [it, _] = dict.emplace(std::move(*key_), Node(std::move(value)));
            key_ = std::nullopt;
            //top_node = &it->second;
            //top_node->GetValue() = std::move(value);
        }else if (top_node->IsArray()) {
            auto& array = std::get<Array>(top_node->GetValue());
            array.emplace_back(GetNode(std::move(value)));
            top_node = &array.back();
        } else {
            throw std::logic_error("Value() invalid error"s);
        }
        return *this;
    }

    void Builder::AddContainer(const Node::Value& container) {
        auto* top_node = nodes_stack_.back();
        if (top_node->IsNull()) {
            top_node->GetValue() = std::move(container);
        }else if (top_node->IsDict()) {
            if (!key_) {
                throw std::logic_error("AddContainer()-IsDict() invalid error"s);
            }
            auto& dict = std::get<Dict>(top_node->GetValue());
            auto [it, _] = dict.emplace(*key_, Node(container));
            key_ = std::nullopt;
            nodes_stack_.emplace_back(&it->second);
        }else if (top_node->IsArray()) {
            auto& array = std::get<Array>(top_node->GetValue());
            array.emplace_back(Node(container));
            nodes_stack_.emplace_back(&array.back());
        }else {
            throw std::logic_error("AddContainer()-IsArray() invalid error"s);
        }
    }

    Builder::DictItemContext Builder::StartDict() {
        AddContainer(Dict());
        return DictItemContext(*this);
    }

    Builder::ArrayItemContext Builder::StartArray() {
        AddContainer(Array());
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

    Builder::ArrayItemContext::ArrayItemContext(Builder& builder) 
        : builder_(builder) {}

    Builder::DictItemContext::DictItemContext(Builder& builder) 
        : builder_(builder) {}

    Builder::KeyContext::KeyContext(Builder& builder) 
        : builder_(builder) {}

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        return ArrayItemContext(builder_.Value(std::move(value)));
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ArrayItemContext::EndArray() {
        return builder_.EndArray();
    }

    Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& Builder::DictItemContext::EndDict() {
            return builder_.EndDict();
    }

    Builder::KeyContext Builder::DictItemContext::Key(std::string key) {
            return builder_.Key(std::move(key));
    }

    Builder::ArrayItemContext Builder::KeyContext::StartArray() {
        return builder_.StartArray();
    }

    Builder::DictItemContext Builder::KeyContext::StartDict() {   
        return builder_.StartDict();
    }

    Builder::DictItemContext Builder::KeyContext::Value(Node::Value value) {
        return DictItemContext(builder_.Value(std::move(value)));
    }
}