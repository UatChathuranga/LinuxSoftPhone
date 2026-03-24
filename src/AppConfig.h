#pragma once
#include <QString>
#include <QObject>

class AppConfig : public QObject {
  Q_OBJECT
public:
  static AppConfig& instance();

  void load();
  void save();

  // Configuration Properties
  int webSocketPort = 11000;
  QString remoteSipPort = "5047";
  bool autoAnswer = false;
  QString apiPort = "11001";
  bool useWss = false;
  QString certPath = "cert.pem";
  QString internalTransferCode = "6";
  QString externalTransferCode = "6";
  QString ivrExternalTransferCode = "9";
  QString inputDevice = "";
  QString outputDevice = "";

private:
  explicit AppConfig(QObject *parent = nullptr);
  QString configPath();
};
