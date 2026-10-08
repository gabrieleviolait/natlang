// NatLang runtime v0.1 — embedded in generated C++ translation units.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
namespace nat {
struct Value {
    using List = std::vector<Value>;
    std::variant<std::monostate, double, std::string, bool, List> data;
    Value() = default;
    Value(double x) : data(x) {}
    Value(int x) : data(static_cast<double>(x)) {}
    Value(bool x) : data(x) {}
    Value(const char *s) : data(std::string(s)) {}
    Value(std::string s) : data(std::move(s)) {}
    Value(List xs) : data(std::move(xs)) {}
};
inline std::string text(const Value &v);
inline const Value &get(const Value &v, const char *name) {
    if (std::holds_alternative<std::monostate>(v.data))
        throw std::runtime_error(std::string("Variable used before assignment: ") + name);
    return v;
}
inline double number(const Value &v) {
    if (auto p = std::get_if<double>(&v.data)) return *p;
    throw std::runtime_error("Expected a number");
}
inline bool truth(const Value &v) {
    if (std::holds_alternative<std::monostate>(v.data)) return false;
    if (auto p = std::get_if<bool>(&v.data)) return *p;
    if (auto p = std::get_if<double>(&v.data)) return *p != 0.0;
    if (auto p = std::get_if<std::string>(&v.data)) return !p->empty();
    return !std::get<Value::List>(v.data).empty();
}
inline std::string text(const Value &v) {
    if (std::holds_alternative<std::monostate>(v.data)) return "null";
    if (auto p = std::get_if<bool>(&v.data)) return *p ? "true" : "false";
    if (auto p = std::get_if<double>(&v.data)) {
        std::ostringstream os; os << std::setprecision(15) << *p; return os.str();
    }
    if (auto p = std::get_if<std::string>(&v.data)) return *p;
    std::string s = "[";
    for (const auto &x : std::get<Value::List>(v.data)) {
        if (s.size() > 1) s += ", ";
        s += text(x);
    }
    return s + "]";
}
inline void print(const Value &v) { std::cout << text(v) << '\n'; }
inline Value add(const Value &a, const Value &b) {
    if (std::holds_alternative<std::string>(a.data) || std::holds_alternative<std::string>(b.data)) return text(a) + text(b);
    return number(a) + number(b);
}
inline Value sub(const Value &a, const Value &b) { return number(a) - number(b); }
inline Value mul(const Value &a, const Value &b) { return number(a) * number(b); }
inline Value div(const Value &a, const Value &b) {
    double n = number(b); if (n == 0) throw std::runtime_error("Division by zero");
    return number(a) / n;
}
inline Value mod(const Value &a, const Value &b) {
    double n = number(b); if (n == 0) throw std::runtime_error("Modulo by zero");
    return std::fmod(number(a), n);
}
inline bool eq(const Value &a, const Value &b) {
    if (a.data.index() != b.data.index()) return false;
    if (auto p = std::get_if<double>(&a.data)) return *p == std::get<double>(b.data);
    if (auto p = std::get_if<std::string>(&a.data)) return *p == std::get<std::string>(b.data);
    if (auto p = std::get_if<bool>(&a.data)) return *p == std::get<bool>(b.data);
    if (auto p = std::get_if<Value::List>(&a.data)) {
        const auto &q = std::get<Value::List>(b.data);
        if (p->size() != q.size()) return false;
        for (size_t i=0;i<p->size();++i) if (!eq((*p)[i],q[i])) return false;
        return true;
    }
    return true;
}
inline int cmp(const Value &a, const Value &b) {
    if (auto p = std::get_if<double>(&a.data)) {
        double x=*p, y=number(b);
        if (std::isnan(x)||std::isnan(y)) throw std::runtime_error("Cannot compare NaN");
        return (x>y)-(x<y);
    }
    if (auto p = std::get_if<std::string>(&a.data)) {
        if (!std::holds_alternative<std::string>(b.data)) throw std::runtime_error("Comparison requires same types");
        return (*p > std::get<std::string>(b.data)) - (*p < std::get<std::string>(b.data));
    }
    throw std::runtime_error("Only numbers and strings can be ordered");
}
inline void append(Value &v, Value x) {
    auto p = std::get_if<Value::List>(&v.data);
    if (!p) throw std::runtime_error("append needs a list");
    p->push_back(std::move(x));
}
inline Value length(const Value &v) {
    if (auto p=std::get_if<Value::List>(&v.data)) return static_cast<double>(p->size());
    if (auto p=std::get_if<std::string>(&v.data)) return static_cast<double>(p->size());
    throw std::runtime_error("length needs a string or list");
}
inline const Value::List &list(const Value &v) {
    auto p=std::get_if<Value::List>(&v.data);
    if (!p) throw std::runtime_error("Expected a list");
    return *p;
}
inline Value sum(const Value &v) {
    double s=0; for (const auto &x : list(v)) s+=number(x); return s;
}
inline Value average(const Value &v) {
    const auto &a=list(v); if (a.empty()) throw std::runtime_error("Cannot average an empty list");
    return number(sum(v))/a.size();
}
inline Value maximum(const Value &v) {
    const auto &a=list(v); if (a.empty()) throw std::runtime_error("Cannot take max of an empty list");
    auto it=std::max_element(a.begin(),a.end(),[](const Value &x,const Value &y){return cmp(x,y)<0;});
    return *it;
}
inline Value minimum(const Value &v) {
    const auto &a=list(v); if (a.empty()) throw std::runtime_error("Cannot take min of an empty list");
    auto it=std::min_element(a.begin(),a.end(),[](const Value &x,const Value &y){return cmp(x,y)<0;});
    return *it;
}
inline Value read_text() {
    std::string s; if (!std::getline(std::cin,s)) throw std::runtime_error("End of input");
    return s;
}
inline Value read_number() {
    const auto s=std::get<std::string>(read_text().data);
    size_t pos=0;
    try {
        double d=std::stod(s,&pos);
        if (pos!=s.size()||!std::isfinite(d)) throw std::runtime_error("Invalid number: " + s);
        return d;
    } catch (const std::invalid_argument &) { throw std::runtime_error("Invalid number: " + s); }
      catch (const std::out_of_range &) { throw std::runtime_error("Number out of range: " + s); }
}
inline long long repeat_count(const Value &v) {
    double n=number(v);
    if (!std::isfinite(n)||n<0||std::floor(n)!=n||n>100000000) throw std::runtime_error("Invalid repetition count");
    return static_cast<long long>(n);
}
inline Value read_numbers(const Value &v) {
    long long n=repeat_count(v);
    if (n>100000) throw std::runtime_error("Too many input numbers");
    Value::List a; a.reserve(static_cast<size_t>(n));
    for (long long i=0;i<n;++i) { std::cout << "Number " << (i+1) << ": " << std::flush; a.push_back(read_number()); }
    return a;
}
inline Value read_file(const Value &path) {
    std::ifstream in(text(path),std::ios::binary);
    if (!in) throw std::runtime_error("Cannot read file: " + text(path));
    std::ostringstream os; os<<in.rdbuf(); return os.str();
}
inline void write_file(const Value &path,const Value &content) {
    std::ofstream out(text(path),std::ios::binary|std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot write file: " + text(path));
    out<<text(content); if (!out) throw std::runtime_error("Write failed: " + text(path));
}
} // namespace nat
