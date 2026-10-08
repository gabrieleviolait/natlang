#include "embedded_runtime.hpp"
#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
namespace fs = std::filesystem;
using std::string;
struct Error:std::runtime_error {using std::runtime_error::runtime_error;};
static string trim(string s) {
    const auto a=s.find_first_not_of(" \t\r\n");
    return a==string::npos ? "" : s.substr(a,s.find_last_not_of(" \t\r\n")-a+1);
}
static string lower(string s) {for(char &c:s)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return s;}
static bool match(const string &s,const char *pattern,std::smatch &m) {return std::regex_match(s,m,std::regex(pattern,std::regex::icase));}
static string read(const fs::path &p) {std::ifstream f(p,std::ios::binary);if(!f)throw Error("Cannot open: "+p.string());std::ostringstream s;s<<f.rdbuf();return s.str();}
static void write(const fs::path &p,const string &s) {std::ofstream f(p,std::ios::binary);if(!f)throw Error("Cannot write: "+p.string());f<<s;if(!f)throw Error("Write failed: "+p.string());}
static bool identifier(const string &s) {return std::regex_match(s,std::regex("[A-Za-z_][A-Za-z0-9_]*"));}
static bool reserved(const string &s) {static const std::set<string> words={"true","false","if","else","end","function","return","while","for","repeat","break","continue","null","and","or","not"};return words.count(lower(s))!=0;}
static void validId(const string &s,int line) {if(!identifier(s)||reserved(s))throw Error("Line "+std::to_string(line)+": invalid name: "+s);}
static string cppQuote(const string &s) {
    string o="\"";
    for(unsigned char c:s) {switch(c){case '\\':o+="\\\\";break;case '"':o+="\\\"";break;case '\n':o+="\\n";break;case '\t':o+="\\t";break;case '\r':o+="\\r";break;default:if(c<32){char b[7];std::snprintf(b,sizeof(b),"\\u%04x",c);o+=b;}else o+=static_cast<char>(c);}}
    return o+"\"";
}
enum class K {Assign,Print,Ask,AskMany,If,Until,While,Times,Function,Return,Append,Save,Break,Continue,ScanHost,ScanNetwork};
struct Node {
    K kind{}; int line=0; string a,b,c;
    std::vector<string> params;std::vector<Node> body,otherwise;
};
static string kindName(K k) {
    switch(k){case K::Assign:return "assign";case K::Print:return "print";case K::Ask:return "input";case K::AskMany:return "input-list";case K::If:return "if";case K::Until:return "repeat-until";case K::While:return "while";case K::Times:return "repeat-times";case K::Function:return "function";case K::Return:return "return";case K::Append:return "append";case K::Save:return "save-file";case K::Break:return "break";case K::Continue:return "continue";case K::ScanHost:return "scan-ip";case K::ScanNetwork:return "scan-network";}
    return "unknown";
}
struct Line{int num;string s;};
struct Parser {
    std::vector<Line> lines;size_t at=0;
    explicit Parser(const string &source) {
        std::istringstream f(source);string s;int n=0;
        while(std::getline(f,s)) {
            ++n;s=trim(s);
            if(s.empty()||s[0]=='#'||s.rfind("//",0)==0)continue;
            if(s.back()==':'&&s.size()>1)s.pop_back();
            lines.push_back({n,s});
        }
    }
    std::vector<string> params(const string &s,int line) {
        std::vector<string> v;string z=trim(s);if(z.empty())return v;
        std::stringstream ss(z);string p;
        while(std::getline(ss,p,',')) {p=trim(p);validId(p,line);if(std::find(v.begin(),v.end(),p)!=v.end())throw Error("Line "+std::to_string(line)+": duplicate parameter "+p);v.push_back(p);}
        return v;
    }
    std::vector<Node> seq(bool nested=false,bool allowElse=false,bool inFunction=false,int loopDepth=0) {
        std::vector<Node> out;std::smatch m;
        while(at<lines.size()) {
            auto line=lines[at++];auto &s=line.s;
            if(match(s,R"(end( if| loop| function| repeat| while)?)",m)) {
                if(!nested)throw Error("Line "+std::to_string(line.num)+": unexpected END");
                return out;
            }
            if(match(s,R"(else|otherwise)",m)) {
                if(!allowElse)throw Error("Line "+std::to_string(line.num)+": unexpected ELSE");
                --at;return out;
            }
            Node n;n.line=line.num;
            if(match(s,R"(if (.+?)(?: then)?)",m)) {
                n.kind=K::If;n.a=trim(m[1]);n.body=seq(true,true,inFunction,loopDepth);
                if(at<lines.size()&&match(lines[at].s,R"(else|otherwise)",m)) {
                    ++at;n.otherwise=seq(true,false,inFunction,loopDepth);
                }
                out.push_back(std::move(n));continue;
            }
            if(match(s,R"(repeat until (.+))",m)) {n.kind=K::Until;n.a=trim(m[1]);n.body=seq(true,false,inFunction,loopDepth+1);}
            else if(match(s,R"(while (.+))",m)) {n.kind=K::While;n.a=trim(m[1]);n.body=seq(true,false,inFunction,loopDepth+1);}
            else if(match(s,R"(repeat (.+) times?)",m)) {n.kind=K::Times;n.a=trim(m[1]);n.body=seq(true,false,inFunction,loopDepth+1);}
            else if(match(s,R"(function ([A-Za-z_]\w*)\(([^)]*)\))",m)) {n.kind=K::Function;n.a=m[1];n.params=params(m[2],line.num);if(nested||inFunction)throw Error("Line "+std::to_string(line.num)+": functions must be top-level");n.body=seq(true,false,true,0);}
            else if(match(s,R"(define (?:a )?function (?:called )?([A-Za-z_]\w*)(?: with (?:input|parameter|parameters) (.+))?)",m)) {n.kind=K::Function;n.a=m[1];n.params=params(m[2],line.num);if(nested||inFunction)throw Error("Line "+std::to_string(line.num)+": functions must be top-level");n.body=seq(true,false,true,0);}
            else if(match(s,R"((?:let|set|remember|store) ([A-Za-z_]\w*) (?:be|to|as|equal to) (.+))",m)) {n.kind=K::Assign;n.a=m[1];n.b=trim(m[2]);}
            else if(match(s,R"((?:create|make) (?:an? )?list (?:called|named) ([A-Za-z_]\w*))",m)) {n.kind=K::Assign;n.a=m[1];n.b="[]";}
            else if(match(s,R"((?:increase|increment) ([A-Za-z_]\w*) by (.+))",m)) {n.kind=K::Assign;n.a=m[1];n.b=n.a+" + ("+trim(m[2])+")";}
            else if(match(s,R"((?:decrease|decrement) ([A-Za-z_]\w*) by (.+))",m)) {n.kind=K::Assign;n.a=m[1];n.b=n.a+" - ("+trim(m[2])+")";}
            else if(match(s,R"((?:add|append|put) (.+) (?:to|into) ([A-Za-z_]\w*))",m)) {n.kind=K::Append;n.a=m[2];n.b=trim(m[1]);}
            else if(match(s,R"(ask (?:the user )?for (?:a |an )?(number|text|string)(?: and)? (?:store (?:it )?(?:in|as)|called|named) ([A-Za-z_]\w*))",m)) {n.kind=K::Ask;n.a=m[2];n.b=lower(m[1]);}
            else if(match(s,R"(ask (?:the )?user(?: for (?:a |an )?(number|text|string))?(?: and (?:store (?:it )?(?:in|as)|save (?:it )?as) ([A-Za-z_]\w*))?)",m)) {
                n.kind=K::Ask;n.a=m[2].matched?string(m[2]):"answer";n.b=lower(m[1].matched?string(m[1]):"text");n.c=n.b=="number"?"Number: ":"Input: ";
            }
            else if(match(s,R"(chiedi (?:all['’]utente|ad? (?:un )?utente)(?: (?:un |una )?(numero|testo|text|string))?(?: (?:e )?(?:salva(?:lo)?|memorizza(?:lo)?|metti) (?:in|come) ([A-Za-z_]\w*))?)",m)) {
                n.kind=K::Ask;n.a=m[2].matched?string(m[2]):"answer";n.b=lower(m[1].matched?string(m[1]):"text");n.c=n.b=="numero"?"Numero: ":"Inserisci un valore: ";if(n.b=="numero")n.b="number";
            }
            else if(match(s,R"(ask (?:the )?user ("[^"]*"|'[^']*')(?: and (?:store (?:it )?in|save (?:it )?as) ([A-Za-z_]\w*))?)",m)) {
                n.kind=K::Ask;n.a=m[2].matched?string(m[2]):"answer";n.b="text";n.c=string(m[1]);
            }
            else if(match(s,R"(chiedi (?:all['’]utente|ad? (?:un )?utente) ("[^"]*"|'[^']*')(?: e (?:salva(?:lo)?|memorizza(?:lo)?) (?:in|come) ([A-Za-z_]\w*))?)",m)) {
                n.kind=K::Ask;n.a=m[2].matched?string(m[2]):"answer";n.b="text";n.c=string(m[1]);
            }
            else if(match(s,R"(ask (?:the user )?for (.+) numbers?(?: and)? store them in ([A-Za-z_]\w*))",m)) {n.kind=K::AskMany;n.a=m[2];n.b=trim(m[1]);}
            else if(match(s,R"((?:show|print|display|output|say|mostra|stampa) (.+))",m)) {n.kind=K::Print;n.a=trim(m[1]);}
            else if(match(s,R"(scan (?:the |my )?(?:local )?network|scan (?:la )?rete(?: locale)?)",m)) {n.kind=K::ScanNetwork;}
            else if(match(s,R"((?:scan|ping|scansiona) (?:(?:this|the|questo) )?(?:ip(?: address)?|host|indirizzo ip)(?: (.+))?)",m)) {
                n.kind=K::ScanHost;n.a=trim(m[1]);
            }
            else if(match(s,R"((?:scan|ping|scansiona) ([0-9]+(?:\.[0-9]+){3}))",m)) {n.kind=K::ScanHost;n.a=trim(m[1]);}
            else if(match(s,R"(return (.+))",m)) {n.kind=K::Return;n.a=trim(m[1]);if(!inFunction)throw Error("Line "+std::to_string(line.num)+": RETURN outside function");}
            else if(match(s,R"(save (.+) to (?:a )?file (.+))",m)) {n.kind=K::Save;n.a=trim(m[1]);n.b=trim(m[2]);}
            else if(match(s,R"(load file (.+) (?:into|as) ([A-Za-z_]\w*))",m)) {n.kind=K::Assign;n.a=m[2];n.b="read_file("+trim(m[1])+")";}
            else if(match(s,R"(break|stop the loop)",m)) {n.kind=K::Break;if(!loopDepth)throw Error("Line "+std::to_string(line.num)+": BREAK outside loop");}
            else if(match(s,R"(continue|skip to next iteration)",m)) {n.kind=K::Continue;if(!loopDepth)throw Error("Line "+std::to_string(line.num)+": CONTINUE outside loop");}
            else { // A bare expression is a statement whose result is printed, e.g. "2 plus 2".
                n.kind=K::Print;n.a=s;
            }
            if(n.kind==K::Assign||n.kind==K::Append||n.kind==K::Ask||n.kind==K::AskMany||n.kind==K::Function)validId(n.a,n.line);
            out.push_back(std::move(n));
        }
        if(nested)throw Error("Missing END for a block");
        return out;
    }
};
static string normalizeOutsideQuotes(const string &s) {
    auto convert=[](string z) {
        using P=std::pair<const char*,const char*>;
        static const std::vector<P> rules={
            {R"(\bis (?:greater|more) than or equal to\b)",">="},{R"(\bis less than or equal to\b)","<="},
            {R"(\bis (?:not equal to|different from)\b)","!="},{R"(\b(?:is )?(?:greater|more|higher|larger) than\b)",">"},
            {R"(\b(?:is )?(?:less|lower|smaller) than\b)","<"},
            {R"(\b(?:is equal to|equals|is)\b)","=="},
            {R"(\bmultiplied by\b)","*"},{R"(\bdivided by\b)","/"},
            {R"(\b(?:plus|piu|più)\b)","+"},{R"(\b(?:minus|meno)\b)","-"},
            {R"(\bthe (?:highest|largest|biggest|maximum) (?:number|value) in ([A-Za-z_]\w*)\b)","max($1)"},
            {R"(\bthe (?:smallest|lowest|minimum) (?:number|value) in ([A-Za-z_]\w*)\b)","min($1)"},
            {R"(\bthe average of ([A-Za-z_]\w*)\b)","average($1)"},
            {R"(\bthe sum of ([A-Za-z_]\w*)\b)","sum($1)"},
            {R"(\bthe length of ([A-Za-z_]\w*)\b)","length($1)"}
        };
        // UTF-8 accented Italian operators need explicit boundaries (regex \b is ASCII word-based).
        z=std::regex_replace(z,std::regex(R"(\bpiù(?=\s|$|[),]))"),"+");
        // Replace long phrases before individual operators, and quantifiers before plain words.
        for(size_t i=0;i<6;++i)z=std::regex_replace(z,std::regex(rules[i].first,std::regex::icase),rules[i].second);
        for(size_t i=10;i<rules.size();++i)z=std::regex_replace(z,std::regex(rules[i].first,std::regex::icase),rules[i].second);
        for(size_t i=6;i<10;++i)z=std::regex_replace(z,std::regex(rules[i].first,std::regex::icase),rules[i].second);
        return z;
    };
    string out,fragment;
    for(size_t i=0;i<s.size();) {
        if(s[i]=='"'||s[i]=='\'') {
            out+=convert(fragment);fragment.clear();char q=s[i];out+=s[i++];bool finished=false;
            while(i<s.size()) {char c=s[i++];out+=c;if(c=='\\'&&i<s.size()){out+=s[i++];continue;}if(c==q){finished=true;break;}}
            if(!finished)throw Error("Unterminated string in expression: "+s);
        } else fragment+=s[i++];
    }
    out+=convert(fragment);return trim(out);
}
enum class T{Id,Number,Str,Op,LParen,RParen,LBracket,RBracket,Comma,End};
struct Token{T type;string text;};
static std::vector<Token> lex(const string &src) {
    const string s=normalizeOutsideQuotes(src);std::vector<Token> v;
    for(size_t i=0;i<s.size();) {
        char c=s[i];if(std::isspace(static_cast<unsigned char>(c))){++i;continue;}
        if(std::isalpha(static_cast<unsigned char>(c))||c=='_') {size_t j=i+1;while(j<s.size()&&(std::isalnum(static_cast<unsigned char>(s[j]))||s[j]=='_'))++j;string t=s.substr(i,j-i);string l=lower(t);v.push_back({l=="and"||l=="or"||l=="not"?T::Op:T::Id,l=="true"||l=="false"?l:t});i=j;continue;}
        if(std::isdigit(static_cast<unsigned char>(c))||(c=='.'&&i+1<s.size()&&std::isdigit(static_cast<unsigned char>(s[i+1])))) {
            size_t j=i;while(j<s.size()&&std::isdigit(static_cast<unsigned char>(s[j])))++j;
            if(j<s.size()&&s[j]=='.'){++j;while(j<s.size()&&std::isdigit(static_cast<unsigned char>(s[j])))++j;}
            v.push_back({T::Number,s.substr(i,j-i)});i=j;continue;
        }
        if(c=='"'||c=='\'') {char q=c;string z;bool done=false;++i;
            while(i<s.size()){char x=s[i++];if(x==q){done=true;break;}if(x=='\\'){if(i==s.size())break;char e=s[i++];switch(e){case 'n':z+='\n';break;case 't':z+='\t';break;case 'r':z+='\r';break;case '\\':z+='\\';break;case '\'':z+='\'';break;case '"':z+='"';break;default:throw Error("Unsupported string escape");}}else z+=x;}
            if(!done)throw Error("Unterminated string literal");
            v.push_back({T::Str,z});continue;
        }
        T type=T::Op;switch(c){case '(':type=T::LParen;break;case ')':type=T::RParen;break;case '[':type=T::LBracket;break;case ']':type=T::RBracket;break;case ',':type=T::Comma;break;}
        string op(1,c);if(type==T::Op){if(i+1<s.size()&&(s.substr(i,2)=="=="||s.substr(i,2)=="!="||s.substr(i,2)==">="||s.substr(i,2)=="<=")){op=s.substr(i,2);++i;}
            if(op!="+"&&op!="-"&&op!="*"&&op!="/"&&op!="%"&&op!="<"&&op!=">"&&op!="=="&&op!="!="&&op!=">="&&op!="<=")throw Error("Unexpected expression character: "+op);
        }
        v.push_back({type,op});++i;
    }
    v.push_back({T::End,""});return v;
}
static void collect(const std::vector<Node> &ns,std::set<string> &vars) {
    for(const auto &n:ns){if(n.kind==K::Assign||n.kind==K::Ask||n.kind==K::AskMany)vars.insert(n.a);collect(n.body,vars);collect(n.otherwise,vars);}
}
struct Expr {
    std::vector<Token> toks;size_t at=0;int line;
    const std::set<string> &vars;
    const std::map<string,size_t> &funcs;
    Expr(const string &src,int ln,const std::set<string> &v,const std::map<string,size_t> &f):toks(lex(src)),line(ln),vars(v),funcs(f){}
    const Token &peek(){return toks.at(at);}
    Token take(){return toks.at(at++);}
    bool eat(T type){if(peek().type==type){++at;return true;}return false;}
    void expect(T type){if(!eat(type))throw Error("Line "+std::to_string(line)+": malformed expression near '"+peek().text+"'");}
    static int prec(const string &s){if(s=="or")return 1;if(s=="and")return 2;if(s=="=="||s=="!="||s=="<"||s==">"||s=="<="||s==">=")return 3;if(s=="+"||s=="-")return 4;if(s=="*"||s=="/"||s=="%")return 5;return -1;}
    string expr(int minp=1) {
        string lhs;Token t=take();
        if(t.type==T::Number){lhs="nat::Value("+t.text+")";}
        else if(t.type==T::Str){lhs="nat::Value("+cppQuote(t.text)+")";}
        else if(t.type==T::Id) {
            const auto low=lower(t.text);
            if(low=="true"||low=="false")lhs="nat::Value("+low+")";
            else if(eat(T::LParen)) {
                std::vector<string> args;
                if(!eat(T::RParen)){do{args.push_back(expr());}while(eat(T::Comma));expect(T::RParen);}
                const std::map<string,std::pair<string,size_t>> builtins={{"sum",{"nat::sum",1}},{"average",{"nat::average",1}},{"avg",{"nat::average",1}},{"max",{"nat::maximum",1}},{"min",{"nat::minimum",1}},{"length",{"nat::length",1}},{"read_file",{"nat::read_file",1}},{"ping",{"nat::ping",1}}};
                string callee;
                if(auto it=builtins.find(low);it!=builtins.end()) {if(args.size()!=it->second.second)throw Error("Line "+std::to_string(line)+": wrong argument count for "+t.text);callee=it->second.first;}
                else if(auto it=funcs.find(t.text);it!=funcs.end()){if(args.size()!=it->second)throw Error("Line "+std::to_string(line)+": wrong argument count for "+t.text);callee="fn_"+t.text;}
                else throw Error("Line "+std::to_string(line)+": unknown function: "+t.text);
                lhs=callee+"(";
                for(size_t i=0;i<args.size();++i){if(i)lhs+=",";lhs+=args[i];}lhs+=")";
            }
            else {if(!vars.count(t.text))throw Error("Line "+std::to_string(line)+": unknown variable '"+t.text+"' (use quotes for text)");lhs="nat::get(v_"+t.text+","+cppQuote(t.text)+")";}
        }
        else if(t.type==T::Op&&(t.text=="-"||t.text=="not"||t.text=="+")) {
            auto rhs=expr(6);lhs=t.text=="not"?"nat::Value(!nat::truth("+rhs+"))":(t.text=="-"?"nat::Value(-nat::number("+rhs+"))":"nat::Value(nat::number("+rhs+"))");
        }
        else if(t.type==T::LParen){lhs=expr();expect(T::RParen);}
        else if(t.type==T::LBracket){
            std::vector<string> items;if(!eat(T::RBracket)){do{items.push_back(expr());}while(eat(T::Comma));expect(T::RBracket);}
            lhs="nat::Value(nat::Value::List{";
            for(size_t i=0;i<items.size();++i){if(i)lhs+=",";lhs+=items[i];}lhs+="})";
        }
        else throw Error("Line "+std::to_string(line)+": expected expression, got '"+t.text+"'");
        while(peek().type==T::Op&&prec(peek().text)>=minp){string op=take().text;int p=prec(op);string rhs=expr(p+1);
            if(op=="and"||op=="or")lhs="nat::Value(nat::truth("+lhs+") "+string(op=="and"?"&&":"||")+" nat::truth("+rhs+"))";
            else if(op=="=="||op=="!=")lhs="nat::Value("+string(op=="!="?"!":"")+"nat::eq("+lhs+","+rhs+"))";
            else if(op=="<"||op==">"||op=="<="||op==">=")lhs="nat::Value(nat::cmp("+lhs+","+rhs+")"+op+"0)";
            else {string fn=op=="+"?"add":op=="-"?"sub":op=="*"?"mul":op=="/"?"div":"mod";lhs="nat::"+fn+"("+lhs+","+rhs+")";}
        }
        return lhs;
    }
    string compile(){string x=expr();if(peek().type!=T::End)throw Error("Line "+std::to_string(line)+": unexpected token: "+peek().text);return x;}
};
struct Emit {
    const std::vector<Node> &program;std::map<string,size_t> funcs;
    explicit Emit(const std::vector<Node> &p):program(p){for(const auto &n:p)if(n.kind==K::Function){if(funcs.count(n.a))throw Error("Line "+std::to_string(n.line)+": duplicate function "+n.a);funcs[n.a]=n.params.size();}}
    string expression(const string &s,int line,const std::set<string>&v){try{return Expr(s,line,v,funcs).compile();}catch(const Error &e){throw Error(e.what());}}
    string statements(const std::vector<Node> &ns,const std::set<string> &vars,int indent) {
        string out;auto pad=[&](){return string(static_cast<size_t>(indent)*4,' ');};
        for(const auto &n:ns) {
            const auto p=pad();out+=p+"// .nat line "+std::to_string(n.line)+"\n";
            switch(n.kind) {
                case K::Function:throw Error("Nested function not allowed");
                case K::Assign:out+=p+"v_"+n.a+" = "+expression(n.b,n.line,vars)+";\n";break;
                case K::Ask:
                    if(n.c.empty())out+=p+"v_"+n.a+" = nat::"+(n.b=="number"?"read_number":"read_text")+"();\n";
                    else {
                        const string prompt=(n.c.front()=='\''||n.c.front()=='"')?expression(n.c,n.line,vars):"nat::Value("+cppQuote(n.c)+")";
                        out+=p+"v_"+n.a+" = nat::"+(n.b=="number"?"ask_number":"ask_text")+"("+prompt+");\n";
                    }
                    break;
                case K::AskMany:out+=p+"v_"+n.a+" = nat::read_numbers("+expression(n.b,n.line,vars)+");\n";break;
                case K::Append:if(!vars.count(n.a))throw Error("Line "+std::to_string(n.line)+": unknown list "+n.a);out+=p+"nat::append(v_"+n.a+","+expression(n.b,n.line,vars)+");\n";break;
                case K::Print:out+=p+"nat::print("+expression(n.a,n.line,vars)+");\n";break;
                case K::Save:out+=p+"nat::write_file("+expression(n.b,n.line,vars)+","+expression(n.a,n.line,vars)+");\n";break;
                case K::Return:out+=p+"return "+expression(n.a,n.line,vars)+";\n";break;
                case K::Break:out+=p+"break;\n";break;
                case K::Continue:out+=p+"continue;\n";break;
                case K::ScanNetwork:out+=p+"nat::scan_network();\n";break;
                case K::ScanHost: {
                    const string target=n.a.empty()?"nat::ask_text(nat::Value(\"IP to scan: \"))":
                      std::regex_match(n.a,std::regex(R"([0-9]+(?:\.[0-9]+){3})"))?"nat::Value("+cppQuote(n.a)+")":expression(n.a,n.line,vars);
                    out+=p+"nat::scan_ip("+target+");\n";
                    break;
                }
                case K::If:
                    out+=p+"if (nat::truth("+expression(n.a,n.line,vars)+")) {\n";
                    out+=statements(n.body,vars,indent+1)+p+"}";
                    if(!n.otherwise.empty())out+=" else {\n"+statements(n.otherwise,vars,indent+1)+p+"}";
                    out+="\n";break;
                case K::While:case K::Until:
                    out+=p+"while ("+string(n.kind==K::Until?"!":"")+"nat::truth("+expression(n.a,n.line,vars)+")) {\n";
                    out+=statements(n.body,vars,indent+1)+p+"}\n";break;
                case K::Times:
                    out+=p+"for(long long _nat_i"+std::to_string(n.line)+"=0,_nat_n"+std::to_string(n.line)+"=nat::repeat_count("+expression(n.a,n.line,vars)+"); _nat_i"+std::to_string(n.line)+"<_nat_n"+std::to_string(n.line)+"; ++_nat_i"+std::to_string(n.line)+") {\n";
                    out+=statements(n.body,vars,indent+1)+p+"}\n";break;
            }
        }
        return out;
    }
    string generate() {
        string out="// NatLang 0.2 | Generated C++20. Review generated code before distributing.\n";
        out+=NAT_RUNTIME;out+="\n";
        for(const auto &n:program)if(n.kind==K::Function){out+="nat::Value fn_"+n.a+"(";for(size_t i=0;i<n.params.size();++i){if(i)out+=",";out+="nat::Value v_"+n.params[i];}out+=");\n";}
        for(const auto &n:program)if(n.kind==K::Function) {
            out+="nat::Value fn_"+n.a+"(";
            for(size_t i=0;i<n.params.size();++i){if(i)out+=",";out+="nat::Value v_"+n.params[i];}out+=") {\n";
            std::set<string> vars;collect(n.body,vars);
            for(const auto &p:n.params)vars.insert(p);
            std::set<string> locals=vars;for(const auto &p:n.params)locals.erase(p);
            for(const auto &x:locals)out+="    nat::Value v_"+x+";\n";
            out+=statements(n.body,vars,1);out+="    return nat::Value();\n}\n";
        }
        std::vector<Node> body;for(const auto &n:program)if(n.kind!=K::Function)body.push_back(n);
        std::set<string> vars;collect(body,vars);
        out+="int main() {\n    try {\n";
        for(const auto &x:vars)out+="        nat::Value v_"+x+";\n";
        out+=statements(body,vars,2);
        out+="        return 0;\n    } catch(const std::exception &e) { std::cerr << \"Runtime error: \" << e.what() << '\\n'; return 1; }\n}\n";
        return out;
    }
};
static string jsonString(const string &obj,const string &key) {
    const auto pos=obj.find(cppQuote(key));if(pos==string::npos)throw Error("JSON field not found: "+key);
    auto i=obj.find(':',pos+key.size()+2);if(i==string::npos)throw Error("Invalid JSON");
    ++i;while(i<obj.size()&&std::isspace(static_cast<unsigned char>(obj[i])))++i;
    if(i>=obj.size()||obj[i]!='"')throw Error("JSON value is not a string: "+key);
    ++i;
    string out;
    while(i<obj.size()) {
        char c=obj[i++];if(c=='"')return out;
        if(c!='\\'){out+=c;continue;}
        if(i>=obj.size())break;
        char e=obj[i++];
        switch(e){case '"':out+='"';break;case '\\':out+='\\';break;case '/':out+='/';break;case 'n':out+='\n';break;case 'r':out+='\r';break;case 't':out+='\t';break;case 'b':out+='\b';break;case 'f':out+='\f';break;
            case 'u':{if(i+4>obj.size())throw Error("Truncated Unicode escape");unsigned u=0;for(int j=0;j<4;++j){char h=obj[i++];u=u*16+static_cast<unsigned>(h>='0'&&h<='9'?h-'0':h>='a'&&h<='f'?h-'a'+10:h>='A'&&h<='F'?h-'A'+10:256);}
                if(u>0xFFFF)throw Error("Invalid Unicode escape");
                if(u<0x80)out+=static_cast<char>(u);else if(u<0x800){out+=static_cast<char>(0xC0|(u>>6));out+=static_cast<char>(0x80|(u&63));}else{out+=static_cast<char>(0xE0|(u>>12));out+=static_cast<char>(0x80|((u>>6)&63));out+=static_cast<char>(0x80|(u&63));}break;}
            default:throw Error("Unsupported JSON escape");
        }
    }
    throw Error("Unterminated JSON string");
}
static string shellQuote(const string &s) {
#ifdef _WIN32
    // Quote a single argument for cmd.exe. Avoid metacharacter expansion.
    if(s.find_first_of("%\r\n\"!^")!=string::npos)throw Error("Unsupported path characters for Windows shell");
    return "\""+s+"\"";
#else
    string z="'";for(char c:s){if(c=='\'')z+="'\\''";else z+=c;}return z+"'";
#endif
}
static string llm(const string &source,const string &url) {
    if(!std::regex_match(url,std::regex(R"(http://(?:127\.0\.0\.1|localhost):[0-9]{2,5}/v1/chat/completions)")))throw Error("--llm-url must be a localhost llama-server /v1/chat/completions endpoint");
    const string rules=R"(Translate English/Italian natural-language program into NatLang v0.2. Reply ONLY JSON with key program. Never guess missing information. NatLang line format:
Set x to EXPR
Show EXPR
EXPR (bare expressions are printed)
Ask user
Ask user "Your name?" and store in name
Chiedi all'utente un numero e salva in x
Scan network
Scan IP 192.168.1.1
Scan this ip
Ask for a number and store it in x
Ask for text and store it in x
Ask for N numbers and store them in listname
Create a list named x
Add EXPR to x
Increase x by EXPR
If CONDITION\n[body]\nOtherwise\n[body]\nEnd
Repeat until CONDITION\n...\nEnd
While CONDITION\n...\nEnd
Repeat N times\n...\nEnd
Define a function called name with parameter x\nReturn EXPR\nEnd
Save EXPR to file "path"
Load file "path" into x
Expressions: numbers, quoted strings, true/false, variables, arithmetic + - * / %, comparisons == != < > <= >=, boolean and/or/not, [items], max(list), min(list), sum(list), average(list), length(list), name(args). Always quote literal text. Use END to close each block. If source cannot be represented, leave offending instruction untouched so compiler fails visibly. Preserve intended behavior. Output canonical program without markdown.)";
    string request="{\"model\":\"local-model\",\"temperature\":0,\"seed\":42,\"stream\":false,\"max_tokens\":2400,\"messages\":[{\"role\":\"system\",\"content\":"+cppQuote(rules)+"},{\"role\":\"user\",\"content\":"+cppQuote(source)+"}],\"response_format\":{\"type\":\"json_schema\",\"schema\":{\"type\":\"object\",\"properties\":{\"program\":{\"type\":\"string\"}},\"required\":[\"program\"],\"additionalProperties\":false}}}";
    const auto id=std::chrono::steady_clock::now().time_since_epoch().count();const auto base=fs::temp_directory_path()/("natc-"+std::to_string(id));
    const fs::path req=base.string()+".request.json",resp=base.string()+".response.json";
    struct Cleanup {fs::path a,b;~Cleanup(){std::error_code ec;fs::remove(a,ec);fs::remove(b,ec);}} cleanup{req,resp};
    write(req,request);
    string cmd="curl -sSf --max-time 120 --connect-timeout 4 -H "+shellQuote("Content-Type: application/json")+" --data-binary "+shellQuote("@"+req.string())+" -o "+shellQuote(resp.string())+" "+shellQuote(url);
    if(std::system(cmd.c_str())!=0)throw Error("Local llama-server request failed (ensure llama-server and curl are running)");
    const string response=read(resp);const auto msg=response.find("\"message\"");
    if(msg==string::npos)throw Error("LLM response missing choices[0].message");
    const string content=jsonString(response.substr(msg),"content");
    return jsonString(content,"program");
}
static void explain(const std::vector<Node> &nodes,int depth=0) {
    for(const auto &n:nodes) {
        std::cout<<string(static_cast<size_t>(depth)*2,' ')<<n.line<<" "<<kindName(n.kind)<<" "<<n.a;
        if(!n.b.empty())std::cout<<" | "<<n.b;
        std::cout<<"\n";
        explain(n.body,depth+1);
        if(!n.otherwise.empty()){std::cout<<string(static_cast<size_t>(depth)*2+2,' ')<<"else\n";explain(n.otherwise,depth+1);}
    }
}
static void help() {
    std::cout<<"NatLang compiler v0.2 (C++20)\n"
    <<"  natc source.nat [-o output] [--compiler clang++|g++|cl]\n"
    <<"  natc source.nat --emit-cpp [generated.cpp]\n"
    <<"  natc source.nat --check [--explain]\n"
    <<"  natc source.nat --llm [--llm-url http://127.0.0.1:8080/v1/chat/completions]\n"
    <<"  natc source.nat --llm-all  (always normalize using local model)\n"
    <<"  natc --eval \"2 plus 2\"  (compile and execute an expression immediately)\n"
    <<"Flags: --keep-cpp (retain generated source), --show-normalized, --help\n";
}
int main(int argc,char **argv) {
    try {
        if(argc<2){help();return 1;}
        fs::path source,output,cpp;string compiler="",url="http://127.0.0.1:8080/v1/chat/completions",evalCode;
        bool emit=false,check=false,exp=false,useLLM=false,forceLLM=false,keep=false,showNormalized=false,eval=false;
        for(int i=1;i<argc;++i) {
            string a=argv[i];if(a=="--help"||a=="-h"){help();return 0;}
            else if(a=="-o"){if(++i==argc)throw Error("-o requires output path");output=argv[i];}
            else if(a=="--compiler"){if(++i==argc)throw Error("--compiler requires compiler name");compiler=argv[i];}
            else if(a=="--llm-url"){if(++i==argc)throw Error("--llm-url requires URL");url=argv[i];}
            else if(a=="--eval"||a=="-e"){if(++i==argc)throw Error("--eval requires a NatLang expression");evalCode=argv[i];eval=true;}
            else if(a=="--llm"){useLLM=true;}
            else if(a=="--llm-all"){useLLM=true;forceLLM=true;}
            else if(a=="--keep-cpp"){keep=true;}
            else if(a=="--show-normalized"){showNormalized=true;}
            else if(a=="--check"){check=true;}
            else if(a=="--explain"){exp=true;}
            else if(a=="--emit-cpp"){emit=true;if(i+1<argc&&argv[i+1][0]!='-')cpp=argv[++i];}
            else if(!a.empty()&&a[0]=='-')throw Error("Unknown flag: "+a);
            else if(source.empty())source=a;
            else throw Error("Unexpected argument: "+a);
        }
        if(eval && !source.empty())throw Error("--eval cannot be combined with a source file");
        if(eval && (!output.empty() || emit || keep || !cpp.empty()))throw Error("--eval cannot be combined with -o, --emit-cpp or --keep-cpp");
        if(!eval && source.empty())throw Error("Missing source file");
        if(eval) {
            source=fs::temp_directory_path()/("natc-eval-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+".nat");
            write(source,evalCode);
        }
        struct EvalCleanup {
            bool active;fs::path source;
            ~EvalCleanup(){if(!active)return;std::error_code ec;fs::remove(source,ec);auto cpp=source;cpp.replace_extension(".generated.cpp");fs::remove(cpp,ec);}
        } evalCleanup{eval,source};
        const string original=read(source);string program=original;
        std::vector<Node> ast;string generated;
        auto compile=[&]() {Parser p(program);ast=p.seq();generated=Emit(ast).generate();};
        if(forceLLM){program=llm(original,url);compile();}
        else {try{compile();}catch(const Error &e){if(!useLLM)throw;std::cerr<<"Deterministic frontend: "<<e.what()<<"\nTrying local LLM...\n";program=llm(original,url);compile();}}
        if(showNormalized&&program!=original)std::cout<<"--- Normalized by local LLM ---\n"<<program<<"\n--- End normalized ---\n";
        if(exp)explain(ast);
        if(check){std::cout<<"OK: "+std::to_string(ast.size())+" top-level statements\n";return 0;}
        if(cpp.empty()){cpp=source;cpp.replace_extension(".generated.cpp");}
        write(cpp,generated);
        if(emit){std::cout<<"C++20 written to "<<cpp.string()<<"\n";return 0;}
#ifdef _WIN32
        if(output.empty()){output=source;output.replace_extension(".exe");}
#else
        if(output.empty()){output=source;output.replace_extension("");}
#endif
        if(compiler.empty()) {
#ifdef _WIN32
            // MSVC is used if available from a Developer Command Prompt; otherwise try Clang/GCC.
            compiler="clang++";
            if(std::system("where clang++ >nul 2>nul")!=0)compiler=(std::system("where g++ >nul 2>nul")==0?"g++":"cl");
#else
            compiler="clang++";
            if(std::system("command -v clang++ >/dev/null 2>&1")!=0)compiler="g++";
#endif
        }
        string cmd;
        const auto base=lower(fs::path(compiler).filename().string());
        if(base=="cl"||base=="cl.exe")cmd=shellQuote(compiler)+" /nologo /std:c++20 /utf-8 /EHsc /Fe:"+shellQuote(output.string())+" "+shellQuote(cpp.string());
        else {
            cmd=shellQuote(compiler)+" -std=c++20 -O2 "+shellQuote(cpp.string())+" -o "+shellQuote(output.string());
#ifdef _WIN32
            cmd+=" -liphlpapi -lws2_32";
#else
            cmd+=" -pthread";
#endif
        }
        if(!eval)std::cout<<"Native compilation: "<<compiler<<"\n"<<std::flush;
        const int rc=std::system(cmd.c_str());
        if(rc!=0)throw Error("C++ compilation failed; generated file retained at "+cpp.string());
        if(!keep){std::error_code ec;fs::remove(cpp,ec);}
        if(eval) {
            const int status=std::system(shellQuote(output.string()).c_str());
            std::error_code ec;fs::remove(output,ec);
            return status==0?0:1;
        }
        std::cout<<"Built "<<output.string()<<"\n";
        return 0;
    } catch(const std::exception &e){std::cerr<<"natc error: "<<e.what()<<"\n";return 1;}
}
