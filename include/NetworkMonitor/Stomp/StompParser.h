#pragma once

#include "StompFrame.h"

#include <cctype>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>

namespace Networking::Stomp {

constexpr const char ASCII_NULL = '\0';
constexpr const char ASCII_NEWLINE = '\0';
constexpr const char ASCII_CARRIAGE_RETURN = '\0';
constexpr const char EOL = '\0';
constexpr const char OCTET = '\0';


enum class TokenType {
  kUndefinedToken = 0,

  kEOF,
  kEOL,
  kNull,

  kStompCommand,
  kStompHeaderKey,
  kStompHeaderValue,
  kStompBody,

  kSizeOfEnum
};

class TokenTypeToStringBimap : public EnumToStringBimap<TokenType> {
  using Base = EnumToStringBimap<TokenType>;

public:
  TokenTypeToStringBimap()
      : Base(
          {{TokenType::kUndefinedToken, "UndefinedToken"},
           {TokenType::kEOF, "EOFToken"},
           {TokenType::kEOL, "EOLToken"},
           {TokenType::kNull, "NullToken"},
           {TokenType::kStompCommand, "StompCommandToken"},
           {TokenType::kStompHeaderKey, "StompHeaderKey"},
           {TokenType::kStompHeaderValue, "StompHeaderValue"},
           {TokenType::kStompBody, "StompBody"},
           {TokenType::kSizeOfEnum, "UndefinedToken"}})
  {
  }
};

class StompToken {
public:
  StompToken() = delete;

  StompToken(TokenType type, TokenValue value)
      : m_type{type},
        m_value{std::move(value)}
  {
  }

  StompToken(const StompToken &) = default;
  auto operator=(const StompToken &) -> StompToken & = default;

  StompToken(StompToken &&) = default;
  auto operator=(StompToken &&) -> StompToken & = default;

  ~StompToken() = default;

  [[nodiscard]] auto GetType() const -> TokenType { return m_type; }
  [[nodiscard]] auto GetValue() const -> TokenValue { return m_value; }

private:
  TokenType m_type{TokenType::kUndefinedToken};
  TokenValue m_value{std::monostate{}};
};

// std::ostream& operator<<(std::ostream& os, const StompToken& stompToken) {
//	TokenTypeToStringBimap tokenTypeMapping;
//   os << "StompToken{" << tokenTypeMapping.ToString(stompToken.GetType()) <<
//   "," << stompToken.GetValue() << "}";
//	return os;
// }

class StompTokenizer {
public:
  using Character = char;

  explicit StompTokenizer(std::string frame)
      : m_frame{std::move(frame)}
  {
    // m_tokenTypeToString = TokenTypeToStringBimap::Create();
    // nextChar();
  }

  auto GetToken() -> StompToken
  {
    StompToken result{TokenType::kUndefinedToken, std::monostate{}};

    while (nextChar() != m_eof) {
      if (isCurrentChar(':')) { //
        // Reading the header value
        nextChar();

        // For testing purposes don't support numerical values for now
        const std::size_t startIdentifier{m_currentPos};
        std::size_t endIdentifier{startIdentifier};
        while (peek()[0] != '\n') {
          nextChar();
          endIdentifier++;
        }

        const auto identifier{
          m_frame.substr(startIdentifier, endIdentifier - startIdentifier)};

        // std::cout << "identifier=" << identifier << " is a header value\n";

        result = {TokenType::kStompHeaderValue, identifier};
        return result;
      }
      else if (std::isalpha(m_currentChar[0]) != 0) {
        // Reading the STOMP command or header key
        const std::size_t startIdentifier{m_currentPos};
        std::size_t endIdentifier{startIdentifier};
        while (true) {
          if (
            peek()[0] == ':' || peek()[0] == '\n' ||
            (std::isalpha(m_currentChar[0]) == 0 && !isCurrentChar('-'))) {
            break;
          }

          nextChar();
          endIdentifier++;
        }

        const auto identifier{
          m_frame.substr(startIdentifier, endIdentifier - startIdentifier + 1)};

        if (isStompCommand(identifier)) {
          result = {TokenType::kStompCommand, identifier};
          // std::cout << "identifier=" << identifier << " is a stomp
          // command\n";
          return result;
        }
        else if (isHeaderKey(identifier)) {
          result = {TokenType::kStompHeaderKey, identifier};
          // std::cout << "identifier=" << identifier << " is a header key\n";
          return result;
        }
        else {
          // std::cout << "not command and header key\n";
        }
      }
      else if (isCurrentChar('\n') && peek()[0] == '\n') {
        const std::size_t startIdentifier{m_currentPos};
        std::size_t endIdentifier{startIdentifier};
        while (nextChar()[0] != '\0') {
          endIdentifier++;
        }

        const auto identifier{
          m_frame.substr(startIdentifier, endIdentifier - startIdentifier)};

        // std::cout << "identifier=" << identifier << " is a stomp body\n";

        result = {TokenType::kStompBody, identifier};
        return result;
      }
    }

    if (result.GetType() != TokenType::kUndefinedToken) {
      // std::cout << "returning type: "
      //           << m_tokenTypeToString.ToString(result.GetType()).value()
      //           << ", " << std::get<std::string>(result.GetValue())
      //           << std::endl;
    }

    return result;
  }

  auto ReadASCIIConstant(char constant) -> std::pair<bool, Character> {}

  auto ReadEOL() -> bool {}

private:
  auto nextChar() -> std::string
  {
    if (m_currentPos != std::string::npos) {
      m_currentPos++;
    }
    else {
      m_currentPos = 0;
    }

    if (m_currentPos >= m_frame.size()) {
      m_currentChar = m_eof;
    }
    else {
      m_currentChar = m_frame[m_currentPos];
    }

    return m_currentChar;
  }

  auto peek() -> std::string
  {
    if (
      m_currentPos == std::string::npos || m_currentPos + 1 >= m_frame.size()) {
      return m_eof;
    }

    return std::string{m_frame[m_currentPos + 1]};
  }

  auto isStompCommand(const std::string &cmd) -> bool
  {
    return m_stompCommandToString.ToEnum(cmd) != std::nullopt;
  }

  auto isHeaderKey(const std::string &key) -> bool
  {
    return key == "content-type";
  }

  auto isCurrentChar(char symbol) const -> bool
  {
    if (
      m_currentChar.empty() || m_currentChar.size() >= 2 ||
      m_currentPos == std::string::npos || m_currentPos + 1 >= m_frame.size()) {
      return false;
    }

    return m_currentChar[0] == symbol;
  }

  const std::string m_eof{"EOF"};

  const TokenTypeToStringBimap m_tokenTypeToString;
  const StompCommmandToStringBimap m_stompCommandToString;

  std::string m_frame{};
  std::string m_currentChar{};
  std::size_t m_currentPos{std::string::npos};
};

class StompParser {
public:
  explicit StompParser(std::string frame)
      : m_frameText{std::move(frame)},
        m_lexer(frame)
  {
  }

  auto Parse() -> StompFrame
  {
    std::cout << "[STOMP]: Parsing STOMP Frame" << std::endl;
    StompFrame result{};

    // Initialize next and current tokens.
    nextToken();
    nextToken();

    // if (checkToken(TokenType::kStompCommand)) {
    auto cmd{parseStompCommand()};
    result.m_command = cmd;

		// }
    auto headers{parseStompHeaders()};
		result.m_headers = std::move(headers);

    return {};
  }

private:
  auto parseStompCommand() -> StompCommand
  {
    std::cout << "[STOMP]: Parsing STOMP Command" << std::endl;
    if (checkToken(TokenType::kStompCommand)) {
      match(TokenType::kStompCommand);
    }
    else {
      throw std::logic_error("expected stomp command token");
    }

    return {};
  };

  auto parseStompHeaders() -> std::vector<StompHeader>
  {
		parseStompHeader();
    return std::vector<StompHeader>{};
  };

  auto parseStompHeader() -> StompHeader {
    StompHeader result{};

		match(TokenType::kStompHeaderKey);
		match(TokenType::kStompHeaderValue);

    result.m_header = m_tokenTypeToString.ToString(m_currentToken.GetType()).value();
    result.m_value = m_currentToken.GetValue();

		return result;
	};

  auto parseStompBodyStart() -> std::pair<bool, StompTokenizer::Character>
  {
    return m_lexer.ReadASCIIConstant(ASCII_CARRIAGE_RETURN);
  };

  auto parseStompBody() -> StompBody
  {
    const auto bodyStartsWithNewline{parseStompBodyStart()};
    std::string body{};
    const auto bodyEndsWithNull{parseStompBodyEnd()};

    return StompBody{.m_body{std::move(body)}};
  };

  auto parseStompBodyEnd() -> std::pair<bool, StompTokenizer::Character>
  {
    return m_lexer.ReadASCIIConstant(ASCII_NULL);
  };

  [[nodiscard]] auto checkToken(const TokenType type) const -> bool
  {
    return m_currentToken.GetType() == type;
  }

  [[nodiscard]] auto checkPeek(const TokenType type) const -> bool
  {
    return m_nextToken.GetType() == type;
  }

  auto match(const TokenType type) -> void
  {
    if (!checkToken(type)) {
      // TODO: Use custom ParserException!
      throw std::logic_error(
        "STOMP parser expected this token but got that token");
    }

    nextToken();
  }

  auto nextToken() -> void
  {
    m_currentToken = m_nextToken;
    m_nextToken = m_lexer.GetToken();
  }

  auto newLine() -> void
  {
    std::cout << "[STOMP]: Parsing newline" << std::endl;
    match(TokenType::kEOL);
  }

	TokenTypeToStringBimap m_tokenTypeToString;
  std::string m_frameText;
  StompTokenizer m_lexer;
  StompToken m_currentToken{TokenType::kUndefinedToken, std::monostate{}};
  StompToken m_nextToken{TokenType::kUndefinedToken, std::monostate{}};
};

} // namespace Networking::Stomp
