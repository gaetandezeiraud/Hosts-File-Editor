#include "newhostdialog.h"
#include "ui_newhostdialog.h"

NewHostDialog::NewHostDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewHostDialog)
{
    ui->setupUi(this);
    setFixedSize(size());
}

NewHostDialog::~NewHostDialog()
{
    delete ui;
}

QString NewHostDialog::ip() const
{
    return ui->ipLineEdit->text().trimmed();
}

QString NewHostDialog::domain() const
{
    return ui->domainLineEdit->text().trimmed();
}
