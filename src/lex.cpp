#include <cctype>
#include "btype.h"
#include "lex.h"

BerryMath::lex::lexToken BerryMath::lex::get() {
    str = "";
    token = BerryMath::INIT_TOKEN;
    while (true) {
        if (parseIndex >= program.length()) break;
        if (program[parseIndex] == ';') {
            if (end) {
                token = BerryMath::END_TOKEN;
            } else {
                end = true;
            }
            break;
        }
        if (program[parseIndex] == ' ') {
            if (token == BerryMath::INIT_TOKEN) {
                parseIndex++;
                continue;
            } else {
                parseIndex++;
                break;
            }
        }
        if (token == BerryMath::NUMBER_TOKEN && BerryMath::isSymbol(program[parseIndex])) break;
        if (token == BerryMath::DOT_TOKEN && BerryMath::isSymbol(program[parseIndex])) {
            whileToEnd();
            break;
        }
        if (BerryMath::isSymbol(program[parseIndex])) {
            if (str == "if") {
                token = BerryMath::IF_TOKEN;
                break;
            } else if (str == "elif") {
                token = BerryMath::ELIF_TOKEN;
                break;
            } else if (str == "else") {
                token = BerryMath::ELSE_TOKEN;
                break;
            } else if (str == "while") {
                token = BerryMath::WHILE_TOKEN;
                break;
            } else if (str == "do") {
                token = BerryMath::DO_TOKEN;
                break;
            } else if (str == "for") {
                token = BerryMath::FOR_TOKEN;
                break;
            } else if (str == "let") {
                token = BerryMath::CREATE_VARIABLE_TOKEN;
                break;
            } else if (str == "function") {
                token = BerryMath::CREATE_FUNCTION_TOKEN;
                break;
            } else {
                if (str != "") {
                    token = BerryMath::UNKNOWN_TOKEN;
                    break;
                }
            }
        }
        str += program[parseIndex];
//        std::cout << program[parseIndex];
        if (isdigit(program[parseIndex])) {
            if (token == BerryMath::INIT_TOKEN) {
                token = BerryMath::NUMBER_TOKEN;
            } else if (token == BerryMath::DOT_TOKEN) {
                token = BerryMath::NUMBER_TOKEN;
            } else if (token == BerryMath::NUMBER_TOKEN) {
            } else {
                whileToEnd();
                break;
            }
        }
        if (str == "=") {
            token = BerryMath::SET_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::EQUAL_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "(") {
            token = BerryMath::BRACKETS_LEFT_TOKEN;
            parseIndex++;
            break;
        } else if (str == ")") {
            token = BerryMath::BRACKETS_RIGHT_TOKEN;
            parseIndex++;
            break;
        } else if (str == "[") {
            token = BerryMath::MIDDLE_BRACKETS_LEFT_TOKEN;
            parseIndex++;
            break;
        } else if (str == "]") {
            token = BerryMath::MIDDLE_BRACKETS_RIGHT_TOKEN;
            parseIndex++;
            break;
        } else if (str == "{") {
            token = BerryMath::BIG_BRACKETS_LEFT_TOKEN;
            parseIndex++;
            break;
        } else if (str == "}") {
            token = BerryMath::BIG_BRACKETS_RIGHT_TOKEN;
            parseIndex++;
            break;
        } else if (str == ">") {
            token = BerryMath::BIGGER_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::BIGGER_EQUAL_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "<") {
            token = BerryMath::SMALLER_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::SMALLER_EQUAL_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "!") {
            token = BerryMath::LOGICAL_NOT_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::NOT_EQUAL_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "&") {
            token = BerryMath::MATH_AND_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '&') {
                token = BerryMath::LOGICAL_AND_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "|") {
            token = BerryMath::MATH_OR_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '&') {
                token = BerryMath::LOGICAL_OR_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "~") {
            token = BerryMath::MATH_NOT_TOKEN;
            parseIndex++;
            break;
        } else if (str == "^") {
            token = BerryMath::MATH_XOR_TOKEN;
            parseIndex++;
            break;
        } else if (str == "+") {
            token = BerryMath::ADD_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '+') {
                token = BerryMath::SELF_ADD_TOKEN;
                parseIndex++;
            }
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::ADD_TO_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "-") {
            token = BerryMath::SUB_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '-') {
                token = BerryMath::SELF_SUB_TOKEN;
                parseIndex++;
            }
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::SUB_TO_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "*") {
            token = BerryMath::MUL_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::MUL_TO_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "/") {
            token = BerryMath::DIV_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '/') {
                token = BerryMath::NOTE_TOKEN;
                parseIndex++;
                while (parseIndex < program.length() && program[parseIndex] != '\n') {
                    parseIndex++;
                }
                parseIndex++;
            }
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::DIV_TO_TOKEN;
                parseIndex++;
            }
            break;
        } else if (str == "%") {
            token = BerryMath::MOD_TOKEN;
            parseIndex++;
            if (parseIndex < program.length() && program[parseIndex] == '=') {
                token = BerryMath::MOD_TO_TOKEN;
                parseIndex++;
            }
            break;
        }
        parseIndex++;
    }
//    std::cout << std::endl << token << std::endl;
    return value();
}
void BerryMath::lex::whileToEnd() {
    token = BerryMath::BAD_TOKEN;
    for ( ; parseIndex < program.length() ; parseIndex++) {
        if (program[parseIndex] == ';') {
            break;
        }
        str += program[parseIndex];
    }
}