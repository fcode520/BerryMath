#include <iostream>
#include <sstream>
#include <BerryMath.h>
#include <vector>
#include <map>
#include "library.h"
using std::vector;
using std::map;
using BM::Object;
using BM::Scope;

Object* print(BM::Scope* scope, vector<Object*> unknowns) {
    auto sp = scope->get("sp")->value()->toString(true, false);
    if (unknowns.size() > 0) {
        UL i = 0;
        for (; i < unknowns.size() - 1; i++) {
            std::cout << unknowns[i]->toString(true, false) << sp;
        }
        std::cout << unknowns[i]->toString(true, false) << scope->get("end")->value()->toString(true, false);
    }
    return new BM::Undefined;
}
Object* input(BM::Scope* scope, vector<Object*> unknowns) {
    auto sp = scope->get("sp")->value()->toString(true, false);
    if (unknowns.size() > 0) {
        UL i = 0;
        for (; i < unknowns.size() - 1; i++) {
            std::cout << unknowns[i]->toString(true, false) << sp;
        }
        std::cout << unknowns[i]->toString(true, false);
    }
    string t;
    std::cin >> t;
    return new BM::String(t);
}
Object* number(BM::Scope* scope, vector<Object*> unknowns) {
    auto value = scope->get("value")->value();
    if (value->type() == BM::NUMBER) return value->copy();
    if (value->type() == BM::NULL_ || value->type() == BM::UNDEFINED || value->type() == BM::NATIVE_FUNCTION || value->type() == BM::FUNCTION || value->type() == BM::OBJECT) return new BM::Number(1);
    std::stringstream ss;
    auto v = (BM::String*) value;
    ss << v->value();
    double t;
    ss >> t;
    return new BM::Number(t);
}
Object* String(BM::Scope* scope, vector<Object*> unknowns) {
    return new BM::String(scope->get("value")->value()->toString(false, false));
}

BM::Object* initModule() {
    // 创建exports
    auto exports = new BM::Object;

    // 设置模块分区
    exports->set("io", new BM::Object);
    exports->set("type", new BM::Object);

    // 为io分区添加函数
    auto printP = new BM::NativeFunction(print);// 创建拓展函数
    auto inputP = new BM::NativeFunction(input);// 创建拓展函数
    // 为printP设置默认值
    printP->defaultValue("end", new BM::String("\n"));
    printP->defaultValue("sp", new BM::String(" "));
    // 为inputP设置默认值
    inputP->defaultValue("sp", new BM::String(" "));
    exports->get("io")->set("print", printP);
    exports->get("io")->set("input", inputP);

    // 为type分区添加函数
    auto numberP = new BM::NativeFunction(number);// 创建拓展函数
    auto stringP = new BM::NativeFunction(String);// 创建拓展函数
    numberP->addDesc("value");
    stringP->addDesc("value");
    exports->get("type")->set("number", numberP);
    exports->get("type")->set("string", stringP);
    return exports;// 导出模块
}