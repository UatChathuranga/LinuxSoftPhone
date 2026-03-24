#pragma once
#include "Agent.h"
#include <QObject>
#include <QStringList>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>
#include <QUuid>

class CommandServer : public QObject {
  Q_OBJECT
public:
  explicit CommandServer(int port, Agent *agent,
                         QObject *parent = nullptr);

private slots:
  void onNewConnection();
  void processTextMessage(QString message);
  void socketDisconnected();
  void handleRegistrationStateChanged(int state, const QString& message);

private:
  QWebSocketServer *m_server;
  Agent *m_agent;
  QList<QWebSocket *> m_clients;
  QString m_currentSessionKey;
};