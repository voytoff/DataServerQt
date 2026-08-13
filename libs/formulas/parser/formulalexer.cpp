#include "formulalexer.h"

#include <cctype>

namespace qds
{

FormulaLexer::FormulaLexer(std::string_view text)
  : m_text(text)
{
}

FormulaToken FormulaLexer::next()
{
  // пропускаем пробелы
  while (m_position < m_text.size() &&
         std::isspace(
           static_cast<unsigned char>(m_text[m_position])))
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

  const std::size_t start = m_position;
  const char ch = m_text[m_position];

  // Identifier
  if (std::isalpha(
        static_cast<unsigned char>(ch)) ||
      ch == '_')
  {
    ++m_position;

    while (m_position < m_text.size())
    {
      const char c = m_text[m_position];

      if (!std::isalnum(
            static_cast<unsigned char>(c)) &&
          c != '_')
      {
        break;
      }

      ++m_position;
    }

    return {
      .type = FormulaTokenType::Identifier,
      .text = std::string(
        m_text.substr(start, m_position - start))
    };
  }

  // Number
  if (std::isdigit(
        static_cast<unsigned char>(ch)) ||
      ch == '.')
  {
    bool hasDot = false;

    if (ch == '.')
      hasDot = true;

    ++m_position;

    while (m_position < m_text.size())
    {
      const char c = m_text[m_position];

      if (std::isdigit(
            static_cast<unsigned char>(c)))
      {
        ++m_position;
        continue;
      }

      if (c == '.' && !hasDot)
      {
        hasDot = true;
        ++m_position;
        continue;
      }

      break;
    }

    const std::string text(
      m_text.substr(start, m_position - start));

    return {
      .type = FormulaTokenType::Number,
      .text = text,
      .number = std::stod(text)
    };
  }

  // Односимвольные токены
  ++m_position;

  switch (ch)
  {
  case '+':
    return {
      .type = FormulaTokenType::Plus,
      .text = "+"
    };

  case '-':
    return {
      .type = FormulaTokenType::Minus,
      .text = "-"
    };

  case '*':
    return {
      .type = FormulaTokenType::Multiply,
      .text = "*"
    };

  case '/':
    return {
      .type = FormulaTokenType::Divide,
      .text = "/"
    };

  case '(':
    return {
      .type = FormulaTokenType::LeftParen,
      .text = "("
    };

  case ')':
    return {
      .type = FormulaTokenType::RightParen,
      .text = ")"
    };

  case ',':
    return {
      .type = FormulaTokenType::Comma,
      .text = ","
    };

  default:
    return {
      .type = FormulaTokenType::Invalid,
      .text = std::string(1, ch)
    };
  }
}

}