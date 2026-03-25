#pragma once
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFormLayout>
#include "Agent.h"

class SettingsWindow : public QDialog {
    Q_OBJECT
public:
    SettingsWindow(Agent* agent, QWidget* parent = nullptr);

private slots:
    void onSaveClicked();
    void onBrowseCertClicked();
    void onBrowseRingtoneClicked();

private:
    void setupUI();
    void loadSettings();

    Agent* m_agent;
    QCheckBox* m_autoAnswerCheck;
    QComboBox* m_inputDeviceCombo;
    QComboBox* m_outputDeviceCombo;
    QLineEdit* m_wsPortEdit;
    QLineEdit* m_apiPortEdit;
    QLineEdit* m_remoteSipPortEdit;
    QCheckBox* m_useWssCheck;
    QLineEdit* m_certPathEdit;
    QLineEdit* m_ringtonePathEdit;
};
