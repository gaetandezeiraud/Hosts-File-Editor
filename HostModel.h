#ifndef HOSTMODEL_H
#define HOSTMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVariant>
#include <vector>

struct Host
{
    QString ip;
    QString domain;
};

class HostModel : public QAbstractListModel
{
    Q_OBJECT

public:
    HostModel(QObject *parent = nullptr) : QAbstractListModel(parent)
    {}

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
        Host newHost;
        _hosts.insert(_hosts.begin() + row, newHost);
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
        return parent.isValid() ? 0 : 2; // ip, domain
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid())
            return QVariant();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            const Host &host = _hosts.at(index.row());
            switch (index.column()) {
                case 0: return host.ip;
                case 1: return host.domain;
                default: return QVariant();
            }
        }

        return QVariant();
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override
    {
        if (!index.isValid() || role != Qt::EditRole)
            return false;

        Host &host = _hosts[index.row()];

        switch (index.column()) {
        case 0:
            host.ip = value.toString();
            break;
        case 1:
            host.domain = value.toString();
            break;
        default:
            return false;
        }

        emit dataChanged(index, index, {role});
        return true;
    }


    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch (section) {
            case 0: return "Ip";
            case 1: return "Domain";
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

        return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }

private:
    std::vector<Host> _hosts;
};

#endif // HOSTMODEL_H
