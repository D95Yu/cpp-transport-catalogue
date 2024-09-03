#pragma once 

#include "json.h"

#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <variant>

namespace json {

    class Builder;
    class DictItemContext;
    class ArrayItemContext;
    class KeyContext;
    

    class DictItemContext {
    public: 
        explicit DictItemContext(Builder& builder);
        KeyContext Key(std::string key);
        Builder& EndDict();
    private: 
    Builder& builder_;
    };

    class ArrayItemContext {
    public: 
        explicit ArrayItemContext(Builder& builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        ArrayItemContext Value(Node::Value value);
        Builder& EndArray();
    private: 
    Builder& builder_;
    };

    class KeyContext {
    public: 
        explicit KeyContext(Builder& builder);
        DictItemContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
    private: 
    Builder& builder_;
    };

    class Builder {
    public:
        Builder();
        KeyContext Key(std::string key);
        Builder& Value(Node::Value value);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();
        Node Build(); 
    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        std::optional<std::string> key_{std::nullopt};

        Node GetNode(Node::Value value);
    };
}