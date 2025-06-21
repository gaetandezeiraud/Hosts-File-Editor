#ifndef NEWHOSTDIALOG_H
#define NEWHOSTDIALOG_H

#include <QDialog>

namespace Ui {
class NewHostDialog;
}

class NewHostDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewHostDialog(QWidget *parent = nullptr);
    ~NewHostDialog();

    QString ip() const;
    QString domain() const;

private:
    Ui::NewHostDialog *ui;
};

#endif // NEWHOSTDIALOG_H
