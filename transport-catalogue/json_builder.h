#pragma once
#include "json.h"

namespace json {

class DictItemContext;
class ArrayItemContext;
class KeyValueContext;

class Builder {
public:
    Builder();
    Builder(Builder&& other) noexcept;
    Builder(const Builder& other) noexcept;

    Builder& operator=(Builder&& other) noexcept;

    Node Build();

    DictItemContext StartDict();
    ArrayItemContext StartArray();
    KeyValueContext Key(std::string key);
    Builder& Value(Node::Value value);
    Builder& EndDict();
    Builder& EndArray();

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    void AssertNewObjectContext() const;
    void AddObject(Node::Value value, bool one_shot);
};

class BaseContext {
protected:
    Builder& builder_;
public:
    explicit BaseContext(Builder& builder);
};

class DictItemContext;
class ArrayItemContext;

class KeyValueContext : public BaseContext {
public:
    using BaseContext::BaseContext;

    DictItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
};

class DictItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;

    KeyValueContext Key(std::string key);
    Builder& EndDict();
};

class ArrayItemContext : public BaseContext {
public:
    using BaseContext::BaseContext;

    ArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();
};

}  // namespace json
