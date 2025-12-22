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
    void requestBacklightSetChange(int value);
    void requestBacklightGetChange(void);
    void requestVolumeSetChange(int value);
    void requestVolumeGetChange(void);

    // ===== 向 View/Model 发结果 =====
    void backlightSetChangeResult(bool success, int value);
    void backlightGetChangeResult(bool success, int value);
    void volumeSetChangeResult(bool success, int value);
    void volumeGetChangeResult(bool success, int value);

public slots:

    // ===== 接收 View 的请求 =====
    void onBacklightSetChangeRequested(int value);
    void onBacklightGetChangeRequested(void);
    void onVolumeSetChangeRequested(int value);
    void onVolumeGetChangeRequested(void);

    // ===== 接收 Service 的返回 =====
    void handleBacklightSetChangeResult(bool success, int value);
    void handleBacklightGetChangeResult(bool success, int value);
    void handleVolumeSetChangeResult(bool success, int value);
    void handleVolumeGetChangeResult(bool success, int value);

};

#endif // SYSINFO_PRESENTER_H
