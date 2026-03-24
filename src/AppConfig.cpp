#include "AppConfig.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDir>

AppConfig& AppConfig::instance() {
  static AppConfig instance;
  return instance;
}

AppConfig::AppConfig(QObject *parent) : QObject(parent) {
  load();
}

QString AppConfig::configPath() {
  return QCoreApplication::applicationDirPath() + "/config.json";
}

void AppConfig::load() {
  QFile file(configPath());
  if (!file.exists()) {
    save(); // Create with defaults
    return;
  }

  if (file.open(QIODevice::ReadOnly)) {
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject obj = doc.object();

    if (obj.contains("WebSocketPort")) webSocketPort = obj["WebSocketPort"].toInt();
    if (obj.contains("RemoteSipPort")) remoteSipPort = obj["RemoteSipPort"].toString();
    if (obj.contains("AutoAnswer")) autoAnswer = obj["AutoAnswer"].toBool();
    if (obj.contains("APIPort")) apiPort = obj["APIPort"].toString();
    if (obj.contains("useWss")) useWss = obj["useWss"].toBool();
    if (obj.contains("certPath")) certPath = obj["certPath"].toString();
    if (obj.contains("internalTransferCode")) internalTransferCode = obj["internalTransferCode"].toString();
    if (obj.contains("externalTransferCode")) externalTransferCode = obj["externalTransferCode"].toString();
    if (obj.contains("IVRernalTransferCode")) ivrExternalTransferCode = obj["IVRernalTransferCode"].toString();
    if (obj.contains("InputDevice")) inputDevice = obj["InputDevice"].toString();
    if (obj.contains("OutputDevice")) outputDevice = obj["OutputDevice"].toString();
    
    file.close();
  }
}

void AppConfig::save() {
  QJsonObject obj;
  obj["WebSocketPort"] = webSocketPort;
  obj["RemoteSipPort"] = remoteSipPort;
  obj["AutoAnswer"] = autoAnswer;
  obj["APIPort"] = apiPort;
  obj["useWss"] = useWss;
  obj["certPath"] = certPath;
  obj["internalTransferCode"] = internalTransferCode;
  obj["externalTransferCode"] = externalTransferCode;
  obj["IVRernalTransferCode"] = ivrExternalTransferCode;
  obj["InputDevice"] = inputDevice;
  obj["OutputDevice"] = outputDevice;

  QJsonDocument doc(obj);
  QFile file(configPath());
  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  }
}
