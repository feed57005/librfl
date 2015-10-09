// Copyright (c) 2015 Pavel Novy. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "rfl-scan/annotation_parser.h"
#include "llvm/Support/raw_ostream.h"
#include <system_error>

namespace rfl {
namespace scan {

namespace {

std::string unescape(const std::string &s) {
  std::string res;
  std::string::const_iterator it = s.begin();
  while (it != s.end()) {
    char c = *it++;
    if (c == '\\' && it != s.end()) {
      switch (*it++) {
        case '\\':
          c = '\\';
          break;
        case 'n':
          c = '\n';
          break;
        case 't':
          c = '\t';
          break;
        case '"':
          c = '"';
          break;
        // all other escapes
        default:
          // invalid escape sequence - skip it. alternatively you can copy it as
          // is, throw an exception...
          continue;
      }
    }
    res += c;
  }

  return res;
}

}  // namespace

std::unique_ptr<AnnotationParser> AnnotationParser::loadFromFile(
    StringRef file_path,
    std::string &error_msg) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
      MemoryBuffer::getFile(file_path);
  if (std::error_code Result = buffer.getError()) {
    error_msg = "Error while opening JSON annotation: " + Result.message();
    return nullptr;
  }
  std::unique_ptr<AnnotationParser> anno(
      new AnnotationParser(std::move(*buffer)));
  if (!anno->Parse(error_msg))
    return nullptr;
  return anno;
}

std::unique_ptr<AnnotationParser> AnnotationParser::loadFromBuffer(
    StringRef annotation_string,
    std::string &error_msg) {
  std::unique_ptr<MemoryBuffer> buffer(
      MemoryBuffer::getMemBuffer(annotation_string));
  std::unique_ptr<AnnotationParser> anno(
      new AnnotationParser(std::move(buffer)));
  if (!anno->Parse(error_msg))
    return nullptr;
  return anno;
}

AnnotationParser::AnnotationParser(std::unique_ptr<MemoryBuffer> buffer)
    : buffer_(std::move(buffer)) {
}

bool AnnotationParser::Parse(std::string &err_msg) {
  if (!Tokenize() || tokens_.size() < 4) {
    return false;
  }
  if (tokens_[0].kind_ != kSymbol_Token ||
      tokens_[1].kind_ != kAssign_Token ||
      tokens_[2].kind_ != kCurlyBracketLeft_Token ||
      tokens_[tokens_.size() -1].kind_ != kCurlyBracketRight_Token) {
    return false;
  }

  kind_ = tokens_[0].range_.str();
  size_t i = 3;
  StringRef key;
  while (i < tokens_.size() -1) {
    Token const &t = tokens_[i];
    if (t.kind_ == kSymbol_Token) {
      key = t.range_;
    } else if (t.kind_ != kAssign_Token && t.kind_ != kComma_Token) {
      //outs() << key << ": " << t.range_ << " " << t.kind_ << "\n";
      if (t.kind_ == kString_Token) {
        value_map_[key] = unescape(t.range_.str());
      } else {
        value_map_[key] = t.range_.str();
      }
    }
    i++;
  }
  return true;
}

StringRef AnnotationParser::GetValue(StringRef key) {
  StringMap<std::string>::const_iterator it = value_map_.find(key);
  if (it == value_map_.end())
    return StringRef();
  std::string const &value = it->getValue();
  return StringRef(value);
}

// Returns whether a character at 'position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
static bool WasEscaped(StringRef::iterator first,
                       StringRef::iterator position) {
  assert(position - 1 >= first);
  StringRef::iterator it = position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  while (it >= first && *it == '\\')
    --it;
  // (position - 1 - it) now contains the number of '\'s before the current
  // position. itf it is odd, the character at 'position' was escaped.
  return (position - 1 - it) % 2 == 1;
}

bool AnnotationParser::TokenizeString() {
  StringRef::iterator start = current_;
  do {
    ++current_;
    while (current_ != end_ && *current_ != '"')
      ++current_;
    // Repeat until the previous character was not a '\' or was an escaped
    // backslash.
  } while (current_ != end_ && *(current_ - 1) == '\\' &&
           WasEscaped(start + 1, current_));
  if (current_ == end_) {
    errs() << "Expected quote at end of scalar\n";
    return false;
  }


  Token t;
  t.kind_ = kString_Token;
  t.range_ = StringRef(start+1, current_ - start -1);
  tokens_.push_back(t);
  if (current_ == end_) {
    errs() << "Already end,Expected quote at end of scalar\n";
    return false;
  }

  current_++; // Skip ending quote.
  return true;
}

bool AnnotationParser::IsBlankOrBreak(StringRef::iterator position) {
  if (position == end_) {
    return false;
  }
  if (*position == ' ' || *position == '\t' || *position == '\r' ||
      *position == '\n')
    return true;
  return false;
}

bool AnnotationParser::TokenizeSymbol() {
  StringRef::iterator start = current_;
  while (current_ != end_ && !IsBlankOrBreak(current_) && *current_ != ':' && *current_ != '=' && *current_ != '}') {
    current_++;
  }
  Token t;
  t.kind_ = kSymbol_Token;
  t.range_ = StringRef(start, current_ - start);
  tokens_.push_back(t);
  //errs() << t.range_.str() << "\n";
  return true;
}

bool AnnotationParser::TokenizeNumber() {
  StringRef::iterator start = current_;
  TokenKind kind = kInt_Token;
  while (isxdigit(*current_) || *current_ == '.' || *current_ == 'x' ||
         *current_ == 'X') {
    if (*current_ == 'x' || *current_ == 'X') {
      if (kind != kInt_Token) {
        errs() << "Malformed float number\n";
        return false;
      }
      kind = kHexInt_Token;
    }
    if (*current_ == '.' || *current_ == ',') {
      if (kind != kInt_Token) {
        errs() << "Malformed hex number\n";
        return false;
      }
      kind = kFloat_Token;
    }
    current_++;
  }
  Token t;
  t.kind_ = kind;
  t.range_ = StringRef(start, current_ - start);
  tokens_.push_back(t);
  return true;
}

bool AnnotationParser::Tokenize() {
  current_ = buffer_->getBufferStart();
  end_ = buffer_->getBufferEnd();
  tokens_.clear();
  current_ = buffer_->getBufferStart();
  while (current_ != end_) {
    switch (*current_) {
      case (':'):
      case ('='): {
        Token t;
        t.kind_ = kAssign_Token;
        t.range_ = StringRef(current_, 1);
        tokens_.push_back(t);
        current_++;
        break;
      }
      case (','): {
        Token t;
        t.kind_ = kComma_Token;
        t.range_ = StringRef(current_, 1);
        tokens_.push_back(t);
        current_++;
        break;
      }
      case ('{'): {
        Token t;
        t.kind_ = kCurlyBracketLeft_Token;
        t.range_ = StringRef(current_, 1);
        tokens_.push_back(t);
        current_++;
        break;
      }
      case ('}'): {
        Token t;
        t.kind_ = kCurlyBracketRight_Token;
        t.range_ = StringRef(current_, 1);
        tokens_.push_back(t);
        current_++;
        break;
      }
      case ('"'): {
        TokenizeString();
        break;
      }
      case (' '):
      case ('\t'):
      case ('\n'):
      case ('\r'):
        current_++;
        break;

      case ('_'):
        TokenizeSymbol();
        break;

      case ('.'):
        TokenizeNumber();
        break;

      default: {
        if (isalpha(*current_)) {
          TokenizeSymbol();
        } else if (isdigit(*current_)) {
          TokenizeNumber();
        } else {
          errs() << "Unknown character '" << *current_ << "'\n";
          return false;
        }
        break;
      }
    }
  }
  return true;
}

char const *AnnotationParser::kind() const {
  return kind_.c_str();
}

} // namespace scan
} // namespace rfl
