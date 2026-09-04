#pragma once

#include <QObject>
#include <filesystem>

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

inline std::filesystem::path makeFileName(
  const std::filesystem::path &directory,
  const uint64_t& time)
{
  const std::time_t seconds =
    time / 1'000'000;

  std::tm tm_time{};

#ifdef _WIN32
  gmtime_s(&tm_time, &seconds);
#else
  gmtime_r(&seconds, &tm_time);
#endif

  char buffer[11];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &tm_time);
  return directory / (std::string(buffer) + ".log");
}

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
