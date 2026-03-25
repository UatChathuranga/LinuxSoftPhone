#pragma once
#include <QObject>
#include <QTimer>
#include <string>
#include <QString>
#include <linphone/core.h>
#include <linphone/factory.h>
#include <vector>

struct AudioDeviceInfo {
    QString id;
    QString name;
    unsigned int capabilities;
};

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
    std::vector<AudioDeviceInfo> getAudioDevices();
    
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
    void callStateChangedSignal(int state, const QString& remoteAddress);
    void incomingCallReceivedSignal(const QString& remoteAddr, const QString& toAddr, const QString& callId);
    void audioDevicesChangedSignal();

public slots:
    void reloadAudioDevices();

private:
    static void globalStateChanged(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message);
    static void registrationStateChanged(LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message);
    static void callStateChanged(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message);

    void initCore();
    LinphoneCore* lc;
    LinphoneCoreCbs* cbs;
    std::string username;
    std::string password;
    std::string domain;
    int port;
    bool running;
    QTimer* m_timer;
};
