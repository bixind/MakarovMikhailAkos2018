#pragma once

#include <sstream>
#include "memory"
#include "math.h"

constexpr double EPS = 1e-9;
bool DoublesEqual(double a, double b) {
    return fabs(a - b) < EPS;
}

class NodeBase;
using NodePtr = std::shared_ptr<NodeBase>;

class NodeBase : std::enable_shared_from_this<NodeBase> {
public:
    virtual ~NodeBase() = default;

    virtual std::string Print() const = 0;

    virtual std::string AsTeX() const = 0;

    virtual std::string Dump() const = 0;

    virtual NodePtr Differentiate(const std::string& variable) const = 0;

    virtual NodePtr Copy() const = 0;

    virtual NodePtr Simplify(const NodePtr& self) {
        return self;
    }
};

template<class T>
std::string PtrToString(const T* ptr) {
    std::stringstream stream;
    stream << ptr;
    return stream.str();
}

NodePtr operator+(const NodePtr& left, const NodePtr& right);
NodePtr operator-(const NodePtr& left, const NodePtr& right);
NodePtr operator-(const NodePtr& val);
NodePtr operator*(const NodePtr& left, const NodePtr& right);
NodePtr operator/(const NodePtr& left, const NodePtr& right);

NodePtr Cos(const NodePtr& val);
NodePtr Sin(const NodePtr& val);
NodePtr Ln(const NodePtr& val);
NodePtr Number(double val);


template <class T>
std::shared_ptr<T> NodeAs(const NodePtr& node) {
    return std::dynamic_pointer_cast<T>(node);
}

class ExpressionTree {
public:
    ExpressionTree(NodePtr root) : root_(root) {
    }

    std::string PrintTree() const {
        return root_->Print();
    }
    std::string AsTeX() const {
        return root_->AsTeX();
    }

    ExpressionTree Differentiate(const std::string& variable) const {
        return ExpressionTree(root_->Differentiate(variable));
    }

    void Simplify() {
        root_ = root_->Simplify(root_);
    }

    std::string Dump() const {
        return "digraph{\n" + root_->Dump() + '}';
    }

private:
    NodePtr root_;
};

class NumberNode : public NodeBase {
public:
    NumberNode(double value) : value_(value) {
    }

    std::string Print() const override {
        return '(' + std::to_string(value_) + ')';
    }

    std::string AsTeX() const override {
        std::stringstream stream;
        stream << value_;
        return stream.str();
    }

    std::string Dump() const override {
        return "\"" + PtrToString(this) +  "\" [label=\"" +
        std::to_string(value_) + "\n"
        "self: " + PtrToString(this) + "\n"
        "\"];\n";
    }

    NodePtr Copy() const override {
        return std::make_shared<NumberNode>(value_);
    }

    bool IsZero() const {
        return DoublesEqual(value_, 0);
    }

    bool IsOne() const {
        return DoublesEqual(value_, 1);
    }

    double Value() const {
        return value_;
    }

    NodePtr Differentiate(const std::string&) const override {
        return Number(0);
    }

private:
    double value_;
};

NodePtr Number(double val) {
    return std::make_shared<NumberNode>(val);
}

class VariableNode : public NodeBase {
public:
    VariableNode(const std::string& name) : name_(name) {
    }

    std::string Print() const override {
        return '(' + name_ + ')';
    }

    std::string AsTeX() const override {
        return name_;
    }

    std::string Dump() const override {
        return "\"" + PtrToString(this) +  "\" [label=\"" +
        name_ + "\n"
        "self: " + PtrToString(this) + "\n" +
        "\"];\n";
    }

    NodePtr Copy() const override {
        return std::make_shared<VariableNode>(name_);
    }

    NodePtr Differentiate(const std::string& variable) const override {
        return Number(variable == name_ ? 1 : 0);
    }

private:
    std::string name_;
};

class OperatorNode : public NodeBase {
public:
    explicit OperatorNode(const NodePtr& first = nullptr, const NodePtr& second = nullptr)
        : first_(first), second_(second) {
    }

    void SetFirst(const NodePtr& first) {
        first_ = first;
    }

    void SetSecond(const NodePtr& second) {
        second_ = second;
    }

    std::string Print() const override {
        return '(' + first_->Print() + ' ' + OperatorName() + ' ' + second_->Print() + ')';
    }

    std::string Dump() const override {
        auto res = "\"" + PtrToString(this) +  "\" [label=\""
            + OperatorName() + "\n"
            "self: " + PtrToString(this) + "\n"
            "first: " + PtrToString(this->first_.get()) + "\n"
            "second: " + PtrToString(this->second_.get()) + "\n"
            "\"];\n";
        if (first_) {
            res += '"' + PtrToString(this) + "\"->\"" + PtrToString(this->first_.get()) + "\"\n";
            res += first_->Dump();
        }
        if (second_) {
            res += '"' + PtrToString(this) + "\"->\"" + PtrToString(this->second_.get()) + "\"\n";
            res += second_->Dump();
        }
        return res;
    }

    NodePtr Simplify(const NodePtr& self) override {
        first_ = first_->Simplify(first_);
        second_ = second_->Simplify(second_);
        return SimplifyOperator(self);
    }

protected:
    virtual std::string OperatorName() const = 0;
    virtual NodePtr SimplifyOperator(const NodePtr& self) {
        return self;
    }

    template <class DerivedNode>
    NodePtr CopyChildren() const {
        auto root = std::make_shared<DerivedNode>();
        root->SetFirst(first_->Copy());
        root->SetSecond(second_->Copy());
        return root;
    }

    NodePtr first_;
    NodePtr second_;
};

using OperatorNodePtr = std::shared_ptr<OperatorNode>;

#define D1 first_->Differentiate(variable)
#define D2 second_->Differentiate(variable)

#define C1 first_->Copy()
#define C2 second_->Copy()
#define CA arg_->Copy()


class SumNode : public OperatorNode {
public:
    using OperatorNode::OperatorNode;

    std::string OperatorName() const override {
        return "+";
    }

    NodePtr Copy() const override {
        return CopyChildren<SumNode>();
    }

    NodePtr SimplifyOperator(const NodePtr& self) override {
        auto left = NodeAs<NumberNode>(first_);
        auto right = NodeAs<NumberNode>(second_);
        if (left && right) {
            return Number(left->Value() + right->Value());
        }
        if (left && left->IsZero()) {
            return second_;
        }
        if (right && right->IsZero()) {
            return first_;
        }

        return self;
    }

    NodePtr Differentiate(const std::string& variable) const override {
        return D1 + D2;
    }

    std::string AsTeX() const override {
        return '(' + first_->AsTeX() + " + " + second_->AsTeX() + ')';
    }


};

class DiffNode : public OperatorNode {
public:
    using OperatorNode::OperatorNode;

    std::string OperatorName() const override {
        return "-";
    }

    NodePtr Copy() const override {
        return CopyChildren<DiffNode>();
    }

    NodePtr SimplifyOperator(const NodePtr& self) override;

    NodePtr Differentiate(const std::string& variable) const override {
        return D1 - D2;
    }

    std::string AsTeX() const override {
        return '(' + first_->AsTeX() + " - " + second_->AsTeX() + ')';
    }
};

class MulNode : public OperatorNode {
public:
    using OperatorNode::OperatorNode;

    std::string OperatorName() const override {
        return "*";
    }

    NodePtr Copy() const override {
        return CopyChildren<MulNode>();
    }

    NodePtr Differentiate(const std::string& variable) const override {
        return (D1 * C2) + (C1 * D2);
    }

    NodePtr SimplifyOperator(const NodePtr& self) override {
        auto left = NodeAs<NumberNode>(first_);
        auto right = NodeAs<NumberNode>(second_);
        if (left && right) {
            return Number(left->Value() * right->Value());
        }
        if (left) {
            if (left->IsZero()) {
                return left;
            }
            if (left->IsOne()) {
                return second_;
            }
        }
        if (right) {
            if (right->IsZero()) {
                return right;
            }
            if (right->IsOne()) {
                return first_;
            }
        }
        return self;
    }

    std::string AsTeX() const override {
        return first_->AsTeX() + " \\cdot " + second_->AsTeX();
    }
};

class DivNode : public OperatorNode {
public:
    using OperatorNode::OperatorNode;

    NodePtr Copy() const override {
        return CopyChildren<DivNode>();
    }

    NodePtr Differentiate(const std::string& variable) const override {
        return (D1 * C2 - C1 * D2) / (C2 * C2);
    }

    NodePtr SimplifyOperator(const NodePtr& self) override {
        auto left = NodeAs<NumberNode>(first_);
        auto right = NodeAs<NumberNode>(second_);
        if (left && right) {
            return Number(left->Value() / right->Value());
        }
        if (left && left->IsZero()) {
                return left;
        }
        if (right && right->IsOne()) {
                return first_;
        }
        return self;
    }

    std::string OperatorName() const override {
        return "/";
    }

    std::string AsTeX() const override {
        return "\\frac{" + first_->AsTeX() + "}{" + second_->AsTeX() + '}';
    }
};

class FunctionNode;
using FunctionNodePtr = std::shared_ptr<FunctionNode>;

class FunctionNode : public NodeBase {
public:
    FunctionNode(const NodePtr& arg = nullptr) : arg_(arg) {
    }

    void SetArg(const NodePtr& arg) {
        arg_ = arg;
    }

    std::string Print() const override {
        return '(' + FuncName() + arg_->Print() + ')';
    }

    NodePtr Differentiate(const std::string& variable) const override {
        auto derivative = FunctionDerivative();
        auto chain = arg_->Differentiate(variable);

        return derivative * chain;
    }

    NodePtr Simplify(const NodePtr& self) override {
        arg_ = arg_->Simplify(arg_);
        if (auto number = NodeAs<NumberNode>(arg_)) {
            return Number(CalcFunc(number->Value()));
        }
        return self;
    }

    std::string Dump() const override {
        auto res = "\"" + PtrToString(this) +  "\" [label=\""
        + FuncName() + "\n"
        "self: " + PtrToString(this) + "\n"
        "arg: " + PtrToString(this->arg_.get()) + "\n"
        "\"];\n";
        if (arg_) {
            res += '"' + PtrToString(this) + "\"->\"" + PtrToString(this->arg_.get()) + "\"\n";
            res += arg_->Dump();
        }
        return res;
    }

protected:
    virtual std::string FuncName() const = 0;
    virtual NodePtr FunctionDerivative() const = 0;
    virtual double CalcFunc(double value) const = 0;

    template <class Derived>
    NodePtr CopyArg() const {
        auto root = std::make_shared<Derived>();
        root->SetArg(arg_->Copy());
        return root;
    }

    NodePtr arg_;
};

class UnaryMinusNode : public FunctionNode {
public:
    using FunctionNode::FunctionNode;

    NodePtr Copy() const override {
        return CopyArg<UnaryMinusNode>();
    }

    NodePtr FunctionDerivative() const override {
        return Number(-1);
    }

    std::string FuncName() const override {
        return "minus";
    };

    virtual double CalcFunc(double value) const override {
        return -value;
    }

    std::string AsTeX() const override {
        return '-' + arg_->AsTeX();
    }
};

NodePtr DiffNode::SimplifyOperator(const NodePtr& self) {
    auto left = NodeAs<NumberNode>(first_);
    auto right = NodeAs<NumberNode>(second_);
    if (left && right) {
        return Number(left->Value() - right->Value());
    }
    if (left && left->IsZero()) {
        return -second_;
    }
    if (right && right->IsZero()) {
        return first_;
    }

    return self;
}

class SinNode : public FunctionNode {
public:
    using FunctionNode::FunctionNode;

    NodePtr Copy() const override {
        return CopyArg<SinNode>();
    }

    NodePtr FunctionDerivative() const override;

    std::string FuncName() const override {
        return "sin";
    };

    virtual double CalcFunc(double value) const override {
        return sin(value);
    }

    std::string AsTeX() const override {
        return "\\sin{" + arg_->AsTeX() + "}";
    }

};

class CosNode : public FunctionNode {
public:
    using FunctionNode::FunctionNode;

    NodePtr Copy() const override {
        return CopyArg<CosNode>();
    }

    NodePtr FunctionDerivative() const override;

    std::string FuncName() const override {
        return "cos";
    };

    virtual double CalcFunc(double value) const override {
        return cos(value);
    }

    std::string AsTeX() const override {
        return "\\cos{" + arg_->AsTeX() + "}";
    }
};

NodePtr SinNode::FunctionDerivative() const {
    return Cos(CA);
}

NodePtr CosNode::FunctionDerivative() const {
    return -Sin(CA);
}

class LnNode : public FunctionNode {
public:
    using FunctionNode::FunctionNode;

    NodePtr Copy() const override {
        return CopyArg<LnNode>();
    }

    NodePtr FunctionDerivative() const override {
        return Number(1) / CA;
    }

    std::string FuncName() const override {
        return "ln";
    };

    virtual double CalcFunc(double value) const override {
        return cos(value);
    }

    std::string AsTeX() const override {
        return "\\ln{" + arg_->AsTeX() + "}";
    }
};

class TanNode : public FunctionNode {
public:
    using FunctionNode::FunctionNode;

    NodePtr Copy() const override {
        return CopyArg<TanNode>();
    }

    NodePtr FunctionDerivative() const override {
        return Number(1) / (Cos(CA) * Cos(CA));
    }

    std::string FuncName() const override {
        return "tan";
    };

    virtual double CalcFunc(double value) const override {
        return tan(value);
    }

    std::string AsTeX() const override {
        return "\\tan{" + arg_->AsTeX() + "}";
    }
};

NodePtr Tan(const NodePtr& val) {
    return std::make_shared<TanNode>(val);
}

class PowerNode : public OperatorNode {
public:
    using OperatorNode::OperatorNode;

    NodePtr Copy() const override {
        return CopyChildren<PowerNode>();
    }

    NodePtr Differentiate(const std::string& variable) const override {
        return Copy() * (C2 / C1 * D1 + D2 * Ln(C1));
    }

    NodePtr SimplifyOperator(const NodePtr& self) override {
        auto left = NodeAs<NumberNode>(first_);
        auto right = NodeAs<NumberNode>(second_);
        if (left && right) {
            return Number(pow(left->Value(), right->Value()));
        }
        if (left && (left->IsZero() || left->IsOne())) {
            return left;
        }
        if (right) {
            if (right->IsZero()) {
                return Number(1);
            }
            if (right->IsOne()) {
                return first_;
            }
        }
        return self;
    }

    std::string OperatorName() const override {
        return "^";
    }

    std::string AsTeX() const override {
        return "{" + first_->AsTeX() + "} ^ {" + second_->AsTeX() + '}';
    }
};

NodePtr operator+(const NodePtr& left, const NodePtr& right) {
    return std::make_shared<SumNode>(left, right);
}

NodePtr operator-(const NodePtr& left, const NodePtr& right) {
    return std::make_shared<DiffNode>(left, right);
}

NodePtr operator-(const NodePtr& val) {
    return std::make_shared<UnaryMinusNode>(val);
}

NodePtr operator*(const NodePtr& left, const NodePtr& right) {
    return std::make_shared<MulNode>(left, right);
}

NodePtr operator/(const NodePtr& left, const NodePtr& right) {
    return std::make_shared<DivNode>(left, right);
}

NodePtr Cos(const NodePtr& val) {
    return std::make_shared<CosNode>(val);
}

NodePtr Sin(const NodePtr& val) {
    return std::make_shared<SinNode>(val);
}

NodePtr Ln(const NodePtr& val) {
    return std::make_shared<LnNode>(val);
}

class StringReader {
public:
    explicit StringReader(const std::string& view) : pos_(0), text_(view) {
        if (!view.empty() && isalnum(view.front())) {
            Next();
        }
    }

    void Next() {
        CheckValid();
        ++pos_;
        while (pos_ < text_.size() && isspace(text_[pos_])) {
            ++pos_;
        }
    }

    double ReadDouble() {
        return std::stod(ReadString(')'));
    }

    std::string ReadString(char stop) {
        size_t end = text_.find(stop, pos_);
        auto res = text_.substr(pos_, end - pos_);
        pos_ = end;
        return res;
    }

    char Value() const {
        CheckValid();
        return text_[pos_];
    }

    void Eat(char c) {
        if (Value() != c) {
            throw std::runtime_error(std::string(c, 1) + " expected");
        }
        Next();
    }

    size_t Find(char c) {
        return text_.find(c, pos_);
    }

private:
    void CheckValid() const {
        if (pos_ >= text_.size()) {
            throw std::runtime_error("end of string reached");
        }
    }

    size_t pos_;
    std::string text_;
};

OperatorNodePtr ParseOperator(StringReader* reader) {
    OperatorNodePtr result;
    if (reader->Value() == '+') {
        result = std::make_shared<SumNode>();
    }
    if (reader->Value() == '-') {
        result = std::make_shared<DiffNode>();
    }
    if (reader->Value() == '*') {
        result = std::make_shared<MulNode>();
    }
    if (reader->Value() == '/') {
        result = std::make_shared<DivNode>();
    }
    if (reader->Value() == '^') {
        result = std::make_shared<PowerNode>();
    }
    if (!result) {
        throw std::runtime_error("Unrecognized operator: " + std::string(reader->Value(), 1));
    }
    reader->Next();
    return result;
}

NodePtr ParseExpression(StringReader* reader);

NodePtr ParseFunction(StringReader* reader) {
    auto name = reader->ReadString('(');
    std::shared_ptr<FunctionNode> func;
    if (name == "sin") {
        func = std::make_shared<SinNode>();
    }
    if (name == "cos") {
        func = std::make_shared<CosNode>();
    }
    if (name == "minus") {
        func = std::make_shared<UnaryMinusNode>();
    }
    if (name == "ln") {
        func = std::make_shared<LnNode>();
    }
    if (name == "tan") {
        func = std::make_shared<TanNode>();
    }
    if (!func) {
        throw std::runtime_error("unknown function: " + name);
    }
    auto node = ParseExpression(reader);
    func->SetArg(node);
    return func;
}

NodePtr ParseVaraible(StringReader* reader) {
    auto name = reader->ReadString(')');
    return std::make_shared<VariableNode>(name);
}

NodePtr ParseExpression(StringReader* reader) {
    reader->Eat('(');
    NodePtr result;
    char value = reader->Value();
    if (value == '(') {
        // operator case
        auto first_child = ParseExpression(reader);
        auto node = ParseOperator(reader);
        auto second_child = ParseExpression(reader);
        node->SetFirst(first_child);
        node->SetSecond(second_child);
        result = node;
    } else {
        if (value == '+' || value == '-' || isdigit(value)) {
            result = Number(reader->ReadDouble());
        } else {
            auto open_pos = reader->Find('(');
            auto close_pos = reader->Find(')');
            if (open_pos == std::string::npos && close_pos == std::string::npos) {
                throw std::runtime_error("eventually expected ( or ) ");
            }

            if (close_pos == std::string::npos || close_pos > open_pos) {
                result = ParseFunction(reader);
            } else {
                result = ParseVaraible(reader);
            }
        }
    }
    reader->Eat(')');
    if (!result) {
        throw std::runtime_error("something went wrong");
    }
    return result;
}

ExpressionTree TreeFromString(const std::string& expression) {
    StringReader reader(expression);
    return ExpressionTree(ParseExpression(&reader));
}
