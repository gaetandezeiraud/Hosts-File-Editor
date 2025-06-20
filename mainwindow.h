#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>
#include <QItemSelection>
#include <QSortFilterProxyModel>

#include "HostModel.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void LoadHosts();

private slots:
    void reload();
    void add();
    void remove();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &);
    void save();

private:
    Ui::MainWindow *ui;
    QScopedPointer<HostModel> _hostModel;
    QSortFilterProxyModel* _proxyModel = nullptr;
};
#endif // MAINWINDOW_H
