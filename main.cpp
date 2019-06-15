#include <iostream>
#include <BerryMath.h>

void TEST1() {
    auto object = new BM::Object;
    auto child = new BM::Object;
    object->insert("number", new BM::Number(0));
    object->insert("string", new BM::String("Hello world!"));
    object->insert("p", new BM::Null);
    object->insert("v", new BM::Undefined);
    object->insert("child", child);
    child->insert("Text", new BM::String("I keep my ideas"));
    std::cout << (*object) << std::endl;
    std::cout << (*object)["p"] << std::endl;
    delete object;
}
void TEST2() {
    string v("let\na = 0;a++;");
    BM::Lexer lexer(v);
    auto t = lexer.get();
    std::cout << t.s << std::endl;
    while (t.t != BM::Lexer::PROGRAM_END) {
        t = lexer.get();
        std::cout << t.s << std::endl;
    }
}
void TEST3() {
    auto r = new BM::ast::node("", 0);
    r->insert("call", 1);
    r->insert("fun", 1);
    r->insert("arg", 1);
    (*r)[-1].insert("1",1);
    std::cout << "finish" << std::endl;
}

int main() {
//    TEST1();
    TEST2();
//    TEST3();
    return 0;
}