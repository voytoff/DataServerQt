#pragma once

#include <QObject>
#include <thread>
#include "ilcardmodule.h"

class tst_hardware : public QObject
{
  Q_OBJECT
public:
  tst_hardware();
  ~tst_hardware() override;

private slots:
  void test_fakeLCardModule_base();
  void test_lCardDataSource();
  void test_lCardDataSource_data_integrity();

  void test_ltr11configurationbuilder();

  void test_acquire_returns_last_frame();
  void test_lCardDataSource_fake_push_archive();

};



class OneBlockLCardModule final
  : public qds::ILCardModule
{
public:
  bool start() noexcept override
  {
    m_running = true;
    return true;
  }

  void stop() noexcept override
  {
    m_running = false;
  }

  std::size_t blockFrameCapacity() const noexcept override
  {
    return 3;
  }

  double frameRate() const noexcept override
  {
    return 1000;
  }

  std::size_t readBlock(
    std::span<double> values) noexcept override
  {
    if (!m_running)
      return 0;

    if (m_read)
    {
      std::this_thread::sleep_for(
        std::chrono::milliseconds(1));

      return 0;
    }

    m_read = true;

    for (std::size_t i = 0; i < 12; ++i)
      values[i] = static_cast<double>(i);

    return 3;
  }

private:
  bool m_running = false;
  bool m_read = false;
};