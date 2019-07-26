#ifndef BERRYMATH_INTERPRETER_H
#define BERRYMATH_INTERPRETER_H

#include <string>
#include <sstream>
#include "value.h"
#include "ast.h"

using std::string;

namespace BM {
    class Interpreter {
    public:
        Interpreter(string s = "", string fn = "", Interpreter *p = nullptr) : script(s), filename(fn), ast(new AST(s)),
                                                                               parent(p),
                                                                               scope(new Scope(p ? p->scope : nullptr)),
                                                                               child(false) {
            script += "\n";
            std::regex pattern("//.*[$\n]", std::regex::icase);
            script = std::regex_replace(script, pattern, "\n");
            script += "\npass";
        }
        void open(string s, string fn = "") {
            script = s;
            ast->open(script);
            filename = fn;
            script += "\n";
            std::regex pattern("//.*[$\n]", std::regex::icase);
            script = std::regex_replace(script, pattern, "\n");
            script += "\npass";
        }

        string compile();
        Object *run();
        Object *runCC();
        string fn() { return filename; }
        ~Interpreter() {}
        Variable &operator[](const string &s) { return *scope->get(s); }
        void set(const string &name, Object *v) { scope->set(name, v); }
        Variable *get(const string &name, Scope::Flag flag = Scope::ALL_MIGHT) { return scope->get(name, flag); }
        void del(const string &name) { scope->del(name); }
    private:
        string script;
        string filename;
        AST *ast;
        Scope *scope;
        bool child = false;
        Interpreter *parent;

        static bool isNumber(const string &n) {
            if (n[0] == '.') return false;
            bool doted = false;
            for (UL i = 0; i < n.length(); i++) {
                if (n[i] == '.') {
                    if (doted) return false;
                    doted = true;
                } else if (!(n[i] >= '0' && n[i] <= '9')) return false;
            }
            return true;
        }

        static bool isString(const string &n) {
            char c = n[0];
            if (c != '\'' && c != '"') return false;
            return n[n.length() - 1] == c;
        }

        double transSD(const string &s) {// trans string to double
            std::stringstream ss(s);
            double t;
            ss >> t;
            return t;
        }

#define PASS_ERROR "__ERROR__"
#define PASS_RETURN "__RETURN__"
#define THROW exports->set(PASS_ERROR, new Number(1));return exports;
#define CHECKITER(e, ast) \
    if (!e || e->get(PASS_ERROR)) { \
        std::cerr << "at <" << filename << ">:" << ast->line() << std::endl; \
        THROW; \
    }
#define RIGHTEXPRTYPE(left, right) if (left->type() == NUMBER && right->type() == NUMBER)
#define WRONGEXPRTYPE(op) else { std::cerr << "TypeError: Cannot perform " << op << " operations at <" << filename << ">:" \
            << ast->line() << std::endl; \
            THROW; \
}
#define WRONGSCRIPT(token) else { std::cerr << "TypeError: Cannot " << token << " at <" << filename << ">:" \
            << ast->line() << std::endl; \
            THROW; \
}
#define WRONG(name, s)  else { std::cerr << name << ": " << s << " at <" << filename << ">:" \
            << ast->line() << std::endl; \
            THROW; \
}
#define NOTDEFINED(v, name) if (!v) { \
        std::cerr << "ReferenceError: " << name << " is not defined at <" << filename << ">:" \
        << ast->line()  << std::endl; \
        THROW; \
    }
#ifdef Windows95
        // 拓展库path
#define BMMPATH "C:\\BM\\libraries\\lib"
        // 模块库path
#define BMLMPATH "C:\\BM\\modules\\"
#else
        // 拓展库path
#define BMMPATH "/usr/local/BM/libraries/lib"
        // 模块库path
#define BMLMPATH "/usr/local/BM/modules/"
#endif

        friend class NativeFunction;

    public:
        using initModuleFun = BM::Object *(*)();
    };
}


#endif //BERRYMATH_INTERPRETER_H