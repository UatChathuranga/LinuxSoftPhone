#include "CommandServer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>

CommandServer::CommandServer(int port, Agent *agent, QObject *parent)
    : QObject(parent), m_agent(agent) {
  m_server = new QWebSocketServer("SoftphoneController",
                                  QWebSocketServer::NonSecureMode, this);
  if (m_server->listen(QHostAddress::AnyIPv4, port)) {
    qDebug() << "WS Server listening on port" << port;
    connect(m_server, &QWebSocketServer::newConnection, this,
            &CommandServer::onNewConnection);
    connect(m_agent, &Agent::registrationStateChangedSignal, this,
            &CommandServer::handleRegistrationStateChanged);
  }
}

void CommandServer::onNewConnection() {
  QWebSocket *pSocket = m_server->nextPendingConnection();
  connect(pSocket, &QWebSocket::textMessageReceived, this,
          &CommandServer::processTextMessage);
  connect(pSocket, &QWebSocket::disconnected, this,
          &CommandServer::socketDisconnected);
  m_clients << pSocket;
  qDebug() << "WS: New connection established";
}

void CommandServer::processTextMessage(QString message) {
  QStringList parts = message.split("|");
  if (parts.isEmpty())
    return;

  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());

  // 1. Handshake: veery|Initiate|veery|othr
  if (parts.size() >= 2 && parts[1] == "Initiate") {
    m_currentSessionKey = QUuid::createUuid().toString(QUuid::WithoutBraces);

    QJsonObject handshake;
    handshake["veery_api_key"] = m_currentSessionKey;
    handshake["veery_command"] = "Handshake";

    QJsonDocument doc(handshake);
    if (pClient) {
      pClient->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
    qDebug() << "WS: Key Generated:" << m_currentSessionKey;
    return;
  }

  // Check session key for subsequent commands
  if (parts.size() < 2 || parts[0] != m_currentSessionKey) {
    if (pClient)
      pClient->sendTextMessage("ERROR|INVALID_KEY");
    return;
  }

  QString command = parts[1];

  if (command == "Registor" && parts.size() >= 4) {
    qDebug() << "--------- Registor command received --------- ";
    qDebug() << "message" << message;
    // API_KEY|Registor|123456789|username-password@host
    QString rawData = parts[3];
    int lastAtIndex = rawData.lastIndexOf('@');
    if (lastAtIndex != -1) {
      QString host = rawData.mid(lastAtIndex + 1);
      QString userInfo = rawData.left(lastAtIndex);
      int firstDashIndex = userInfo.indexOf('-');
      if (firstDashIndex != -1) {
        QString user = userInfo.left(firstDashIndex);
        QString passwordPart = userInfo.mid(firstDashIndex + 1);
        QString suffixToRemove = "-" + user;
        if (passwordPart.endsWith(suffixToRemove)) {
          passwordPart = passwordPart.left(passwordPart.length() -
                                           suffixToRemove.length());
        }

        qDebug() << "WS: Registration -> User:" << user << "Host:" << host;
        m_agent->setCredentials(user.toStdString(), passwordPart.toStdString(),
                                host.toStdString());
        m_agent->Register();
      }
    }
  } else if (command == "Unregistor") {
    m_agent->Unregister();
  } else if (command == "MakeCall" && parts.size() >= 3) {
    m_agent->MakeCall(parts[2].toStdString());
  } else if (command == "AnswerCall") {
    m_agent->answerCall();
  } else if (command == "EndCall") {
    m_agent->endCall();
  } else if (command == "HoldCall") {
    m_agent->toggleHold();
  } else if (command == "TransferCall" && parts.size() >= 3) {
    m_agent->transferCall(parts[2].toStdString());
  } else if (command == "EtlCall") {
    m_agent->endTransferCall();
  } else if (command == "ConfCall") {
    m_agent->conferenceCall();
  } else if (command == "MuteCall") {
    m_agent->toggleMute();
  }
}

void CommandServer::socketDisconnected() {
  QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
  if (pClient) {
    m_clients.removeAll(pClient);
    pClient->deleteLater();
    qDebug() << "WS: Connection closed";
  }
}
void CommandServer::handleRegistrationStateChanged(int state, const QString& message) {
    QJsonObject regInfo;
    regInfo["veery_api_key"] = m_currentSessionKey;
    
    // state mapping:
    // 2: LinphoneRegistrationOk -> "Initialized"
    // 3: LinphoneRegistrationCleared -> "Failed"
    // 4: LinphoneRegistrationFailed -> "Failed"
    
    if (state == 2) { // LinphoneRegistrationOk
        regInfo["veery_command"] = "Initialized";
    } else if (state == 3 || state == 4) { // Cleared or Failed
        regInfo["veery_command"] = "Failed";
    } else {
        // Other states (Progressing, None, etc.) - maybe don't send anything or send as is
        return;
    }

    QJsonDocument doc(regInfo);
    QString jsonResponse = doc.toJson(QJsonDocument::Compact);
    
    for (QWebSocket *pClient : m_clients) {
        if (pClient && pClient->isValid()) {
            pClient->sendTextMessage(jsonResponse);
        }
    }
    qDebug() << "WS: Sent registration status update:" << jsonResponse;
}
