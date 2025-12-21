#ifndef SYSINFO_PRESENTER_H
#define SYSINFO_PRESENTER_H

#include <QObject>

class SettingPresenter : public QObject
{
    Q_OBJECT

public:

    explicit SettingPresenter(QObject* parent = nullptr);

signals:

    // ===== 向 Service 请求 =====
    void requestBacklightChange(int value);
    void requestVolumeChange(int value);

    // ===== 向 View/Model 发结果 =====
    void backlightChangeResult(bool success, int value);
    void volumeChangeResult(bool success, int value);

public slots:

    // ===== 接收 View 的请求 =====
    void onBacklightChangeRequested(int value);
    void onVolumeChangeRequested(int value);

    // ===== 接收 Service 的返回 =====
    void handleBacklightChangeResult(bool success, int value);
    void handleVolumeChangeResult(bool success, int value);

};

#endif // SYSINFO_PRESENTER_H
