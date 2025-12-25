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
    void requestBacklightSetChange(int value);          // 设置背光请求
    void requestBacklightGetChange(void);               // 获取背光请求
    void requestVolumeSetChange(int value);             // 设置音量请求
    void requestVolumeGetChange(void);                  // 获取音量请求

    // ===== 向 View/Model 发结果 =====
    void backlightSetChangeResult(bool success, int value);     // 背光设置结果
    void backlightGetChangeResult(bool success, int value);     // 背光获取结果
    void volumeSetChangeResult(bool success, int value);        // 音量设置结果    
    void volumeGetChangeResult(bool success, int value);        // 音量获取结果    

public slots:

    // ===== 接收 View 的请求 =====
    void onBacklightSetChangeRequested(int value);      // 设置背光请求
    void onBacklightGetChangeRequested(void);           // 获取背光请求
    void onVolumeSetChangeRequested(int value);         // 设置音量请求
    void onVolumeGetChangeRequested(void);              // 获取音量请求

    // ===== 接收 Service 的返回 =====
    void handleBacklightSetChangeResult(bool success, int value);   // 背光设置结果
    void handleBacklightGetChangeResult(bool success, int value);   // 背光获取结果
    void handleVolumeSetChangeResult(bool success, int value);      // 音量设置结果
    void handleVolumeGetChangeResult(bool success, int value);      // 音量获取结果

private slots:

private:

};

#endif // SYSINFO_PRESENTER_H
