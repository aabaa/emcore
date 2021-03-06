#pragma once

#include <fstream>
#include <memory>

namespace mizcore {

class SymbolTable;
class TokenTable;
class MizFlexLexer;

class MizFileHandler
{
  public:
    MizFileHandler(std::istream* in,
                   const std::shared_ptr<SymbolTable>& symbol_table);
    virtual ~MizFileHandler() = default;

    MizFileHandler(const MizFileHandler&) = delete;
    MizFileHandler(MizFileHandler&&) = delete;
    MizFileHandler& operator=(const MizFileHandler&) = delete;
    MizFileHandler& operator=(MizFileHandler&&) = delete;

    int yylex();
    std::shared_ptr<TokenTable> GetTokenTable() const;

  private:
    std::shared_ptr<MizFlexLexer> miz_flex_lexer_;
};

} // namespace mizcore
