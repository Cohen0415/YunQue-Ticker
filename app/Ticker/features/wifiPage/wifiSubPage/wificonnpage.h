#ifndef WIFICONNPAGE_H
#define WIFICONNPAGE_H

#include <QWidget>

namespace Ui {
class WifiConnPage;
}

class WifiConnPage : public QWidget
{
    Q_OBJECT

public:

    explicit WifiConnPage(QWidget *parent = nullptr);
    ~WifiConnPage();

signals:


public slots:

    void testGetStatus(void);

private slots:

    void on_ssidLineEdit_textChanged(const QString &arg1);
    void on_pwdLineEdit_textChanged(const QString &arg1);

private:

    // UI 初始化
    void UIInit(void);
    // 用户输入框内容检测
    void inputLineInspect(void);

private:

    Ui::WifiConnPage *ui;

};

#endif // WIFICONNPAGE_H
