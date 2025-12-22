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

    void init();

signals:

    // 向 presenter 发送的信号
    void setBacklightRequested(int value);
    void getBacklightRequested();
    void setVolumeRequested(int value);
    void getVolumeRequested();

private slots:

    // 滑动条释放槽函数
    void on_backlightHarSlider_sliderReleased();
    void on_soundHarSlider_sliderReleased();

    // 加减按钮槽函数
    void on_backlightPreBtn_clicked();
    void on_backlightNextBtn_clicked();
    void on_soundPreBtn_clicked();
    void on_soundNextBtn_clicked();

    // 复选框槽函数
    void on_soundCheckBox_stateChanged(int arg1);

    // 滑动条值变化槽函数
    void on_soundHarSlider_valueChanged(int value);
    void on_backlightHarSlider_valueChanged(int value);

public slots:

    // 接收 presenter 发送的结果
    void onBacklightSetResult(bool success, int value);
    void onBacklightGetResult(bool success, int value);
    void onVolumeSetResult(bool success, int value);
    void onVolumeGetResult(bool success, int value);

private:

    // UI init
    void UIinit();

    // 更新 UI 显示
    void updateBacklightUI(int value);
    void updateVolumeUI(int value);

    // 加载样式表
    QString LoadQssStyle(const QString &path);

private:

    Ui::SettingPage *ui;

    int m_oldVolumeValue = -1; // 用于记录静音前的音量值
};

#endif // SETTINGPAGE_H
