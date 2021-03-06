#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

#include "spdlog/spdlog.h"

#include "symbol_table.hpp"
#include "token.hpp"
#include "token_table.hpp"

#undef yyFlexLexer
#define yyFlexLexer yyMizFlexLexer
#include <FlexLexer.h>

#include "miz_flex_lexer.hpp"

using mizcore::MizFlexLexer;

MizFlexLexer::MizFlexLexer(std::istream* in,
                           std::shared_ptr<SymbolTable> symbol_table)
  : yyMizFlexLexer(in)
  , symbol_table_(std::move(symbol_table))
  , token_array_(std::make_shared<TokenTable>())
  , line_number_(1)
  , column_number_(1)
  , is_in_environ_section_(false)
  , is_in_vocabulary_section_(false)
{}

size_t
MizFlexLexer::ScanSymbol()
{
    Symbol* symbol = symbol_table_->QueryLongestMatchSymbol(yytext);
    if (symbol != nullptr) {
        Token* token = new SymbolToken(line_number_, column_number_, symbol);
        token_array_->AddToken(token);
        size_t length = token->GetText().size();
        column_number_ += length;

        if (token->GetText() == ";" && is_in_vocabulary_section_) {
            is_in_vocabulary_section_ = false;
        }
        return length;
    }

    return 0;
}

size_t
MizFlexLexer::ScanIdentifier()
{
    Token* token = new IdentifierToken(line_number_, column_number_, yytext);
    token_array_->AddToken(token);
    column_number_ += yyleng;
    return yyleng;
}

size_t
MizFlexLexer::ScanKeyword(KEYWORD_TYPE type)
{
    Token* token = new KeywordToken(line_number_, column_number_, type);
    token_array_->AddToken(token);
    column_number_ += yyleng;

    if (type == KEYWORD_TYPE::ENVIRON) {
        is_in_environ_section_ = true;
    } else if (type == KEYWORD_TYPE::VOCABULARIES) {
        if (is_in_environ_section_) {
            is_in_vocabulary_section_ = true;
        }
    } else if (type == KEYWORD_TYPE::BEGIN_) {
        is_in_environ_section_ = false;
        is_in_vocabulary_section_ = false;
        symbol_table_->BuildQueryMap();
    }

    return yyleng;
}

size_t
MizFlexLexer::ScanNumeral()
{
    Token* token = new NumeralToken(line_number_, column_number_, yytext);
    assert(token);
    token_array_->AddToken(token);
    column_number_ += yyleng;
    return yyleng;
}

size_t
MizFlexLexer::ScanFileName()
{
    if (is_in_environ_section_) {
        Token* token = new IdentifierToken(
          line_number_, column_number_, yytext, IDENTIFIER_TYPE::FILENAME);
        assert(token);
        token_array_->AddToken(token);
        column_number_ += yyleng;

        if (is_in_vocabulary_section_) {
            symbol_table_->AddValidFileName(token->GetText());
        }
        return yyleng;
    }
    return 0;
}

size_t
MizFlexLexer::ScanComment(COMMENT_TYPE type)
{
    Token* token = new CommentToken(line_number_, column_number_, yytext, type);
    assert(token);
    token_array_->AddToken(token);
    column_number_ += yyleng;
    return yyleng;
}

size_t
MizFlexLexer::ScanUnknown()
{
    spdlog::error(
      "[Error] Unknown token found: [{},{}]", line_number_, column_number_);
    Token* token = new UnknownToken(line_number_, column_number_, yytext);
    assert(token);
    token_array_->AddToken(token);
    column_number_ += yyleng;
    return yyleng;
}
