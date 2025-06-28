#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "newhostdialog.h"
#include "helper.h"

#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QRegularExpression>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Set up proxy
    _proxyModel = new QSortFilterProxyModel(this);
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxyModel->setFilterKeyColumn(-1); // All columns

    // Load data
    LoadHosts();

    // Init UI
    ui->tableView->setModel(_proxyModel);

    // This line resizes column 0 to fit its contents (checkbox)
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    // Connect
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::add);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::remove);

    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            _proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::save);
    connect(ui->reloadButton, &QPushButton::clicked, this, &MainWindow::reload);
    connect(ui->closeButton, &QPushButton::clicked, this, &QApplication::quit);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LoadHosts()
{
    _hostModel.reset(new HostModel(this));
    if (!_hostModel->loadFromFile("C:\\Windows\\System32\\drivers\\etc\\hosts"))
        QMessageBox::critical(this, "Error Opening Hosts File", "Unable to open hosts file.");

    _proxyModel->setSourceModel(_hostModel.get());
}

void MainWindow::reload()
{
    LoadHosts();
}

void MainWindow::add()
{
    NewHostDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString ip = dialog.ip();
        QString domain = dialog.domain();

        if (ip.isEmpty() || domain.isEmpty())
            return;

        int row = _hostModel->rowCount();
        if (_hostModel->insertRow(row)) {
            _hostModel->setData(_hostModel->index(row, 0), true);
            _hostModel->setData(_hostModel->index(row, 1), ip);
            _hostModel->setData(_hostModel->index(row, 2), domain);
        }
    }
}

void MainWindow::remove()
{
    auto reply = QMessageBox::warning(this,
                                  "Confirm Deletion",
                                  "Do you want to delete the host?",
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        QModelIndex proxyIndex = ui->tableView->currentIndex();
        QModelIndex sourceIndex = _proxyModel->mapToSource(proxyIndex);
        _hostModel->removeRow(sourceIndex.row());
    }
}

void MainWindow::onSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    ui->deleteButton->setEnabled(!selected.indexes().isEmpty());
}

void MainWindow::save()
{
    QString tempPath = QDir::temp().filePath("hosts_temp");
    if (!_hostModel->saveToFile(tempPath)) {
        QMessageBox::critical(this, "Error", "Cannot write temporary hosts file.");
        return;
    }

    QString tempPathNative = QDir::toNativeSeparators(tempPath);
    QString hostsFilePath = "C:\\Windows\\System32\\drivers\\etc\\hosts";

    // Call elevated copy
    if (!runElevatedCopy(tempPathNative, hostsFilePath))
    {
        QMessageBox::warning(this, "Error", "Failed to elevate privileges or copy hosts file.");
        return;
    }

    QFile::remove(tempPath);

    QMessageBox::information(this, "Success", "Hosts file saved successfully.");
}
