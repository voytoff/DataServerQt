#pragma once

#include "ilogger.h"
#include "loglevel.h"

#include <QObject>
#include <string_view>

namespace qds
{

class GuiLogger : public QObject, public ILogger
{
  Q_OBJECT

public:
  explicit GuiLogger(
    ILogger& fileLogger,
    QObject* parent = nullptr);

  bool debug(std::string_view message) override;
  bool info(std::string_view message) override;
  bool warning(std::string_view message) override;
  bool error(std::string_view message) override;

private:
  void write(
    LogLevel level,
    std::string_view message);

  ILogger& m_fileLogger;

signals:
  void messageReceived(
    LogLevel level,
    QString message);

};

}
