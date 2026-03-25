#include "CommandServer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <linphone/core.h>
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
    connect(m_agent, &Agent::callStateChangedSignal, this,
            &CommandServer::handleCallStateChanged);
    connect(m_agent, &Agent::incomingCallReceivedSignal, this,
            &CommandServer::handleIncomingCallReceived);
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
    QJsonObject response;
    response["veery_api_key"] = m_currentSessionKey;
    response["veery_command"] = "MakeCall";
    if (pClient) pClient->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
  } else if (command == "AnswerCall") {
    m_agent->answerCall();
    QJsonObject response;
    response["veery_api_key"] = m_currentSessionKey;
    response["veery_command"] = "AnswerCall";
    if (pClient) pClient->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
  } else if (command == "EndCall") {
    m_agent->endCall();
    // EndCall response is usually handled in handleCallStateChanged (LinphoneCallEnd)
  } else if (command == "HoldCall") {
    m_agent->toggleHold();
    // Feedback for Hold/Unhold usually comes from state changes if needed
  } else if (command == "TransferCall" && parts.size() >= 3) {
    m_agent->transferCall(parts[2].toStdString());
    QJsonObject response;
    response["veery_api_key"] = m_currentSessionKey;
    response["veery_command"] = "TransferCall";
    if (pClient) pClient->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
  } else if (command == "EtlCall") {
    m_agent->endTransferCall();
    QJsonObject response;
    response["veery_api_key"] = m_currentSessionKey;
    response["veery_command"] = "EtlCall";
    if (pClient) pClient->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
  } else if (command == "ConfCall") {
    m_agent->conferenceCall();
    QJsonObject response;
    response["veery_api_key"] = m_currentSessionKey;
    response["veery_command"] = "ConfCall";
    if (pClient) pClient->sendTextMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
  } else if (command == "MuteCall") {
    m_agent->toggleMute();
    // Feedback for Mute could be added if Agent provides a way to check current mute state
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
    // LinphoneRegistrationOk -> "Initialized"
    // LinphoneRegistrationCleared -> "Failed"
    // LinphoneRegistrationFailed -> "Failed"
    
    if (state == LinphoneRegistrationOk) {
        regInfo["veery_command"] = "Initialized";
    } else if (state == LinphoneRegistrationCleared || state == LinphoneRegistrationFailed) {
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

void CommandServer::handleCallStateChanged(int state, const QString& remoteAddress) {
    QJsonObject callInfo;
    callInfo["veery_api_key"] = m_currentSessionKey;
    
    // Mapping LinphoneCallState to veery_command
    if (state == LinphoneCallEnd || state == LinphoneCallError || state == LinphoneCallReleased) {
        callInfo["veery_command"] = "EndCall";
    } else {
        return;
    }

    QJsonDocument doc(callInfo);
    QString jsonResponse = doc.toJson(QJsonDocument::Compact);
    for (QWebSocket *pClient : m_clients) {
        if (pClient && pClient->isValid()) {
            pClient->sendTextMessage(jsonResponse);
        }
    }
}

void CommandServer::handleIncomingCallReceived(const QString& remoteAddr, const QString& toAddr, const QString& callId) {
    QString username = remoteAddr;
    int sipPos = username.indexOf("sip:");
    if (sipPos != -1) username = username.mid(sipPos + 4);
    int atPos = username.indexOf("@");
    if (atPos != -1) username = username.left(atPos);

    // 1. IncomingCall
    QJsonObject incoming;
    incoming["veery_api_key"] = m_currentSessionKey;
    incoming["veery_command"] = "IncomingCall";
    incoming["number"] = username;

    QJsonDocument docIncoming(incoming);
    
    // 2. ReceiveCallInfo
    QJsonObject callInfo;
    callInfo["veery_api_key"] = m_currentSessionKey;
    callInfo["veery_command"] = "ReciveCallInfo";
    callInfo["Number"] = username;
    
    QJsonArray veeryData;
    veeryData.append(QString("INVITE %1 SIP/2.0\r").arg(remoteAddr));
    veeryData.append(QString("To: %1\r").arg(toAddr));
    veeryData.append(QString("From: %1\r").arg(remoteAddr));
    veeryData.append(QString("Call-ID: %1\r").arg(callId));
    veeryData.append(QString("User-Agent: FacetoneSoftPhone/1.0\r"));
    
    callInfo["veery_data"] = veeryData;
    
    QJsonDocument docCallInfo(callInfo);

    QString jsonIncoming = docIncoming.toJson(QJsonDocument::Compact);
    QString jsonCallInfo = docCallInfo.toJson(QJsonDocument::Compact);

    for (QWebSocket *pClient : m_clients) {
        if (pClient && pClient->isValid()) {
            pClient->sendTextMessage(jsonIncoming);
            pClient->sendTextMessage(jsonCallInfo);
        }
    }
}
