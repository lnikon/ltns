#include <NetworkMonitor/Stomp/StompParser.h>

#include <optional>
#include <iostream>
#include <stdexcept>

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

using namespace Networking::Stomp;
using namespace std::string_literals;

BOOST_AUTO_TEST_SUITE(StompParserTestSuite);

BOOST_AUTO_TEST_CASE(TokenTypeMapToString)
{
  TokenTypeToStringBimap tokenMap;

  BOOST_CHECK(
    tokenMap.ToString(TokenType::kUndefinedToken) == "UndefinedToken");
  BOOST_CHECK(tokenMap.ToString(TokenType::kEOL) == "EOLToken");
  BOOST_CHECK(tokenMap.ToString(TokenType::kEOF) == "EOFToken");
  BOOST_CHECK(
    tokenMap.ToString(TokenType::kStompCommand) == "StompCommandToken");
  BOOST_CHECK(tokenMap.ToString(TokenType::kSizeOfEnum) == std::nullopt);
}

BOOST_AUTO_TEST_CASE(TokenTypeMapToEnum)
{
  TokenTypeToStringBimap tokenMap;

  BOOST_CHECK(tokenMap.ToEnum("UndefinedToken") == TokenType::kUndefinedToken);
  BOOST_CHECK(tokenMap.ToEnum("EOLToken") == TokenType::kEOL);
  BOOST_CHECK(tokenMap.ToEnum("EOFToken") == TokenType::kEOF);
  BOOST_CHECK(tokenMap.ToEnum("StompCommandToken") == TokenType::kStompCommand);
  BOOST_CHECK(tokenMap.ToEnum("UndefinedToken") == TokenType::kUndefinedToken);
}

BOOST_AUTO_TEST_CASE(LexerProducesCorrectTokens)
{
  std::string frame{"STOMP\n"
                    "content-type:applicationjson\n\n"
                    "login:vagag\n"
                    "passcode:vagagord\n"
                    "\0"s};

  StompTokenizer lexer(frame);
	auto token{lexer.GetToken()};
	BOOST_CHECK(token.GetType() == TokenType::kStompCommand);
	token = lexer.GetToken();
	BOOST_CHECK(token.GetType() == TokenType::kStompHeaderKey);
	token = lexer.GetToken();
	BOOST_CHECK(token.GetType() == TokenType::kStompHeaderValue);
	token = lexer.GetToken();
	BOOST_CHECK(token.GetType() == TokenType::kStompBody);
}

BOOST_AUTO_TEST_CASE(ParserMathesTokens)
{
  std::string frameText{"STOMP\n"
                    "content-type:applicationjson\n\n"
                    "login:vagag\n"
                    "passcode:vagagord\n"
                    "\0"s};

	StompParser parser(frameText);
	auto frame{parser.Parse()};
}

BOOST_AUTO_TEST_SUITE_END()

