#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class ConfirmDialog : public QDialog
{
    Q_OBJECT

public:

    explicit ConfirmDialog(const QString &text, QWidget *parent = nullptr);

    // 推荐静态便捷调用
    static bool ask(QWidget *parent, const QString &text);

private:

    QLabel *m_label;
    QPushButton *m_btnCancel;
    QPushButton *m_btnOk;
    
};

#endif // CONFIRMDIALOG_H
