#ifndef HOSTMODEL_H
#define HOSTMODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QVariant>
#include <vector>
#include <QFile>
#include <QRegularExpression>

struct Host
{
    bool enabled;
    QString ip;
    QString domain;
};

class HostModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    HostModel(QObject *parent = nullptr) : QAbstractTableModel(parent)
    {}

    HostModel(const QString &path, QObject *parent = nullptr) : QAbstractTableModel(parent)
    {
        loadFromFile(path);
    }

    bool loadFromFile(const QString &path)
    {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        QTextStream in(&file);
        std::vector<Host> newHosts;
        static const QRegularExpression re("\\s+");

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.isEmpty() || line.startsWith("#"))
            {
                if (!line.startsWith("#!"))
                    continue;
            }

            bool enabled = true;

            if (line.startsWith("#!")) {
                enabled = false;
                line = line.mid(2).trimmed();  // Remove "#!" prefix
            }

            const QStringList parts = line.split(re, Qt::SkipEmptyParts);
            if (parts.size() >= 2)
                newHosts.push_back({ enabled, parts[0], parts[1] });
        }

        file.close();

        beginResetModel();
        _hosts = std::move(newHosts);
        endResetModel();

        return true;
    }

    bool saveToFile(const QString &path) const
    {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QTextStream out(&file);

        // Optional header — could be stored elsewhere
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

        for (const Host &h : _hosts)
            out << (h.enabled ? "" : "#!") << h.ip << "\t" << h.domain << "\n";

        file.close();
        return true;
    }

    void setHostData(const std::vector<Host> &data)
    {
        beginResetModel();
        _hosts = data;
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : _hosts.size();
    }

    bool insertRow(int row, const QModelIndex &parent = QModelIndex())
    {
        if (row < 0 || row > _hosts.size())
            return false;

        beginInsertRows(parent, row, row);
        _hosts.insert(_hosts.begin() + row, {true, "", ""});
        endInsertRows();

        return true;
    }

    void removeRow(int row, const QModelIndex &parent = QModelIndex())
    {
        if (row < 0 || row >= _hosts.size())
            return;

        beginRemoveRows(parent, row, row);
        _hosts.erase(_hosts.begin() + row);
        endRemoveRows();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        return parent.isValid() ? 0 : 3; // enabled, ip, domain
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        const Host &host = _hosts.at(index.row());
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case 1: return host.ip;
            case 2: return host.domain;
            default: return QVariant();
            }

        case Qt::CheckStateRole:
            if (index.column() == 0)
                return host.enabled ? Qt::Checked : Qt::Unchecked;
            break;

        default:
            break;
        }

        return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (!index.isValid())
            return false;

        Host &host = _hosts[index.row()];

        if (index.column() == 0 && role == Qt::CheckStateRole) {
            host.enabled = (value.toInt() == Qt::Checked);
            emit dataChanged(index, index, {role});
            return true;
        }

        if (role == Qt::EditRole) {
            switch (index.column()) {
            case 1:
                host.ip = value.toString();
                break;
            case 2:
                host.domain = value.toString();
                break;
            default:
                return false;
            }

            emit dataChanged(index, index, {role});
            return true;
        }

        return false;
    }


    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return "";
            case 1: return "Ip";
            case 2: return "Domain";
            default: return QVariant();
            }
        }
        return QVariant();
    }

    // Allow edition
    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        if (!index.isValid())
            return Qt::NoItemFlags;

        Qt::ItemFlags defaultFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

        if (index.column() == 0)
            return defaultFlags | Qt::ItemIsUserCheckable;
        else
            return defaultFlags | Qt::ItemIsEditable;
    }

private:
    std::vector<Host> _hosts;
};

#endif // HOSTMODEL_H
