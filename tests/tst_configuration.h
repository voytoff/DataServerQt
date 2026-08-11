#ifndef TST_CONFIGURATION_H
#define TST_CONFIGURATION_H

#include <QObject>

class tst_configuration : public QObject
{
  Q_OBJECT
public:
  tst_configuration();
  ~tst_configuration() override;

private slots:
  void test_configuration_parse_module_ids();
  //void test_configuration_modules();
  //  test_configuration_moduleTags
  void test_configuration_moduleChannelCount();
  //  test_configuration_tags
  void test_configuration_signalDefinitions();

};

#endif // TST_CONFIGURATION_H
