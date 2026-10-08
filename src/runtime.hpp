// NatLang runtime v0.3 — embedded in generated C++ translation units.
#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <set>
#include <thread>
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
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#if defined(_MSC_VER)
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#endif
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
// Numerical operations share a single language-independent runtime.
inline Value finite_result(double x) {
    if (!std::isfinite(x)) throw std::runtime_error("Non-finite numerical result");
    return x;
}
inline Value power(const Value &a, const Value &b) {
    const double base=number(a), exponent=number(b);
    if (base==0 && exponent<0) throw std::runtime_error("Zero cannot have a negative exponent");
    if (base<0 && std::floor(exponent)!=exponent) throw std::runtime_error("Negative base with fractional exponent");
    return finite_result(std::pow(base,exponent));
}
inline Value square_root(const Value &v) {
    double x=number(v); if (x<0) throw std::runtime_error("Square root of negative number");
    return finite_result(std::sqrt(x));
}
inline Value cube_root(const Value &v) {return finite_result(std::cbrt(number(v)));}
inline Value absolute(const Value &v) {return finite_result(std::fabs(number(v)));}
inline Value round_number(const Value &v) {return finite_result(std::round(number(v)));}
inline Value floor_number(const Value &v) {return finite_result(std::floor(number(v)));}
inline Value ceil_number(const Value &v) {return finite_result(std::ceil(number(v)));}
inline Value natural_log(const Value &v) {
    double x=number(v); if (x<=0) throw std::runtime_error("Logarithm requires a positive argument");
    return finite_result(std::log(x));
}
inline Value decimal_log(const Value &v) {
    double x=number(v); if (x<=0) throw std::runtime_error("Logarithm requires a positive argument");
    return finite_result(std::log10(x));
}
inline Value exponential(const Value &v) {return finite_result(std::exp(number(v)));}
// Trigonometric arguments and results use radians.
inline Value sine(const Value &v) {return finite_result(std::sin(number(v)));}
inline Value cosine(const Value &v) {return finite_result(std::cos(number(v)));}
inline Value tangent(const Value &v) {return finite_result(std::tan(number(v)));}
inline Value arc_sine(const Value &v) {
    double x=number(v);if(x<-1||x>1)throw std::runtime_error("asin domain is [-1, 1]");
    return finite_result(std::asin(x));
}
inline Value arc_cosine(const Value &v) {
    double x=number(v);if(x<-1||x>1)throw std::runtime_error("acos domain is [-1, 1]");
    return finite_result(std::acos(x));
}
inline Value arc_tangent(const Value &v) {return finite_result(std::atan(number(v)));}
inline Value arc_tangent2(const Value &a,const Value &b) {return finite_result(std::atan2(number(a),number(b)));}
inline Value factorial(const Value &v) {
    double x=number(v);
    if (x<0||x>170||std::floor(x)!=x)throw std::runtime_error("Factorial requires an integer from 0 to 170");
    double result=1;for(int i=2;i<=static_cast<int>(x);++i)result*=i;
    return finite_result(result);
}
inline Value percent(const Value &rate,const Value &amount) {return finite_result(number(rate)*number(amount)/100.0);}
inline Value clamp(const Value &v,const Value &lo,const Value &hi) {
    double x=number(v),a=number(lo),b=number(hi);
    if(a>b)throw std::runtime_error("clamp requires min <= max");
    return finite_result(std::clamp(x,a,b));
}
inline Value sign(const Value &v) {double x=number(v);return static_cast<double>((x>0)-(x<0));}
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
inline Value ask_text(const Value &prompt) {
    std::cout << text(prompt) << std::flush;
    return read_text();
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
inline Value ask_number(const Value &prompt) {
    std::cout << text(prompt) << std::flush;
    return read_number();
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

// Conservative IPv4 ICMP reachability tools. No port probing, DNS resolution,
// privileged/raw sockets, persistence or exploit code. ICMP filters can cause false negatives.
inline uint32_t parse_ipv4(const std::string &ip) {
    // Only canonical dotted decimal: this is also the shell-command injection boundary.
    if (ip.empty() || ip.size() > 15) throw std::runtime_error("Invalid IPv4 address: " + ip);
    uint32_t addr=0; size_t from=0;
    for(int part=0;part<4;++part) {
        const size_t next=ip.find('.',from);
        if((part<3 && next==std::string::npos)||(part==3 && next!=std::string::npos))
            throw std::runtime_error("Invalid IPv4 address: " + ip);
        const size_t end=next==std::string::npos?ip.size():next;
        if(end==from || end-from>3 || (end-from>1 && ip[from]=='0'))
            throw std::runtime_error("Invalid IPv4 address: " + ip);
        unsigned octet=0;
        for(size_t k=from;k<end;++k) {
            if(ip[k]<'0'||ip[k]>'9') throw std::runtime_error("Invalid IPv4 address: " + ip);
            octet=octet*10+static_cast<unsigned>(ip[k]-'0');
        }
        if(octet>255) throw std::runtime_error("Invalid IPv4 address: " + ip);
        addr=(addr<<8)|octet;from=end+1;
    }
    return addr;
}
inline std::string ipv4_string(uint32_t ip) {
    return std::to_string((ip>>24)&255)+"."+std::to_string((ip>>16)&255)+"."+
           std::to_string((ip>>8)&255)+"."+std::to_string(ip&255);
}
inline bool private_ipv4(uint32_t ip) {
    return (ip>>24)==10 || (ip>>20)==0xAC1 || (ip>>16)==0xC0A8;
}
inline bool ping_ipv4(uint32_t address) {
    const auto ip=ipv4_string(address); // Produced entirely from validated integer octets.
#ifdef _WIN32
    const std::string command="ping -n 1 -w 900 " + ip + " >NUL 2>&1";
#elif defined(__APPLE__)
    const std::string command="ping -n -c 1 -W 900 " + ip + " >/dev/null 2>&1";
#else
    const std::string command="ping -n -c 1 -W 1 " + ip + " >/dev/null 2>&1";
#endif
    return std::system(command.c_str())==0;
}
inline Value ping(const Value &ip) { return ping_ipv4(parse_ipv4(text(ip))); }
inline void scan_ip(const Value &ip) {
    const auto addr=parse_ipv4(text(ip));
    const bool responding=ping_ipv4(addr);
    std::cout << ipv4_string(addr) << ": " << (responding?"responding":"no ICMP response") << '\n';
}
struct LocalIPv4 { uint32_t address=0,mask=0; };
inline std::vector<LocalIPv4> local_interfaces() {
    std::vector<LocalIPv4> out;
#ifdef _WIN32
    ULONG size=16384;
    std::vector<unsigned char> data(size);
    DWORD status=ERROR_BUFFER_OVERFLOW;
    for(int retry=0;retry<3 && status==ERROR_BUFFER_OVERFLOW;++retry) {
        status=GetAdaptersAddresses(AF_INET,GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
              GAA_FLAG_SKIP_DNS_SERVER,nullptr,
              reinterpret_cast<IP_ADAPTER_ADDRESSES*>(data.data()),&size);
        if(status==ERROR_BUFFER_OVERFLOW) data.resize(size);
    }
    if(status!=NO_ERROR)throw std::runtime_error("Could not inspect local interfaces");
    auto *adapter=reinterpret_cast<IP_ADAPTER_ADDRESSES*>(data.data());
    for(;adapter;adapter=adapter->Next) {
        if(adapter->OperStatus!=IfOperStatusUp || adapter->IfType==IF_TYPE_SOFTWARE_LOOPBACK)continue;
        for(auto *u=adapter->FirstUnicastAddress;u;u=u->Next) {
            if(!u->Address.lpSockaddr || u->Address.lpSockaddr->sa_family!=AF_INET || u->OnLinkPrefixLength>32)continue;
            auto *sin=reinterpret_cast<const sockaddr_in*>(u->Address.lpSockaddr);
            const auto addr=ntohl(sin->sin_addr.s_addr);
            const uint32_t mask=u->OnLinkPrefixLength==0?0:u->OnLinkPrefixLength==32?0xffffffffu:
                (0xffffffffu << (32-u->OnLinkPrefixLength));
            if(private_ipv4(addr))out.push_back({addr,mask});
        }
    }
#else
    ifaddrs *head=nullptr;
    if(getifaddrs(&head)!=0)throw std::runtime_error("Could not inspect local interfaces");
    for(auto *it=head;it;it=it->ifa_next) {
        if(!it->ifa_addr || !it->ifa_netmask || it->ifa_addr->sa_family!=AF_INET ||
           !(it->ifa_flags&IFF_UP) || (it->ifa_flags&IFF_LOOPBACK))continue;
        auto *sin=reinterpret_cast<const sockaddr_in*>(it->ifa_addr);
        auto *mask=reinterpret_cast<const sockaddr_in*>(it->ifa_netmask);
        const uint32_t addr=ntohl(sin->sin_addr.s_addr);
        if(private_ipv4(addr))out.push_back({addr,ntohl(mask->sin_addr.s_addr)});
    }
    freeifaddrs(head);
#endif
    return out;
}
inline void scan_network() {
    const auto interfaces=local_interfaces();
    std::set<uint32_t> targets,subnets;
    size_t segments=0;
    for(const auto &iface:interfaces) {
        // On large LANs probe only the /24 containing the local host, at most 2 distinct /24s.
        const uint32_t network=iface.address & iface.mask;
        const uint32_t effective_mask=iface.mask | 0xffffff00u;
        const uint32_t segment=iface.address & effective_mask;
        if(!subnets.insert(segment).second)continue;
        if(segments++>=2)break;
        const uint32_t last=segment | (~effective_mask);
        for(uint32_t host=segment+1;host<last;++host) {
            if((host & iface.mask)!=network || !private_ipv4(host))continue;
            targets.insert(host);
        }
    }
    if(targets.empty()) {
        std::cout << "No active private IPv4 LAN found (loopback and public interfaces excluded).\n";
        return;
    }
    std::vector<uint32_t> addresses(targets.begin(),targets.end());
    std::vector<unsigned char> responses(addresses.size(),0);
    std::atomic<size_t> next{0};
    const size_t workers=std::min<size_t>(16,addresses.size());
    std::vector<std::thread> pool;
    pool.reserve(workers);
    for(size_t k=0;k<workers;++k)pool.emplace_back([&]() {
        for(;;) {
            const auto i=next.fetch_add(1);
            if(i>=addresses.size())break;
            responses[i]=static_cast<unsigned char>(ping_ipv4(addresses[i]));
        }
    });
    for(auto &t:pool)t.join();
    size_t responded=0;
    for(size_t i=0;i<addresses.size();++i) {
        if(!responses[i])continue;
        ++responded;
        std::cout << ipv4_string(addresses[i]) << ": responding\n";
    }
    std::cout << "LAN ICMP discovery: " << responded << "/" << addresses.size()
              << " addresses responded (max two local /24 slices; ICMP may be filtered).\n";
}

} // namespace nat
