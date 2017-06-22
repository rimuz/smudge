/*
 *      Copyright 2016-2017 Riccardo Musso
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 *
 *      File parse/Tokenizer.cpp
 *
*/

#include <string>
#include <iostream>

#include "error/error.h"
#include "parse/Tokenizer.h"
#include "runtime/gc.h"
#include "utils/String.h"

namespace sm{
    namespace parse{
        //TODO: commentare intero file!
        /*
         * TODO: fix: se un commento contiene caratteri estesi, allora il conteggio
         * dei caratteri sarà sballato!! Ipotizzo sia lo stesso per i literal di stringhe...
        */

        bool isAsciiValidLetter(ascii_t ascii) noexcept{
            return  ascii == '_' || ascii == '$'
                    ||  (ascii >= 'A' && ascii <= 'Z')
                    ||  (ascii >= 'a' && ascii <= 'z');
        }

        bool isAsciiValidDigit(ascii_t ascii) noexcept{
            return ascii >= '0' && ascii <= '9';
        }

        bool isAsciiValidOperator(ascii_t ascii) noexcept{
            return ascii == '%' || ascii == '&' || ascii == '!'
                    || ascii == '#' || (ascii >= '{' && ascii <= '~')
                    || (ascii >= '(' && ascii <= '/')
                    || (ascii >= ':' && ascii <= '?')
                    || (ascii >= '[' && ascii <= '^');
        }

        void submit(TokenVec_t& tokens, TokenizerStates& states, enum_t newToken) noexcept{
            if(tokens.empty()){
                return;
            }

            Token& tok = tokens.back();

            if((states.double_quote || states.single_quote) && states.back_slash){
                states.back_slash = false;
                switch(states.curr_ch){
                    /*
                     * Escape sequences \", \' and \\ are handled below in for cycle inside tokenize(string_t&)
                     *
                    */

                    case 'a':{
                        tok.content.push_back('\a');
                        break;
                    }

                    case 'b':{
                        tok.content.push_back('\b');
                        break;
                    }

                    case 'f':{
                        tok.content.push_back('\f');
                        break;
                    }

                    case 'n':{
                        tok.content.push_back('\n');
                        break;
                    }

                    case 'r':{
                        tok.content.push_back('\r');
                        break;
                    }

                    case 't':{
                        tok.content.push_back('\t');
                        break;
                    }

                    case 'v':{
                        tok.content.push_back('\v');
                        break;
                    }

                    case '0': case '1': case '2':
                    case '3': case '4': case '5':
                    case '6': case '7': {
                        size_t out_of_range = *states.idx + 3;
                        char ch = states.str->operator[](*states.idx);
                        std::string octal = { ch };
                        for(;*states.idx + 1 < out_of_range; (*states.idx)++, states.file_char++){
                            ch = states.str->operator[](*states.idx + 1);
                            if(ch >= '0' && ch <= '7'){
                                octal.push_back(ch);
                            } else {
                                break;
                            }
                        }
                        unsigned char newChar = static_cast<unsigned char>(std::stoi(octal, nullptr, 8) & 0xFF);
                        tok.content.push_back(newChar);
                        break;
                    }

                    case 'x':{
                        size_t out_of_range = ++(*states.idx) + 2;
                        states.file_char++;
                        char ch = states.str->operator[](*states.idx);
                        std::string hex { ch };
                        for(;*states.idx + 1 < out_of_range; (*states.idx)++, states.file_char++){
                            ch = states.str->operator[](*states.idx + 1);
                            if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')){
                                hex.push_back(ch);
                            }
                        }
                        unsigned char newChar = static_cast<unsigned char>(std::stoi(hex, nullptr, 16) & 0xFF);
                        tok.content.push_back(newChar);
                        break;
                    }

                    case 'u':{
                        size_t out_of_range = ++(*states.idx) + 4;
                        states.file_char++;
                        char ch = states.str->operator[](*states.idx);
                        std::string unihex = { ch };
                        for(;*states.idx + 1 < out_of_range; (*states.idx)++, states.file_char++){
                            ch = states.str->operator[](*states.idx + 1);
                            if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')){
                                unihex.push_back(ch);
                            } else {
                                break;
                            }
                        }
                        unsigned codePoint = static_cast<unsigned>(std::stoul(unihex, nullptr, 16));
                        unicode_t ucode = uByCodepoint(codePoint);
                        if(ucode == UTF8_ERROR){
                            states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                "Unicode codepoint too big");
                        }
                        String::uAppend(tok.content, ucode);
                        break;
                    }

                    case 'U':{
                        size_t out_of_range = ++(*states.idx) + 8;
                        states.file_char++;
                        char ch = states.str->operator[](*states.idx);
                        std::string unihex = { ch };
                        for(;*states.idx + 1 < out_of_range; (*states.idx)++, states.file_char++){
                            ch = states.str->operator[](*states.idx + 1);
                            if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')){
                                unihex.push_back(ch);
                            } else {
                                break;
                            }
                        }
                        unsigned codePoint = static_cast<unsigned>(std::stoul(unihex, nullptr, 16));
                        unicode_t ucode = uByCodepoint(codePoint);
                        if(ucode == UTF8_ERROR){
                            states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                "Unicode codepoint too big");
                        }
                        String::uAppend(tok.content, ucode);
                        break;
                    }

                    default:{
                        states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                            "invalid escape character");
                    }
                }
                return;
            }

            if(tok.type == newToken || newToken == TT_STRING){
                return;
            }

            if(tok.type == TT_UNPARSED_NUMBER && newToken != TT_DOT && newToken != TT_TEXT){
                if(tok.content.find_first_of('.') == std::string::npos){ // if it doesn't contain dots:
                    int base = 0;
                    if(tok.content.size() > 2 && tok.content[0] == '0'
                            && (tok.content[1] == 'b' || tok.content[1] == 'B')){
                        tok.content.erase(0, 2);
                        base = 2;
                    }
                    tok.type = TT_INTEGER;
                    try {
                        tok.i = std::stol(tok.content, nullptr, base);
                    } catch(std::out_of_range){
                        states.rt->sources.msg(error::ERROR, tok.source, tok.ln, tok.ch,
                            "integer unsupported by Smudge (too big)");
                    }
                } else {
                    tok.type = TT_FLOAT;
                    try {
                        tok.f = std::stod(tok.content, nullptr);
                    } catch(std::out_of_range){
                        states.rt->sources.msg(error::ERROR, tok.source, tok.ln, tok.ch,
                            "floating-point unsupported by Smudge (too big)");
                    }
                }
                tok.content.clear();
            }

            if((tok.type == TT_STRING)
                    && (states.single_quote || states.double_quote)){
                tok.content.push_back(states.curr_ch);
            } else if(tok.type == TT_TEXT){
                for(size_t i = 0; i < keywordsLen; i++){
                    if(keywords[i] == tok.content){
                        tok.type = TT_KEYWORDS_START + i;
                        tok.content.clear();
                    }
                }
            } else if(tok.type == TT_UNPARSED_OPERATOR){
                string_t& operStr = tok.content;
                size_t size;
                bool foundOperator;
                TokenVec_t operators;

                while((size = operStr.size())){
                    foundOperator = false;

                    if(size >= 3){
                        string_t lastThreeChars = operStr.substr(size-3, size);
                        for(unsigned i = 0; i < threeCharsOperatorsLen; i++){
                            if(lastThreeChars == threeCharsOperators[i]){
                                states.nullToken = false;
                                Token newToken;
                                newToken.ln = tok.ln;
                                newToken.ch = tok.ch + size-3;
                                newToken.source = tok.source;
                                newToken.type = lengthOrderedOperatorIds[
                                    singleCharOperatorsLen + twoCharsOperatorsLen + i];
                                newToken.i = operatorPriorities[newToken.type - TT_OPERATORS_START];
                                operators.push_back(newToken);
                                foundOperator = true;
                                break;
                            }
                        }

                        if(foundOperator){
                            operStr.erase(operStr.end()-3, operStr.end());
                            continue;
                        }
                    }

                    if(size >= 2){
                        string_t lastTwoChars = operStr.substr(size-2, size);
                        for(unsigned i = 0; i < twoCharsOperatorsLen; i++){
                            if(lastTwoChars == twoCharsOperators[i]){
                                states.nullToken = false;
                                Token newToken;
                                newToken.ln = tok.ln;
                                newToken.ch = tok.ch + size-2;
                                newToken.source = tok.source;
                                newToken.type = lengthOrderedOperatorIds[
                                    singleCharOperatorsLen + i];
                                newToken.i = operatorPriorities[newToken.type - TT_OPERATORS_START];
                                operators.push_back(newToken);
                                foundOperator = true;
                                break;
                            }
                        }

                        if(foundOperator){
                            operStr.erase(operStr.end()-2, operStr.end());
                            continue;
                        }
                    }

                    char lastChar = operStr.back();
                    for(unsigned i = 0; i < singleCharOperatorsLen; i++){
                        if(lastChar == singleCharOperators[i]){
                            states.nullToken = false;
                            Token newToken;
                            newToken.ln = tok.ln;
                            newToken.ch = tok.ch + size-1; // note: size CAN be greater than 1 here!
                            newToken.source = tok.source;
                            newToken.type = lengthOrderedOperatorIds[i];
                            newToken.i = operatorPriorities[newToken.type - TT_OPERATORS_START];
                            operators.push_back(newToken);
                            foundOperator = true;
                            break;
                        }
                    }

                    if(!foundOperator){
                        states.rt->sources.msg(error::ERROR, tok.source, tok.ln, tok.ch,
                            std::string("can't parse operator: \'") + tok.content + "\'.");
                    }

                    operStr.pop_back();
                }

                tokens.pop_back(); // removes TT_UNPARSED_OPERATOR token
                tokens.insert(tokens.end(), operators.rbegin(), operators.rend());
            }

            if(tokens.size() > 1 && (tok.type < TT_OPERATORS_START || tok.type > TT_OPERATORS_END)){
                Token* backPtr;
                for(TokenVec_t::iterator it = tokens.end() -2; it >= tokens.begin(); --it){
                    backPtr = &*it;
                    if(it > tokens.begin()){
                        Token& before = *(it-1);
                        if(((before.type < TT_OPERATORS_START || before.type > TT_OPERATORS_END)
                                    || before.type == TT_ROUND_CLOSE
                                    || before.type == TT_SQUARE_CLOSE
                                    || before.type == TT_CURLY_CLOSE)){
                            return;
                        }
                    }

                    switch(it->type){
                        case TT_POST_DEC:
                            backPtr->type = TT_PRE_DEC;
                            break;

                        case TT_POST_INC:
                            backPtr->type = TT_PRE_INC;
                            break;

                        case TT_MINUS:
                            if(it > tokens.begin()){
                                Token& before = *(it-1);
                                if(before.type == TT_POST_INC
                                        || before.type == TT_POST_DEC){
                                    return;
                                }
                            }

                            backPtr->type = TT_PRE_MINUS;
                            break;

                        case TT_PLUS:
                            if(it > tokens.begin()){
                                Token& before = *(it-1);
                                if(before.type == TT_POST_INC
                                        || before.type == TT_POST_DEC){
                                    return;
                                }
                            }

                            backPtr->type = TT_PRE_PLUS;
                            break;

                        case TT_ROUND_OPEN:
                        case TT_SQUARE_OPEN:
                        case TT_CURLY_OPEN:
                            break;

                        case TT_COMPL:
                        case TT_NOT:
                            continue;

                        default:
                            return;
                    }

                    backPtr->i = operatorPriorities[backPtr->type - TT_OPERATORS_START];
                }
            }
        }

        TokenVec_t tokenize(runtime::Runtime_t* rt, unsigned source) noexcept{
            std::string& code = *rt->sources.getSource(source)->code;
            TokenVec_t tokens;
            TokenizerStates states;
            Token* back = nullptr;
            unsigned ch_idx = 0;

            states.rt = rt;
            states.idx = &ch_idx;
            states.str = &code;
            states.back_ptr = &back;
            states.source = source;
            states.file_char = 0;
            states.file_line = 1;

            for(; ch_idx < code.size(); ch_idx++){
                const ascii_t& ch = code[ch_idx];
                states.file_char++;

                states.curr_ch = ch;
                back = tokens.empty() ? nullptr : &tokens.back();

                if(ch == '\n'){
                    submit(tokens, states, TT_USELESS);
                    states.file_line++;
                    states.file_char = 0;
                    if(states.double_quote
                        || states.single_quote)
                            continue;
                    states.nullToken = true;
                    continue;
                } else if(static_cast<unsigned>(ch) <= static_cast<unsigned>(' ')){
                    submit(tokens, states, TT_USELESS);
                    if(states.double_quote || states.single_quote)
                        continue;
                    states.nullToken = true;
                    continue;
                } else if(ch == '@'){
                    //TODO: preprocessor directives
                    submit(tokens, states, TT_UNPARSED_DIRECTIVE);
                    if(states.double_quote || states.single_quote)
                        continue;
                    states.nullToken = false;
                    Token newToken;
                    newToken.ln = states.file_line;
                    newToken.ch = states.file_char;
                    newToken.source = states.source;
                    newToken.type = TT_UNPARSED_DIRECTIVE;
                    tokens.push_back(newToken);
                } else if(ch == '\''){
                    if(!states.back_slash){
                        if(states.double_quote){
                            back->content.push_back('\'');
                            continue;
                        }

                        if(!states.single_quote){
                            submit(tokens, states, TT_USELESS);
                            submit(tokens, states, TT_STRING);
                            if(!tokens.empty()){
                                Token* backRef = &tokens.back();

                                if(backRef->type == TT_STRING){
                                    states.single_quote = !states.single_quote;
                                    continue;
                                }
                            }

                            states.nullToken = false;
                            Token newToken;
                            newToken.ln = states.file_line;
                            newToken.ch = states.file_char;
                            newToken.source = states.source;
                            newToken.type = TT_STRING;
                            tokens.push_back(newToken);
                        } else {
                            submit(tokens, states, TT_STRING);
                        }

                        states.single_quote = !states.single_quote;
                    } else {
                        back->content.push_back('\'');
                        states.back_slash = false;
                    }
                } else if(ch == '\"'){
                    if(!states.back_slash){
                        if(states.single_quote){
                            back->content.push_back('\"');
                            continue;
                        }

                        if(!states.double_quote){
                            submit(tokens, states, TT_USELESS);
                            submit(tokens, states, TT_STRING);

                            if(!tokens.empty()){
                                Token* backRef = &tokens.back();

                                if(backRef->type == TT_STRING){
                                    states.double_quote = !states.double_quote;
                                    continue;
                                }
                            }

                            states.nullToken = false;
                            Token newToken;
                            newToken.ln = states.file_line;
                            newToken.ch = states.file_char;
                            newToken.source = states.source;
                            newToken.type = TT_STRING;
                            tokens.push_back(newToken);
                        } else {
                            submit(tokens, states, TT_STRING);
                        }

                        states.double_quote = !states.double_quote;
                    } else {
                        back->content.push_back('\"');
                        states.back_slash = false;
                    }
                } else if(ch == '\\'){
                    if(states.double_quote || states.single_quote){
                        if(!states.back_slash){
                            states.back_slash = true;
                        } else {
                            back->content.push_back('\\');
                            states.back_slash = false;
                        }
                    }
                } else if(ch == '.'){
                    submit(tokens, states, TT_DOT);
                    if(states.double_quote || states.single_quote)
                        continue;
                    if(back && back->type == TT_UNPARSED_NUMBER){
                        if(back->i){ // if it is floating point, octal, hexadecimal, binary or has open exp.
                            states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                "\'.\' not expected here");
                        } else {
                            back->i |= UNH_FLOATING_POINT;
                            back->content.push_back('.');
                        }
                    } else {
                        states.nullToken = false;
                        Token newToken;
                        newToken.ln = states.file_line;
                        newToken.ch = states.file_char;
                        newToken.source = states.source;
                        newToken.type = TT_DOT;
                        tokens.push_back(newToken);
                    }
                } else if(isAsciiValidDigit(ch)){
                    submit(tokens, states, TT_UNPARSED_NUMBER);
                    if(states.double_quote || states.single_quote)
                        continue;
                    if(back && (back->type == TT_UNPARSED_NUMBER || back->type == TT_TEXT) && !states.nullToken){
                        if(back->type == TT_UNPARSED_NUMBER){
                            if(back->content.length() == 1 && back->content[0] == '0'){
                                back->i |= UNH_OCTAL;
                            }
                            if(back->i & UNH_OPEN_EXPONENT){
                                back->i ^= UNH_OPEN_EXPONENT;
                            }
                            switch(ch){
                                case '0': case '1': {
                                    back->content.push_back(ch);
                                    break;
                                }

                                case '2': case '3': case '4':
                                case '5': case '6': case '7':{
                                    if(back->i & UNH_BINARY){
                                        states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                            "expected binary digit after '0b'");
                                    } else {
                                        back->content.push_back(ch);
                                    }
                                    break;
                                }

                                case '8': case '9':{
                                    if(back->i & UNH_BINARY){
                                        states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                            "expected binary digit after '0b'");
                                    } else if (back->i & UNH_OCTAL){
                                        states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                            "expected octal digit after '0'");
                                    } else {
                                        back->content.push_back(ch);
                                    }
                                    break;
                                }

                                default:{
                                    states.rt->sources.msg(error::FATAL_ERROR, states.source, states.file_line, states.file_char,
                                        "something gone wrong (err 1).");
                                }
                            }
                        } else {
                            back->content.push_back(ch);
                        }
                    } else {
                        states.nullToken = false;
                        Token newToken;
                        newToken.ln = states.file_line;
                        newToken.ch = states.file_char;
                        newToken.source = states.source;
                        newToken.type = TT_UNPARSED_NUMBER;
                        newToken.content.push_back(ch);
                        if(back && back->type == TT_DOT){
                            newToken.content.insert(0, "0.");
                            newToken.i |= UNH_FLOATING_POINT;
                            tokens.pop_back();
                        }
                        tokens.push_back(newToken);
                        continue;
                    }
                } else if(isAsciiValidLetter(ch)){
                    submit(tokens, states, TT_TEXT);
                    if(states.double_quote || states.single_quote)
                        continue;
                    if(back && back->type == TT_UNPARSED_NUMBER && !states.nullToken){
                        switch(ch){
                            case 'a': case 'c': case 'd':
                            case 'f': case 'A': case 'C':
                            case 'D': case 'F': {
                                if(back->i & UNH_HEXADECIMAL){
                                    back->content.push_back(ch);
                                } else {
                                    states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                        "unexpected hex digit in a non-hex literal");
                                }
                                break;
                            }

                            case 'B':
                            case 'b':{
                                if(back->i & UNH_HEXADECIMAL){
                                    back->content.push_back(ch);
                                } else {
                                    if(back->content.length() == 1 && back->content[0] == '0'){
                                        back->content.push_back(ch);
                                        back->i |= UNH_BINARY;
                                    } else {
                                        states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                            "unexpected hex digit in a non-hex literal");
                                    }
                                }
                                break;
                            }

                            case 'e':
                            case 'E':{
                                if(back->i & UNH_FLOATING_POINT){
                                    if(back->i & UNH_OPEN_EXPONENT || back->i & UNH_ALREDY_OPEN_EXPONENT){
                                        states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                            "cannot perform exponentiation of exponentiation");
                                    } else {
                                        back->content.push_back(ch);
                                        back->i |= UNH_OPEN_EXPONENT;
                                        back->i |= UNH_ALREDY_OPEN_EXPONENT;
                                    }
                                } else if(back->i & UNH_HEXADECIMAL){
                                    back->content.push_back(ch);
                                } else {
                                    states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                        "unexpected hex digit in a non-hex number literal");
                                }
                                break;
                            }

                            case 'X':
                            case 'x':{
                                if(back->content.length() == 1 && back->content[0] == '0'){
                                    back->content.push_back(ch);
                                    back->i |= UNH_HEXADECIMAL;
                                    break;
                                } else {
                                    states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                        "unexpected character in number literal");
                                }
                                break;
                            }

                            default:{
                                states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                                    "unexpected character in number literal");
                                break;
                            }
                        }
                    } else if(back && back->type == TT_TEXT && !states.nullToken){
                        back->content.push_back(ch);
                    } else {
                        states.nullToken = false;
                        Token newToken;
                        newToken.ln = states.file_line;
                        newToken.ch = states.file_char;
                        newToken.source = states.source;
                        newToken.type = TT_TEXT;
                        newToken.content.push_back(ch);
                        tokens.push_back(newToken);
                    }
                } else if(isAsciiValidOperator(ch)){
                    if(!states.single_quote && !states.double_quote){
                        if(ch == '/' && code.size() != ch_idx+1){
                            if(code[ch_idx+1] == '*'){
                                submit(tokens, states, TT_USELESS);
                                ascii_t curr = 0;
                                size_t cmt_start = ch_idx +1;
                                ch_idx += 2;
                                while(true){
                                    if(ch_idx == code.size()){
                                        states.rt->sources.msg(error::ERROR, states.source,
                                            states.file_line, states.file_char,
                                                "multi line comment never closed");
                                    }

                                    curr = code[ch_idx];
                                    if(curr == '\n'){
                                        states.file_line++;
                                        states.file_char = 0;
                                    } else if(curr == '/'){
                                        if(code[ch_idx-1] == '*' && ch_idx-1 != cmt_start){
                                            states.nullToken = true;
                                            break;
                                        }
                                    }

                                    ch_idx++;
                                    states.file_char++;
                                }
                                continue;
                            } else if(code[ch_idx+1] == '/'){
                                submit(tokens, states, TT_USELESS);
                                while(true){
                                    if(ch_idx == code.size() || code[ch_idx] == '\n'){
                                        states.file_line++;
                                        states.file_char = 0;
                                        break;
                                    }

                                    ch_idx++;
                                    states.file_char++; // useless!
                                }
                                continue;
                            }
                        } else if(ch == '#'){
                            submit(tokens, states, TT_USELESS);
                            while(true){
                                if(ch_idx == code.size() || code[ch_idx] == '\n'){
                                    states.file_line++;
                                    states.file_char = 0;
                                    break;
                                }

                                ch_idx++;
                                states.file_char++; // useless!
                            }
                            continue;
                        }
                    }

                    if(!states.double_quote && !states.single_quote
                            && (ch == '+' || ch == '-') && !states.nullToken
                            && back && back->type == TT_UNPARSED_NUMBER
                            && (back->i & UNH_OPEN_EXPONENT)){
                        back->content.push_back(ch);
                        back->i ^= UNH_OPEN_EXPONENT;
                        continue;
                    }

                    submit(tokens, states, TT_UNPARSED_OPERATOR);
                    if(states.double_quote || states.single_quote)
                        continue;
                    if(back && back->type == TT_UNPARSED_OPERATOR){
                        back->content.push_back(ch);
                    } else {
                        states.nullToken = false;
                        states.nullToken = false;
                        Token newToken;
                        newToken.ln = states.file_line;
                        newToken.ch = states.file_char;
                        newToken.source = states.source;
                        newToken.type = TT_UNPARSED_OPERATOR;
                        newToken.content.push_back(ch);
                        tokens.push_back(newToken);
                    }
                } else {
                    submit(tokens, states, TT_USELESS);
                    if(states.double_quote || states.single_quote)
                        continue;
                    states.rt->sources.msg(error::ERROR, states.source, states.file_line, states.file_char,
                        "illegal character in code");
                }
            }
            submit(tokens, states, TT_UNPARSED);

            if(states.single_quote || states.double_quote){
                states.rt->sources.msg(error::ERROR, states.source, tokens.back().ln, tokens.back().ch,
                    "string literal never closed");
            }

            if(states.back_slash){
                states.rt->sources.msg(error::ERROR, states.source, tokens.back().ln, tokens.back().ch,
                    "escape character reaches eof");
            }
            return tokens;
        }

        std::string representation(const Token& token) noexcept{
            return representations[token.type];
        }

        namespace test{
            std::string to_string(const Token& token) noexcept{
                return std::string(tokenTypes[token.type]) + ": '" + token.content
                    + "' [" + std::to_string(token.i) + ", " + std::to_string(token.f)
                    + "] (" + std::to_string(token.ln) + ":" + std::to_string(token.ch) +  ")";
            }
        }

        static_assert (arraySize(test::tokenTypes) == TT_MAX+1, "size of test::tokenTypes (string array) must be equal to size of TokenType (enum)");
    }
}
