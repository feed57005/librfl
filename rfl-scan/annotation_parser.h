#ifndef __RFL_SCAN_ANNOTATION_PARSER_H__
#define __RFL_SCAN_ANNOTATION_PARSER_H__

#include "rfl/reflected.h"

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringMap.h"

#include <string>

namespace rfl {
namespace scan {

using namespace llvm;

class AnnotationParser {
public:
  static std::unique_ptr<AnnotationParser> loadFromFile(
      StringRef file_path,
      std::string &error_msg);
  static std::unique_ptr<AnnotationParser> loadFromBuffer(
      StringRef annotation_string,
      std::string &error_msg);

  StringRef GetValue(StringRef key);
private:
  AnnotationParser(std::unique_ptr<MemoryBuffer> buffer);

  bool Parse(std::string &err_msg);

private:
  enum TokenKind {
    kNone_Token = 0,
    kAssign_Token,
    kComma_Token,
    kInt_Token,
    kHexInt_Token,
    kFloat_Token,
    kSymbol_Token,
    kString_Token,
    kCurlyBracketLeft_Token,
    kCurlyBracketRight_Token,
  };

  struct Token {
    TokenKind kind_;
    StringRef range_;
  };
  typedef std::vector<Token> Tokens;
  bool TokenizeString();
  bool TokenizeSymbol();
  bool TokenizeNumber();
  bool IsBlankOrBreak(StringRef::iterator position);
  bool Tokenize();

  std::unique_ptr<MemoryBuffer> buffer_;
  SourceMgr source_mgr_;
  StringMap<std::string> value_map_;

  Tokens tokens_;
  StringRef::iterator current_;
  StringRef::iterator end_;
};

} // namespace scan
} // namespace rfl

#endif /* __RFL_SCAN_ANNOTATION_PARSER_H__ */
