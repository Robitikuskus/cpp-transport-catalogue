#include "json_builder.h"
#include <exception>
#include <utility>

using namespace std::literals;

namespace json {

DictItemContext KeyValueContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return DictItemContext(builder_);
}

DictItemContext KeyValueContext::StartDict() {
    builder_.StartDict();
    return DictItemContext(builder_);
}

ArrayItemContext KeyValueContext::StartArray() {
    builder_.StartArray();
    return ArrayItemContext(builder_);
}

KeyValueContext DictItemContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& DictItemContext::EndDict() {
    builder_.EndDict();
    return builder_;
}

ArrayItemContext ArrayItemContext::Value(Node::Value value) {
    builder_.Value(std::move(value));
    return *this;
}

DictItemContext ArrayItemContext::StartDict() {
    builder_.StartDict();
    return DictItemContext(builder_);
}

ArrayItemContext ArrayItemContext::StartArray() {
    builder_.StartArray();
    return *this;
}

Builder& ArrayItemContext::EndArray() {
    builder_.EndArray();
    return builder_;
}

Builder::Builder()
    : root_()
    , nodes_stack_{&root_} {
}

Builder& Builder::operator=(Builder&& other) noexcept {
    if (this != &other) {
        root_ = std::move(other.root_);
        nodes_stack_ = std::move(other.nodes_stack_);

        other.nodes_stack_.clear();
    }
    return *this;
}

Builder::Builder(Builder&& other) noexcept
    : root_(std::move(other.root_))
    , nodes_stack_(std::move(other.nodes_stack_)) {
        other.nodes_stack_.clear();
}

Builder::Builder(const Builder& other) noexcept
    : root_(other.root_)
    , nodes_stack_(other.nodes_stack_) {
}

Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("Attempt to build JSON which isn't finalized"s);
    }
    return std::move(root_);
}

DictItemContext Builder::StartDict() {
    AddObject(Dict{}, false);
    return DictItemContext(*this);
}

ArrayItemContext Builder::StartArray() {
    AddObject(Array{}, false);
    return ArrayItemContext(*this);
}

KeyValueContext Builder::Key(std::string key) {
    Node::Value& host_value = GetCurrentValue();
    if (!std::holds_alternative<Dict>(host_value)) {
        throw std::logic_error("Key() outside a dict"s);
    }
    nodes_stack_.push_back(&std::get<Dict>(host_value)[std::move(key)]);
    return KeyValueContext(*this);
}

Builder& Builder::Value(Node::Value value) {
    AddObject(std::move(value), true);
    return *this;
}

Builder& Builder::EndDict() {
    if (!std::holds_alternative<Dict>(GetCurrentValue())) {
        throw std::logic_error("EndDict() outside a dict"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (!std::holds_alternative<Array>(GetCurrentValue())) {
        throw std::logic_error("EndArray() outside an array"s);
    }
    nodes_stack_.pop_back();
    return *this;
}

Node::Value& Builder::GetCurrentValue() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("Attempt to change finalized JSON"s);
    }
    return nodes_stack_.back()->GetValue();
}

const Node::Value& Builder::GetCurrentValue() const {
    return const_cast<Builder*>(this)->GetCurrentValue();
}

void Builder::AssertNewObjectContext() const {
    if (!std::holds_alternative<std::nullptr_t>(GetCurrentValue())) {
        throw std::logic_error("New object in wrong context"s);
    }
}

void Builder::AddObject(Node::Value value, bool one_shot) {
    Node::Value& host_value = GetCurrentValue();
    if (std::holds_alternative<Array>(host_value)) {
        Node& node = std::get<Array>(host_value).emplace_back(std::move(value));
        if (!one_shot) {
            nodes_stack_.push_back(&node);
        }
    } else {
        AssertNewObjectContext();
        host_value = std::move(value);

        if (one_shot) {
            nodes_stack_.pop_back();
        }
    }
}

BaseContext::BaseContext(Builder& builder)
    : builder_(builder) {
}

}  // namespace json
