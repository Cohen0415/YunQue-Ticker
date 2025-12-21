#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>

namespace Ui {
class SettingPage;
}

class SettingPage : public QWidget
{
    Q_OBJECT

public:

    explicit SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

signals:

    // 向 presenter 发送的信号
    void setBacklightRequested(int value);
    void setVolumeRequested(int value);

private slots:

    void on_backlightHarSlider_sliderReleased();
    void on_soundHarSlider_sliderReleased();

public slots:

    // 接收 presenter 发送的结果
    void onBacklightSetResult(bool success, int value);
    void onVolumeSetResult(bool success, int value);

private:

    // UI init
    void UIinit();

    // 更新 UI 显示
    void updateBacklightUI(bool success, int value);
    void updateVolumeUI(bool success, int value);

private:

    Ui::SettingPage *ui;
};

#endif // SETTINGPAGE_H
