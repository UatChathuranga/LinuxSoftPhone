#include "Agent.h"
#include "AppConfig.h"
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTimer>
#include <QCoreApplication>
#include <iostream>
#include <string>
#include <unistd.h>

Agent::Agent(const std::string &username, const std::string &password,
             const std::string &domain, QObject *parent)
    : QObject(parent), username(username), password(password), domain(domain),
      port(5060), lc(nullptr), cbs(nullptr), running(false), m_timer(nullptr) {
  initCore();
}

Agent::~Agent() { stop(); }

void Agent::globalStateChanged(LinphoneCore *lc, LinphoneGlobalState gstate,
                               const char *message) {
  qDebug() << "Global state changed:" << (message ? message : "");
}

void Agent::registrationStateChanged(LinphoneCore *lc, LinphoneProxyConfig *cfg,
                                     LinphoneRegistrationState cstate,
                                     const char *message) {
  Agent *self = (Agent *)linphone_core_get_user_data(lc);
  if (self) {
    qDebug() << "Registration state changed:" << cstate
             << (message ? message : "");
    emit self->registrationStateChangedSignal(
        (int)cstate, QString::fromUtf8(message ? message : ""));
  }
}

void Agent::callStateChanged(LinphoneCore *lc, LinphoneCall *call,
                             LinphoneCallState cstate, const char *message) {
  Agent *self = (Agent *)linphone_core_get_user_data(lc);
  if (self) {
    qDebug() << "Call state changed:" << cstate << (message ? message : "");

    QString remoteAddress;
    const LinphoneAddress *addr = linphone_call_get_remote_address(call);

    if (addr) {
      char *as_string = linphone_address_as_string(addr);
      remoteAddress = QString::fromUtf8(as_string);
      ms_free(as_string);
    }

    if (cstate == LinphoneCallIncomingReceived) {
      QString toAddress;
      const LinphoneAddress *to_addr = linphone_call_get_to_address(call);
      if (to_addr) {
        char *as_string = linphone_address_as_string(to_addr);
        toAddress = QString::fromUtf8(as_string);
        ms_free(as_string);
      }

      QString callId;
      LinphoneCallLog *log = linphone_call_get_call_log(call);
      if (log) {
        const char *cid = linphone_call_log_get_call_id(log);
        if (cid)
          callId = QString::fromUtf8(cid);
      }

      emit self->incomingCallReceivedSignal(remoteAddress, toAddress, callId);
    }

    emit self->callStateChangedSignal((int)cstate, remoteAddress);
  }
}

bool Agent::start() {
  Register();
  return true;
}

void Agent::initiateCall(const std::string &destination) {
  if (!lc)
    return;
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

void Agent::setCredentials(const std::string &username,
                           const std::string &password,
                           const std::string &domain) {
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

void Agent::initCore() {
  if (lc)
    return;
  LinphoneFactory *factory = linphone_factory_get();
  cbs = linphone_factory_create_core_cbs(factory);
  linphone_core_cbs_set_global_state_changed(cbs, globalStateChanged);
  linphone_core_cbs_set_registration_state_changed(cbs,
                                                   registrationStateChanged);
  linphone_core_cbs_set_call_state_changed(cbs, callStateChanged);

  lc = linphone_factory_create_core(factory, cbs, nullptr, nullptr);
  linphone_core_set_user_data(lc, this);
  linphone_core_set_network_reachable(lc, TRUE);
  linphone_core_set_user_agent(lc, "FacetoneSoftPhone", "1.0");

  // Apply ringtone from config
  QString ringtonePath = AppConfig::instance().ringtonePath;
  if (ringtonePath.isEmpty() || !QFile::exists(ringtonePath)) {
    // Try local build directory first (for development)
    QString localRingtone = QCoreApplication::applicationDirPath() + "/ringtone.wav";
    if (QFile::exists(localRingtone)) {
      ringtonePath = localRingtone;
    } else {
      // Fallback for system installation
      QString systemRingtone = "/usr/local/share/facetonesoftphone/ringtone.wav";
      // Fallback for bundled version
      QString bundledRingtone = "/opt/facetonesoftphone/share/ringtone.wav";
      
      if (QFile::exists(systemRingtone)) {
        ringtonePath = systemRingtone;
      } else if (QFile::exists(bundledRingtone)) {
        ringtonePath = bundledRingtone;
      }
    }
  }

  if (QFile::exists(ringtonePath)) {
    linphone_core_set_ring(lc, ringtonePath.toStdString().c_str());
  }

  linphone_core_start(lc);
  running = true;
  reloadAudioDevices();

  if (!m_timer) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Agent::loop);
  }
  m_timer->start(30);
}

void Agent::Register() {
  if (!lc)
    initCore();
  LinphoneFactory *factory = linphone_factory_get();

  // Clear previous auth info
  linphone_core_clear_all_auth_info(lc);
  linphone_core_clear_proxy_config(lc);

  // Auth Info
  LinphoneAuthInfo *auth_info = linphone_factory_create_auth_info(
      factory, username.c_str(), NULL, password.c_str(), NULL, NULL,
      domain.c_str());
  linphone_core_add_auth_info(lc, auth_info);
  linphone_auth_info_unref(auth_info);

  // Transport
  LinphoneTransports *trans = linphone_core_get_transports(lc);
  linphone_transports_set_udp_port(trans, 5060);
  linphone_core_set_transports(lc, trans);

  // Proxy Config (Registration)
  std::string identity = "sip:" + username + "@" + domain;
  std::string proxy = "sip:" + domain + ":" + std::to_string(port);

  LinphoneProxyConfig *proxy_cfg = linphone_core_create_proxy_config(lc);
  LinphoneAddress *identity_addr =
      linphone_factory_create_address(factory, identity.c_str());

  if (identity_addr) {
    linphone_proxy_config_set_identity_address(proxy_cfg, identity_addr);
    linphone_proxy_config_set_server_addr(proxy_cfg, proxy.c_str());
    linphone_proxy_config_enable_register(proxy_cfg, TRUE);
    linphone_core_add_proxy_config(lc, proxy_cfg);
    linphone_core_set_default_proxy_config(lc, proxy_cfg);
    linphone_address_unref(identity_addr);
  }
  linphone_proxy_config_unref(proxy_cfg);

  // Set Audio Devices from Config
  reloadAudioDevices();
  QString inputDev = AppConfig::instance().inputDevice;
  QString outputDev = AppConfig::instance().outputDevice;
  if (!inputDev.isEmpty())
    linphone_core_set_capture_device(lc, inputDev.toStdString().c_str());
  if (!outputDev.isEmpty()) {
    linphone_core_set_playback_device(lc, outputDev.toStdString().c_str());
    linphone_core_set_ringer_device(lc, outputDev.toStdString().c_str());
  }

  // Set Ringtone
  QString ringtonePath = AppConfig::instance().ringtonePath;
  if (!ringtonePath.isEmpty()) {
    // If relative, make absolute relative to app dir or use as is
    QFile rtFile(ringtonePath);
    if (!rtFile.exists()) {
      // Fallback to system sound if default placeholder is missing/empty
      ringtonePath = "/usr/share/sounds/linphone/ringback.wav";
    }
    linphone_core_set_ring(lc, ringtonePath.toStdString().c_str());
  }

  qDebug() << "Agent: Started registration for"
           << QString::fromStdString(identity);
}

void Agent::Unregister() { stop(); }

void Agent::MakeCall(const std::string &number) { initiateCall(number); }

void Agent::answerCall() {
  LinphoneCall *call = linphone_core_get_current_call(lc);
  if (call && linphone_call_get_state(call) == LinphoneCallIncomingReceived) {
    linphone_call_accept(call);
  }
}

void Agent::endCall() {
  LinphoneCall *call = linphone_core_get_current_call(lc);
  if (call) {
    linphone_call_terminate(call);
  }
}

void Agent::toggleHold() {
  LinphoneCall *call = linphone_core_get_current_call(lc);
  if (call) {
    LinphoneCallState state = linphone_call_get_state(call);
    if (state == LinphoneCallPaused || state == LinphoneCallPausing) {
      linphone_call_resume(call);
    } else if (state == LinphoneCallStreamsRunning ||
               state == LinphoneCallConnected) {
      linphone_call_pause(call);
    }
  }
}

void Agent::transferCall(const std::string &destination) {
  LinphoneCall *call = linphone_core_get_current_call(lc);
  if (call) {
    std::string dtmf =
        "*" + AppConfig::instance().externalTransferCode.toStdString() +
        destination + "#";
    linphone_call_send_dtmfs(call, dtmf.c_str());
  }
}

void Agent::endTransferCall() {
  LinphoneCall *call = linphone_core_get_current_call(lc);
  if (call) {
    linphone_call_send_dtmfs(call, "#");
  }
}

void Agent::conferenceCall() {
  LinphoneCall *call = linphone_core_get_current_call(lc);
  if (call) {
    linphone_call_send_dtmfs(call, "0");
  }
}

void Agent::toggleMute() {
  bool enabled = linphone_core_mic_enabled(lc);
  linphone_core_enable_mic(lc, !enabled);
}

void Agent::reloadAudioDevices() {
  if (!lc)
    return;
  const bctbx_list_t *devices = linphone_core_get_audio_devices(lc);
  qDebug() << "--- Audio Devices ---";
  for (const bctbx_list_t *it = devices; it != NULL; it = it->next) {
    LinphoneAudioDevice *dev = (LinphoneAudioDevice *)it->data;
    qDebug() << "Device:" << linphone_audio_device_get_device_name(dev)
             << "ID:" << linphone_audio_device_get_id(dev)
             << "Capabilities:" << linphone_audio_device_get_capabilities(dev);
  }
  qDebug() << "---------------------";
}

std::vector<AudioDeviceInfo> Agent::getAudioDevices() {
  std::vector<AudioDeviceInfo> result;
  if (!lc)
    return result;

  const bctbx_list_t *devices = linphone_core_get_audio_devices(lc);
  const bctbx_list_t *it;

  if (devices) {
    for (it = devices; it != NULL; it = it->next) {
      LinphoneAudioDevice *dev = (LinphoneAudioDevice *)it->data;
      AudioDeviceInfo info;
      info.name = QString::fromUtf8(linphone_audio_device_get_device_name(dev));
      info.id = QString::fromUtf8(linphone_audio_device_get_id(dev));
      info.capabilities = linphone_audio_device_get_capabilities(dev);

      // Fallback categorization for "unknown" devices
      if (info.capabilities == 0) {
        if (info.name.contains("Microphone", Qt::CaseInsensitive) ||
            info.name.contains("Capture", Qt::CaseInsensitive))
          info.capabilities |= 1;
        if (info.name.contains("Speaker", Qt::CaseInsensitive) ||
            info.name.contains("Headphones", Qt::CaseInsensitive) ||
            info.name.contains("Output", Qt::CaseInsensitive))
          info.capabilities |= 2;
        if (info.name == "default" || info.name == "sof-hda-dsp")
          info.capabilities |= 3;
      }
      result.push_back(info);
    }
  }

  // If new API returns nothing, fallback to old sound devices API
  if (result.empty()) {
    const char **sound_devices = linphone_core_get_sound_devices(lc);
    if (sound_devices) {
      for (int i = 0; sound_devices[i] != NULL; i++) {
        AudioDeviceInfo info;
        info.name = QString::fromUtf8(sound_devices[i]);
        info.id = info.name;
        info.capabilities = 0;

        if (info.name.contains("Microphone", Qt::CaseInsensitive) ||
            info.name.contains("Capture", Qt::CaseInsensitive))
          info.capabilities |= 1;
        if (info.name.contains("Speaker", Qt::CaseInsensitive) ||
            info.name.contains("Headphones", Qt::CaseInsensitive) ||
            info.name.contains("Output", Qt::CaseInsensitive))
          info.capabilities |= 2;

        if (info.capabilities == 0 || info.name == "default" ||
            info.name == "sof-hda-dsp")
          info.capabilities |= 3; // Assume both if unknown

        result.push_back(info);
      }
    }
  }

  qDebug() << "Agent::getAudioDevices: Final count:" << result.size();
  return result;
}