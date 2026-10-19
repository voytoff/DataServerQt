#pragma once

#include "ilogger.h"
#include "loglevel.h"

#include <QObject>
#include <string_view>

namespace qds
{

class GuiLogger : public QObject, public qds::ILogger
{
  Q_OBJECT

public:
  explicit GuiLogger(
    qds::ILogger& fileLogger,
    QObject* parent = nullptr);

  bool debug(std::string_view message) override;
  bool info(std::string_view message) override;
  bool warning(std::string_view message) override;
  bool error(std::string_view message) override;

signals:
  void messageReceived(
    qds::LogLevel level,
    QString message);

private:
  bool write(
    qds::LogLevel level,
    std::string_view message);

  qds::ILogger& m_fileLogger;
};

}
