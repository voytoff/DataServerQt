#pragma once

#include <QObject>
#include <charconv>

class tst_logger : public QObject
{
  Q_OBJECT
public:
  tst_logger();
  ~tst_logger() override;

private slots:
  void test_logger_base();
  void test_logger_levels();
  void test_logger_midnight();
  void test_logger_multithread();

};

// Функция для поиска и извлечения числа после определенного ключа
inline std::optional<int> extract_value(std::string_view log, std::string_view key) {
  // Находим позицию ключа (например, "thread=")
  auto pos = log.find(key);
  if (pos == std::string_view::npos) {
    return std::nullopt;
  }

  // Сдвигаем указатель на начало самого числа
  auto number_start = log.data() + pos + key.length();
  auto number_end = log.data() + log.length();

  int value = 0;
  // std::from_chars — самый быстрый способ парсинга в C++
  auto [ptr, ec] = std::from_chars(number_start, number_end, value);

  if (ec == std::errc{}) {
    return value;
  }

  return std::nullopt;
}
