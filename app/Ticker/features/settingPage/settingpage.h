#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>
#include "features/pagelifecycleaware.h"

#define SETTING_INFO_CONFIG_FILE_NAME       "settinginfo.json"      // 设置项配置文件名

#define BACKLIGHT_MIN_VALUE                 (1)                     // 背光最小值
#define BACKLIGHT_MAX_VALUE                 (100)                   // 背光最大值
#define BACKLIGHT_STEP_VALUE                (1)                     // 背光加减步进值
#define VOLUME_MIN_VALUE                    (0)                     // 音量最小值
#define VOLUME_MAX_VALUE                    (100)                   // 音量最大值
#define VOLUME_STEP_VALUE                   (1)                     // 音量加减步进值

namespace Ui {
class SettingPage;
}

class SettingPage : public QWidget, public PageLifecycleAware
{
    Q_OBJECT

public:

    explicit SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

    void init();

    // PageLifecycleAware 接口实现
    void onPageEnter() override;        // 页面进入回调
    void onPageLeave() override;        // 页面离开回调

signals:

    // 向 presenter 发送的信号
    void setBacklightRequested(int value);      // 设置背光请求
    void getBacklightRequested();               // 获取背光请求
    void setVolumeRequested(int value);         // 设置音量请求
    void getVolumeRequested();                  // 获取音量请求

public slots:

    // 接收 presenter 发送的结果
    void onBacklightSetResult(bool success, int value);     // 背光设置结果
    void onBacklightGetResult(bool success, int value);     // 背光获取结果
    void onVolumeSetResult(bool success, int value);        // 音量设置结果
    void onVolumeGetResult(bool success, int value);        // 音量获取结果

private slots:

    // 滑动条释放槽函数
    void on_backlightHarSlider_sliderReleased();// 背光滑动条释放
    void on_soundHarSlider_sliderReleased();    // 音量滑动条释放
    // 滑动条值变化槽函数
    void on_soundHarSlider_valueChanged(int value);         // 音量滑动条值变化
    void on_backlightHarSlider_valueChanged(int value);     // 背光滑动条值变化

    // 加减按钮槽函数
    void on_backlightPreBtn_clicked();          // 背光减按钮
    void on_backlightNextBtn_clicked();         // 背光加按钮
    void on_soundPreBtn_clicked();              // 音量减按钮
    void on_soundNextBtn_clicked();             // 音量加按钮

    // 复选框槽函数
    void on_soundCheckBox_stateChanged(int arg1);           // 音量静音勾选框状态变化

private:

    void uiInit();                              // UI 初始化
    void updateBacklightUI(int value);          // 更新背光相关控件的 UI 显示
    void updateVolumeUI(int value);             // 更新音量相关控件的 UI 显示

    void saveSettingToLocal();                  // 保存设置到本地
    void loadSettingFromLocal();                // 从本地加载设置

    Ui::SettingPage *ui;

    int m_oldVolumeValue = -1;                  // 用于记录静音前的音量值
};

#endif // SETTINGPAGE_H
