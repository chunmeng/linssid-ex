#ifndef _DATAPROXYMODEL_H
#define _DATAPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <string>

struct FilterState {
    bool byBand = true;
    bool showBand5G = true;
    bool showBand24G = true;
    bool byChannel = false;
    std::string channels = "-";
};

class DataProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    DataProxyModel(QObject* parent = 0);
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;
    bool isFiltered(int sourceRow) const;

public slots:
    void setFilter(const FilterState& state);

private:
    FilterState state_;
};

#endif // _DATAPROXYMODEL_H
