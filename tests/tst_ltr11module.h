#pragma once

#include "moduleruntimeconfiguration.h"
#include <QObject>

class tst_ltr11module : public QObject
{
  Q_OBJECT
public:
  tst_ltr11module();
  ~tst_ltr11module() override;

private slots:
  void test_Ltr11Module_base();
  void test_Ltr11Module_without_print();
  void test_LCardDataSource_base();
};

inline qds::ModuleRuntimeConfiguration createModuleRuntimeConfiguration() {
  using namespace qds;
  ModuleRuntimeConfiguration cfg
    {
      .module = {
        .id = {0},
        .serial = "",
        .crate = {0},
        .slot = 1,
        .type = ModuleType::LTR11
      },
      .crate = {
        .id = {0},
        .serial = "",//"5T776728",
        .type = CrateType::LTR_EU_16_1,
        .host = "127.0.0.1",
        .port = 11111
      },
      .configuration = {
        .configurationId = {1},
        .module = {0},
        .settings = QJsonDocument::fromJson(R"({})").object()
      },
      .tags = {
       {TagId{1}, ModuleId{0}, ChannelId{0}, QJsonDocument::fromJson(R"({"mode":0,"range":0})").object()},
       {TagId{5}, ModuleId{0}, ChannelId{1}, QJsonDocument::fromJson(R"({"mode":2,"range":1})").object()},
       {TagId{7}, ModuleId{0}, ChannelId{3}, QJsonDocument::fromJson(R"({"mode":1,"range":2})").object()},
       {TagId{9}, ModuleId{0}, ChannelId{24}, QJsonDocument::fromJson(R"({"mode":1,"range":3})").object()},
      }
    };

  return cfg;
}
