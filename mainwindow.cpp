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

    LoadHosts();

    // Set up proxy
    _proxyModel = new QSortFilterProxyModel(this);
    _proxyModel->setSourceModel(_hostModel.get());
    _proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    _proxyModel->setFilterKeyColumn(-1); // All columns

    // Init UI
    ui->tableView->setModel(_proxyModel);

    // Connect
    connect(ui->addButton, &QPushButton::clicked, this, &MainWindow::add);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::remove);

    connect(ui->searchLineEdit, &QLineEdit::textChanged,
            _proxyModel, &QSortFilterProxyModel::setFilterFixedString);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(ui->reloadButton, &QPushButton::clicked, this, &MainWindow::reload);
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::save);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::LoadHosts()
{
    QFile hostsFile = QFile("C:\\Windows\\System32\\drivers\\etc\\hosts");

    if (!hostsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString errorMsg = "Unable to open hosts file:\n" + hostsFile.errorString();
        qWarning() << errorMsg;

        QMessageBox::critical(this, "Error Opening Hosts File", errorMsg);
        return;
    }

    QTextStream in(&hostsFile);
    QStringList entries;

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#"))
            continue; // Skip empty or comment lines
        entries << line;
    }

    hostsFile.close();

    // Generate entries list
    std::vector<Host> hosts;
    hosts.reserve(entries.size());

    static const QRegularExpression expression = QRegularExpression("\\s+");

    for (const QString& entry : entries)
    {
        const QStringList values = entry.split(expression, Qt::SkipEmptyParts);
        if (values.size() < 2)
            continue;

        hosts.push_back(Host{ values[0], values[1] });
    }

    // Init model
    _hostModel.reset(new HostModel(this));
    _hostModel->setHostData(hosts);
}

void MainWindow::reload()
{
    LoadHosts();
    _proxyModel->setSourceModel(_hostModel.get());
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
            _hostModel->setData(_hostModel->index(row, 0), ip);
            _hostModel->setData(_hostModel->index(row, 1), domain);
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

    // Write to temp file
    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Cannot write temporary hosts file.");
        return;
    }
    QTextStream out(&tempFile);

    // Write the full hosts file header as a raw string literal
    out << R"(# Copyright (c) 1993-2009 Microsoft Corp.
#
# This is a sample HOSTS file used by Microsoft TCP/IP for Windows.
#
# This file contains the mappings of IP addresses to host names. Each
# entry should be kept on an individual line. The IP address should
# be placed in the first column followed by the corresponding host name.
# The IP address and the host name should be separated by at least one
# space.
#
# Additionally, comments (such as these) may be inserted on individual
# lines or following the machine name denoted by a '#' symbol.
#
# For example:
#
#      102.54.94.97     rhino.acme.com          # source server
#       38.25.63.10     x.acme.com              # x client host
#
# localhost name resolution is handled within DNS itself.
#	127.0.0.1       localhost
#	::1             localhost
)";

    int rowCount = _hostModel->rowCount();
    int colCount = _hostModel->columnCount();
    for (int row = 0; row < rowCount; ++row)
    {
        QStringList columns;
        for (int col = 0; col < colCount; ++col)
            columns << _hostModel->data(_hostModel->index(row, col)).toString();
        out << columns.join("\t") << "\n";
    }
    tempFile.close();

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
