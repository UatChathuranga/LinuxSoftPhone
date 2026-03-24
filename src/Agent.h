#pragma once
#include <QObject>
#include <QTimer>
#include <string>
#include <QString>
#include <linphone/core.h>
#include <linphone/factory.h>

class Agent : public QObject {
    Q_OBJECT
public:
    Agent(const std::string& username, const std::string& password, const std::string& domain, QObject* parent = nullptr);
    ~Agent();

    bool start();
    void stop();
    void setCredentials(const std::string& username, const std::string& password, const std::string& domain);
    void loop();
    void initiateCall(const std::string& destination);
    
    // SIP Operations (stubs for WS commands)
    void Register();
    void Unregister();
    void MakeCall(const std::string& number);
    void answerCall();
    void endCall();
    void toggleHold();
    void transferCall(const std::string& destination);
    void endTransferCall();
    void conferenceCall();
    void toggleMute();

signals:
    void registrationStateChangedSignal(int state, const QString& message);
    void callStateChangedSignal(int state, const QString& message);

private:
    static void globalStateChanged(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message);
    static void registrationStateChanged(LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message);

    LinphoneCore* lc;
    LinphoneCoreCbs* cbs;
    std::string username;
    std::string password;
    std::string domain;
    int port;
    bool running;
    QTimer* m_timer;
};
