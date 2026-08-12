#include "formulalexer.h"

#include <cctype>
#include <string>

namespace qds
{

FormulaLexer::FormulaLexer(std::string_view text)
  : m_text(text)
  , m_position(0)
{
}

FormulaToken FormulaLexer::next()
{
  // пропускаем пробелы
  while (m_position < m_text.size() &&
         std::isspace(
           static_cast<unsigned char>(
             m_text[m_position])))
  {
    ++m_position;
  }

  // конец строки
  if (m_position >= m_text.size())
  {
    return {
      .type = FormulaTokenType::End
    };
  }

  const char c = m_text[m_position];

  // операторы
  switch (c)
  {
  case '+':
    ++m_position;
    return {
      .type = FormulaTokenType::Plus,
      .text = "+"
    };

  case '-':
    ++m_position;
    return {
      .type = FormulaTokenType::Minus,
      .text = "-"
    };

  case '*':
    ++m_position;
    return {
      .type = FormulaTokenType::Multiply,
      .text = "*"
    };

  case '/':
    ++m_position;
    return {
      .type = FormulaTokenType::Divide,
      .text = "/"
    };

  case '(':
    ++m_position;
    return {
      .type = FormulaTokenType::LeftParen,
      .text = "("
    };

  case ')':
    ++m_position;
    return {
      .type = FormulaTokenType::RightParen,
      .text = ")"
    };

  default:
    break;
  }

  // число
  if (std::isdigit(
        static_cast<unsigned char>(c)) ||
      c == '.')
  {
    const std::size_t begin = m_position;

    bool hasDot = false;

    while (m_position < m_text.size())
    {
      const char ch = m_text[m_position];

      if (std::isdigit(
            static_cast<unsigned char>(ch)))
      {
        ++m_position;
        continue;
      }

      if (ch == '.' && !hasDot)
      {
        hasDot = true;
        ++m_position;
        continue;
      }

      break;
    }

    const std::string text =
      std::string(
        m_text.substr(
          begin,
          m_position - begin));

    FormulaToken token;

    token.type = FormulaTokenType::Number;
    token.text = text;
    token.number = std::stod(text);

    return token;
  }

  // идентификатор
  if (std::isalpha(
        static_cast<unsigned char>(c)) ||
      c == '_')
  {
    const std::size_t begin = m_position;

    ++m_position;

    while (m_position < m_text.size())
    {
      const char ch = m_text[m_position];

      if (std::isalnum(
            static_cast<unsigned char>(ch)) ||
          ch == '_')
      {
        ++m_position;
        continue;
      }

      break;
    }

    return {
      .type = FormulaTokenType::Identifier,
      .text = std::string(
        m_text.substr(
          begin,
          m_position - begin))
    };
  }

  // неизвестный символ
  ++m_position;

  return {
    .type = FormulaTokenType::Invalid,
    .text = std::string(1, c)
  };
}

}