#include "SettingsWindow.h"
#include "AppConfig.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

SettingsWindow::SettingsWindow(Agent* agent, QWidget* parent)
    : QDialog(parent), m_agent(agent) {
    setWindowTitle("Settings");
    setMinimumWidth(450);
    setupUI();
    loadSettings();
}

void SettingsWindow::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    m_autoAnswerCheck = new QCheckBox(this);
    m_autoAnswerCheck->setEnabled(false);
    formLayout->addRow("Auto answer:", m_autoAnswerCheck);

    m_inputDeviceCombo = new QComboBox(this);
    m_inputDeviceCombo->setMinimumWidth(250);
    formLayout->addRow("Input device:", m_inputDeviceCombo);

    m_outputDeviceCombo = new QComboBox(this);
    m_outputDeviceCombo->setMinimumWidth(250);
    formLayout->addRow("Output device:", m_outputDeviceCombo);

    m_wsPortEdit = new QLineEdit(this);
    m_wsPortEdit->setReadOnly(true);
    formLayout->addRow("WS port:", m_wsPortEdit);

    m_apiPortEdit = new QLineEdit(this);
    m_apiPortEdit->setReadOnly(true);
    formLayout->addRow("API port:", m_apiPortEdit);

    m_remoteSipPortEdit = new QLineEdit(this);
    formLayout->addRow("Remote SIP port:", m_remoteSipPortEdit);

    m_useWssCheck = new QCheckBox(this);
    m_useWssCheck->setEnabled(false);
    formLayout->addRow("Use WSS:", m_useWssCheck);

    QHBoxLayout* certLayout = new QHBoxLayout();
    m_certPathEdit = new QLineEdit(this);
    QPushButton* browseBtn = new QPushButton("Browse", this);
    certLayout->addWidget(m_certPathEdit);
    certLayout->addWidget(browseBtn);
    formLayout->addRow("Certificate path:", certLayout);
    connect(browseBtn, &QPushButton::clicked, this, &SettingsWindow::onBrowseCertClicked);
    
    QHBoxLayout* ringtoneLayout = new QHBoxLayout();
    m_ringtonePathEdit = new QLineEdit(this);
    QPushButton* browseRingtoneBtn = new QPushButton("Browse", this);
    ringtoneLayout->addWidget(m_ringtonePathEdit);
    ringtoneLayout->addWidget(browseRingtoneBtn);
    formLayout->addRow("Ringtone path:", ringtoneLayout);
    connect(browseRingtoneBtn, &QPushButton::clicked, this, &SettingsWindow::onBrowseRingtoneClicked);

    mainLayout->addLayout(formLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* saveBtn = new QPushButton("Save", this);
    QPushButton* closeBtn = new QPushButton("Close", this);
    btnLayout->addStretch();
    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &SettingsWindow::onSaveClicked);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void SettingsWindow::loadSettings() {
    AppConfig& config = AppConfig::instance();
    m_autoAnswerCheck->setChecked(config.autoAnswer);
    m_wsPortEdit->setText(QString::number(config.webSocketPort));
    m_apiPortEdit->setText(config.apiPort);
    m_remoteSipPortEdit->setText(config.remoteSipPort);
    m_useWssCheck->setChecked(config.useWss);
    m_certPathEdit->setText(config.certPath);
    m_ringtonePathEdit->setText(config.ringtonePath);

    std::vector<AudioDeviceInfo> devices = m_agent->getAudioDevices();
    qDebug() << "SettingsWindow::loadSettings: UI layer found" << devices.size() << "devices";
    m_inputDeviceCombo->clear();
    m_outputDeviceCombo->clear();

    for (const auto& dev : devices) {
        qDebug() << "  Evaluating device:" << dev.name << "Caps:" << dev.capabilities;
        if (dev.capabilities & 1) { // LinphoneAudioDeviceCapabilityCapCapture
            m_inputDeviceCombo->addItem(dev.name, dev.id);
            qDebug() << "    Added to Input Combo";
        }
        if (dev.capabilities & 2) { // LinphoneAudioDeviceCapabilityCapPlay
            m_outputDeviceCombo->addItem(dev.name, dev.id);
            qDebug() << "    Added to Output Combo";
        }
    }

    // Restore selection
    int inputIdx = m_inputDeviceCombo->findData(config.inputDevice);
    if (inputIdx != -1) m_inputDeviceCombo->setCurrentIndex(inputIdx);
    
    int outputIdx = m_outputDeviceCombo->findData(config.outputDevice);
    if (outputIdx != -1) m_outputDeviceCombo->setCurrentIndex(outputIdx);
}

void SettingsWindow::onSaveClicked() {
    AppConfig& config = AppConfig::instance();
    config.inputDevice = m_inputDeviceCombo->currentData().toString();
    config.outputDevice = m_outputDeviceCombo->currentData().toString();
    config.remoteSipPort = m_remoteSipPortEdit->text();
    config.certPath = m_certPathEdit->text();
    config.ringtonePath = m_ringtonePathEdit->text();
    config.save();

    m_agent->reloadAudioDevices(); // Apply immediately
    QMessageBox::information(this, "Settings", "Settings saved successfully.");
    accept();
}

void SettingsWindow::onBrowseCertClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Certificate", "", "Certificates (*.pem *.crt *.pfx);;All Files (*)");
    if (!fileName.isEmpty()) {
        m_certPathEdit->setText(fileName);
    }
}
void SettingsWindow::onBrowseRingtoneClicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Ringtone", "", "Audio Files (*.wav);;All Files (*)");
    if (!fileName.isEmpty()) {
        m_ringtonePathEdit->setText(fileName);
    }
}
