#include <iostream>
#include <regex>
#include "color.h"
#include "memory.h"
#include "AST.h"
#include "script.h"
#include "version.h"
#include <fstream>
#include <dlfcn.h>
using std::to_string;

BerryMath::value* BerryMath::script::run(long line) {
    int i = 0;
    scope = new block(parent);
    auto ret = new value();
    string c("");

    if (!parent) {
        // 要载入预定义值
        // 预定义值存放在system.json->defines->values中
        // system.json存放在系统目录中, 类unix系统为/usr/local/BerryMath/, windows为C:\Program Files\BerryMath\.
        auto version = systemJson["version"].asString();
        if (!canRun(version)) {
            Throw(-1, "ProgramError: The system and program versions are too different to run");
            note("System version: " + string(BERRYMATH_VERSION) + ", program version: " + version);
            return ret;
        }
    }
    auto values = systemJson["defines"]["values"];
    auto valueKeys = values.getMemberNames();
    for (auto iter = valueKeys.begin(); iter != valueKeys.end(); iter++) {
        auto v = values[*iter].asString();
//            std::cout << *iter << ": " << v << std::endl;
        scope->insert(new variable(*iter, new value(v)));
    }
    auto libs = systemJson["defines"]["libs"];
    auto libKeys = libs.getMemberNames();
    for (auto iter = libKeys.begin(); iter != libKeys.end(); iter++) {
        auto v = libs[*iter].asString();
        void* handle = dlopen(v.c_str(), RTLD_NOW);
        if (!handle) {
            Throw(-1, "ModuleError: Unable to open the dynamic link library '" + v + "' of library: '" + *iter + "'");
        }
//        std::cout << *iter << ": " << v << std::endl;
        libraries.insert(std::pair<string, void*>(v, handle));
    }

//    std::regex replacer_self_add("++");
//    code = std::regex_replace(code, replacer_self_add, "+= 1");
    int bigBracketsCount = 0;
    bool noBrackets = true;
    while (true) {
        if (i >= code.length()) break;
        c += code[i];
//        std::cout << code[i];
        if (code[i] == '\n') {
            line++;
        }
        if (code[i] == '{') {
            noBrackets = false;
            bigBracketsCount++;
        }
        if (code[i] == '}') {
            noBrackets = false;
            bigBracketsCount--;
        }
        if (code[i] == ';' && noBrackets) {// 单行语句
//            std::cout << "build AST." << std::endl;
            ast = new BerryMath::AST(c);
            ast->parse();
            auto root = ast->value();
            parse(ret, root, line);
            c = "";
        }
        if (!noBrackets && bigBracketsCount == 0) {// 包含大括号的诸如if语句等
            ast = new BerryMath::AST(c);
            ast->parse();
//            std::cout << "!@$" << std::endl;
            if (ast->value()->at(0)->value() == "if") {
//                std::cout << "123" << std::endl;
                auto s = new script(new AST(ast->value()->at(0)->at(0)), filename, this);
                s->init(systemJsonContent);
                auto r = s->run(line);
//                std::cout << r->valueOf() << std::endl;
            }
            c = "";
        }
        i++;
    }
    if (selfAst) {
        auto root = ast->value();
        parse(ret, root, line);
        c = "";
    }
    if (!parent) {
        std::cout << "[Program finish]" << std::endl;
        std::cout << "=========Hash Table=========" << std::endl;
        scope->each([](variable* var) -> void {
            std::cout << var->nameOf() << ": " << var->valueOf().valueOf() << std::endl;
        });
        std::cout << "=======Hash Table End=======" << std::endl;
    }
    return ret;
}

void BerryMath::script::parse(value*& ret, AST::ASTNode *root, long line) {
    auto now = root;
    if (root->value() == "bad-tree") {
        // 错误的树
        Throw(line, "SyntaxError: Bad tree");
    }
    if (root->value() == "root") {
        now = root->at(-1);
    }
    string op = now->value();
    if (now->children.size() == 0) {
        delete ret;
//        std::cout << op << std::endl;
        ret = new value(op);
//        std::cout << op << ", " << ret->typeOf() << std::endl;
        if (ret->typeOf() == TOKEN_NAME) {// 可能是变量
//            std::cout << "123" << std::endl;
            auto var = scope->of(op);
            if (var) {// 是变量(即能在作用域中找到该变量)
                delete ret;
                if (var->valueOf().typeOf() == OBJECT) ret = &var->valueOf();
                else ret = new value(var->valueOf().typeOf(), var->valueOf().valueOf());
            }
        }
    }
    if (op == "=") {
        string name = now->at(0)->value();
        auto s = new script(new AST(now->at(1)), filename, this);
        s->init(systemJsonContent);
        auto v = s->run(line);
        scope->set(name, v);
//                std::cout << "set: " << name << std::endl;
    }
    if (op == "~") {
        auto s = new script(new AST(now->at(0)), filename, this);
        s->init(systemJsonContent);
        auto r = s->run(line);
        if (r->typeOf() != NUMBER) {
            Throw(line, "SyntaxError: Wrong expression");
            return;
        }
        int n = (int) atof(r->valueOf().c_str());
        delete ret;
        ret = new value(to_string(~n));
//                std::cout << "set: " << name << std::endl;
    }
    if (op == "++" || op == "--") {
        string name = now->at(0)->value();
        auto var = scope->of(name);
        if (var) {
            auto v = &var->valueOf();
            if (v->typeOf() == NUMBER) {
                double res = atof(v->valueOf().c_str());
                if (op == "++") res += 1;
                if (op == "--") res += 1;
                delete v;
                v = new value(to_string(res));
            } else {
                Throw(line, "TypeError: name '" + name + "' must be number");
                return;
            }
            scope->set(name, v);
        } else {
            Throw(line, "NameError: name '" + name + "' not defined");
        }
    }
    if (op == "!") {
        auto s = new script(new AST(now->at(0)), filename, this);
        s->init(systemJsonContent);
        auto r = s->run(line);
        bool n = isTrue(r->valueOf());
        delete ret;
        if (n) {
            ret = new value("0");
        } else {
            ret = new value("1");
        }
//                std::cout << "set: " << name << std::endl;
    }
    if (
            op == "*" || op == "/" || op == "%"
            || op == "+" || op == "-"
            || op == ">>" || op == "<<"
            || op == "&" || op == "|" || op == "^"
            || op == ">" || op == ">="
            || op == "<" || op == "<="
            ) {// 四则运算
        auto ls = new script(new AST(now->at(0)), filename, this);
        ls->init(systemJsonContent);
        auto rs = new script(new AST(now->at(1)), filename, this);
        rs->init(systemJsonContent);
        auto l = ls->run(line);
        auto r = rs->run(line);
        if (l->typeOf() == STRING && r->typeOf() == STRING && op == "+") {// 字符串加法
            string lv = l->valueOf();
            string rv = r->valueOf();
            // 去除引号
            lv.erase(lv.begin());
            lv.erase(lv.end() - 1);
            rv.erase(rv.begin());
            rv.erase(rv.end() - 1);
            string res = lv + rv;
            delete ret;
            ret = new value(STRING, "\"" + res + "\"");
        } else {
            if (l->typeOf() != NUMBER || r->typeOf() != NUMBER) {
//                std::cout << l->valueOf() << ", " << r->valueOf() << std::endl;
                Throw(line, "TypeError: Unable to run expression because this expression must be (number operator number)");
                return;
            }
            double lv = atof(l->valueOf().c_str());
            double rv = atof(r->valueOf().c_str());
            double res = 0;
            if (op == "*") res = lv * rv;
            if (op == "/") res = lv / rv;
            if (op == "%") res = ((int)lv) % ((int)rv);
            if (op == "+") res = lv + rv;
            if (op == "-") res = lv - rv;
            if (op == "<<") res = ((int)lv) << ((int)rv);
            if (op == ">>") res = ((int)lv) >> ((int)rv);
            if (op == "&") res = ((int)lv) & ((int)rv);
            if (op == "|") res = ((int)lv) | ((int)rv);
            if (op == "^") res = ((int)lv) ^ ((int)rv);
            if (op == "<") res = lv < rv;
            if (op == "<=") res = lv <= rv;
            if (op == ">") res = lv > rv;
            if (op == ">=") res = lv >= rv;
            delete ret;
            ret = new value(NUMBER, to_string(res));
        }
    }
    if (op == "&&" || op == "||") {
        auto ls = new script(new AST(now->at(0)), filename, this);
        ls->init(systemJsonContent);
        auto rs = new script(new AST(now->at(1)), filename, this);
        rs->init(systemJsonContent);
        auto l = ls->run(line);
        auto r = rs->run(line);
        bool lv = isTrue(l->valueOf());
        bool rv = isTrue(r->valueOf());
        int res;
        if (op == "&&") {
            res = lv && rv;
        }
        if (op == "||") {
            res = lv || rv;
        }
        delete ret;
        ret = new value(to_string(res));
    }
    if (
            op == "*=" || op == "/=" || op == "%="
            || op == "+=" || op == "-="
            || op == "&=" || op == "|=" || op == "^="
            || op == "<<=" || op == ">>="
            ) {
        string name = now->at(0)->value();
        auto s = new script(new AST(now->at(1)), filename, this);
        s->init(systemJsonContent);
        auto v = s->run(line);
        auto nameValue = scope->of(name);
        if (nameValue == nullptr) {
            Throw(line, "NameError: name '" + name + "' not defined");
            return;;
        }
        if (nameValue->valueOf().typeOf() != NUMBER || v->typeOf() != NUMBER) {
            if (nameValue->valueOf().typeOf() == STRING && op == "+=" && v->typeOf() == STRING) {
                string nameV = nameValue->valueOf().valueOf();
                string addV = v->valueOf();
                // 去除引号
                nameV.erase(nameV.begin());
                nameV.erase(nameV.end() - 1);
                addV.erase(addV.begin());
                addV.erase(addV.end() - 1);
                string res = "\"" + nameV + addV + "\"";
                delete ret;
                ret = new value(STRING, res);
                scope->set(name, ret);
            } else {
                Throw(line, "TypeError: Unable to run expression because name '" + name +
                            "' must be number and right value be number");
            }
            return;
        }
        double l = atof(nameValue->valueOf().valueOf().c_str());
        double r = atof(v->valueOf().c_str());
        double res = 0;
        if (op == "*=") res = l * r;
        if (op == "/=") res = l / r;
        if (op == "%=") res = ((int) l) % ((int) r);
        if (op == "+=") res = l + r;
        if (op == "-=") res = l - r;
        if (op == "|=") res = ((int) l) | ((int) r);
        if (op == "&=") res = ((int) l) & ((int) r);
        if (op == "^=") res = ((int) l) ^ ((int) r);
        if (op == "<<=") res = ((int) l) << ((int) r);
        if (op == ">>=") res = ((int) l) >> ((int) r);
        delete ret;
//        std::cout << l << ", " << r << ", " << res << std::endl;
        ret = new value(NUMBER, to_string(res));
        scope->set(name, ret);
    }
    if (op == "call") {
        string name = now->at(0)->value();
        std::cout << "call " << name << std::endl;
        std::cout << "get doc " << (name + "Doc") << std::endl;
//        std::cout << "libname: " << systemJson["defines"]["libs"]["BerryMathGlobal"].asString() << std::endl;
        auto fun = new function(name, systemJson["defines"]["libs"]["BerryMathGlobal"].asString(), true);
        DocFunction doc = (DocFunction) dlsym(library(systemJson["defines"]["libs"]["BerryMathGlobal"].asString()), ("_" + name + "Doc").c_str());
        std::cout << "fun: " << doc << std::endl;
        std::vector<value*> values;
        for (int i = 1; i < now->size(); i++) {// 0项是函数名
            auto s = new script(new AST(now->at(i)), filename, this);
            s->init(systemJsonContent);
            auto v = s->run(line);
            values.push_back(v);
            delete s;
        }
        std::map<string, BerryMath::value*> valuesHash;
//        std::vector<string> docs = doc();
//        std::cout << "doc: " << doc << std::endl;
        for (int i = 0; i < values.size(); i++) {
            string argvName("a" + to_string(i));
            if (doc) {
//                if (docs.size() > i) {
//                    argvName = docs[i];
//                }
            }
//            std::cout << values[i]->valueOf() << std::endl;
            valuesHash[argvName] = values[i];
//            std::cout << argvName << std::endl;
        }
        ret = fun->run(this, values, valuesHash);
    }
}
void BerryMath::script::Throw(long line, string message) {
    if (filename.empty()) std::cout << RED << message << " at line " << (line + 1) << ". " << RESET << std::endl;
    else std::cout << RED << message << " at line " << (line + 1) << " in file '" << filename << "'. " << RESET << std::endl;
}

void BerryMath::script::note(string message) {
    std::cout << BLACK << "\033[47mnote:" << RESET << " " << message << ". " << std::endl;
}

void BerryMath::script::init(string json) {
//    std::cout << "code: $" << code << "$ at line " << __LINE__ << std::endl;
    Json::Reader reader;
    systemJsonContent = json;
    string c(code);
    if (!reader.parse(json, systemJson, false)) {// 解析Json
        Throw(-1, "SystemError: Parse system.json failed.");
    }
    code = c;
//    std::cout << "code: $" << code << "$ at line " << __LINE__ << std::endl;
}

void BerryMath::script::finish() {
    for (auto iter = libraries.begin(); iter != libraries.end(); iter++) {
        dlclose(iter->second);
    }
    libraries.clear();
}
void* BerryMath::script::library(string lib) {
    auto iter = libraries.find(lib);
    if (iter == libraries.end()) {
        return nullptr;
    }
    return iter->second;
}