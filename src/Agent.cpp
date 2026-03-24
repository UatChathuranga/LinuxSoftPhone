#include "Agent.h"
#include "AppConfig.h"
#include <QDebug>
#include <QString>
#include <iostream>
#include <unistd.h>
#include <string>

Agent::Agent(const std::string& username, const std::string& password, const std::string& domain, QObject* parent)
    : QObject(parent), username(username), password(password), domain(domain), port(5060), lc(nullptr), cbs(nullptr), running(false), m_timer(nullptr) {
}

Agent::~Agent() {
    stop();
}

void Agent::globalStateChanged(LinphoneCore *lc, LinphoneGlobalState gstate, const char *message) {
    qDebug() << "Global state changed:" << (message ? message : "");
}

void Agent::registrationStateChanged(LinphoneCore *lc, LinphoneProxyConfig *cfg, LinphoneRegistrationState cstate, const char *message) {
    Agent* self = (Agent*)linphone_core_get_user_data(lc);
    if (self) {
        qDebug() << "Registration state changed:" << cstate << (message ? message : "");
        emit self->registrationStateChangedSignal((int)cstate, QString::fromUtf8(message ? message : ""));
    }
}

bool Agent::start() {
    Register();
    return true;
}

void Agent::initiateCall(const std::string& destination) {
    if (!lc) return;
    std::string destination_uri = "sip:" + destination;
    if (destination.find("@") == std::string::npos) {
        destination_uri += "@" + domain;
    }
    qDebug() << "Initiating call to" << QString::fromStdString(destination_uri);
    linphone_core_invite(lc, destination_uri.c_str());
}

void Agent::stop() {
    running = false;
    if (m_timer) {
        m_timer->stop();
    }
    if (lc) {
        linphone_core_stop(lc);
        linphone_core_unref(lc);
        lc = nullptr;
    }
}

void Agent::setCredentials(const std::string& username, const std::string& password, const std::string& domain) {
    this->username = username;
    this->password = password;
    this->domain = domain;
    // Extract port from domain if present, otherwise use config default
    size_t colonPos = domain.find(':');
    if (colonPos != std::string::npos) {
        this->domain = domain.substr(0, colonPos);
        try {
            this->port = std::stoi(domain.substr(colonPos + 1));
        } catch (...) {
            this->port = AppConfig::instance().remoteSipPort.toInt();
        }
    } else {
        this->port = AppConfig::instance().remoteSipPort.toInt();
    }
}

void Agent::loop() {
    if (lc) {
        linphone_core_iterate(lc);
    }
}

void Agent::Register() {
    if (lc) {
        stop();
    }

    LinphoneFactory *factory = linphone_factory_get();
    lc = linphone_factory_create_core_3(factory, nullptr, nullptr, nullptr);
    if (!lc) return;

    linphone_core_set_user_data(lc, this);
    
    // Set up callbacks
    cbs = linphone_factory_create_core_cbs(factory);
    linphone_core_cbs_set_global_state_changed(cbs, globalStateChanged);
    linphone_core_cbs_set_registration_state_changed(cbs, registrationStateChanged);
    linphone_core_add_callbacks(lc, cbs);
    linphone_core_cbs_unref(cbs);

    // Auth Info
    LinphoneAuthInfo* auth_info = linphone_factory_create_auth_info(factory, username.c_str(), nullptr, password.c_str(), nullptr, nullptr, domain.c_str());
    linphone_core_add_auth_info(lc, auth_info);
    linphone_auth_info_unref(auth_info);

    // Transport
    LinphoneTransports *trans = linphone_core_get_transports(lc);
    linphone_transports_set_udp_port(trans, 5060);
    linphone_core_set_transports(lc, trans);

    // Proxy Config (Registration)
    std::string identity = "sip:" + username + "@" + domain;
    std::string proxy = "sip:" + domain + ":" + std::to_string(port);

    LinphoneProxyConfig* proxy_cfg = linphone_core_create_proxy_config(lc);
    LinphoneAddress* identity_addr = linphone_factory_create_address(factory, identity.c_str());
    
    if (identity_addr) {
        linphone_proxy_config_set_identity_address(proxy_cfg, identity_addr);
        linphone_proxy_config_set_server_addr(proxy_cfg, proxy.c_str());
        linphone_proxy_config_enable_register(proxy_cfg, TRUE);
        linphone_core_add_proxy_config(lc, proxy_cfg);
        linphone_core_set_default_proxy_config(lc, proxy_cfg);
        linphone_address_unref(identity_addr);
    }
    linphone_proxy_config_unref(proxy_cfg);

    linphone_core_set_network_reachable(lc, TRUE);
    linphone_core_set_user_agent(lc, "FacetoneSoftPhone", "1.0");

    linphone_core_start(lc);
    running = true;

    if (!m_timer) {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &Agent::loop);
    }
    m_timer->start(30);

    qDebug() << "Agent: Started registration for" << QString::fromStdString(identity);
}

void Agent::Unregister() {
    stop();
}

void Agent::MakeCall(const std::string& number) {
    initiateCall(number);
}

void Agent::answerCall() {
    // To be implemented
}

void Agent::endCall() {
    // To be implemented
}

void Agent::toggleHold() {
    // To be implemented
}

void Agent::transferCall(const std::string& destination) {
    // To be implemented
}

void Agent::endTransferCall() {
    // To be implemented
}

void Agent::conferenceCall() {
    // To be implemented
}

void Agent::toggleMute() {
    // To be implemented
}
