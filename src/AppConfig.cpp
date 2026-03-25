#include "AppConfig.h"
#include <QString>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

AppConfig& AppConfig::instance() {
  static AppConfig instance;
  return instance;
}

AppConfig::AppConfig(QObject *parent) : QObject(parent) {
  load();
}

QString AppConfig::configPath() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  qDebug() << "AppConfig::configPath: Base path is" << path;
  if (path.isEmpty()) {
    qDebug() << "AppConfig::configPath: WARNING: Standard path is empty, falling back to local";
    return QCoreApplication::applicationDirPath() + "/config.json";
  }
  QDir dir(path);
  if (!dir.exists()) {
    if (dir.mkpath(".")) {
      qDebug() << "AppConfig::configPath: Created directory" << path;
    } else {
      qDebug() << "AppConfig::configPath: FAILED to create directory" << path;
    }
  }
  return path + "/config.json";
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
    if (obj.contains("RingtonePath")) ringtonePath = obj["RingtonePath"].toString();
    
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
  obj["RingtonePath"] = ringtonePath;

  QJsonDocument doc(obj);
  QString path = configPath();
  qDebug() << "AppConfig::save: Saving to" << path;
  QFile file(path);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
  } else {
    qDebug() << "AppConfig::save: Failed to open file for writing:" << file.errorString();
  }
}
