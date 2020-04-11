#ifndef _DATAPROXYMODEL_H
#define _DATAPROXYMODEL_H

#include <QSortFilterProxyModel>

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
    void setBand(bool show5G, bool show24G);

private:
    bool showBand5G_;
    bool showBand24G_;
};

#endif // _DATAPROXYMODEL_H
