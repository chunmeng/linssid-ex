#ifndef _DATAPROXYMODEL_H
#define _DATAPROXYMODEL_H

#include <QSortFilterProxyModel>

class DataProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    DataProxyModel(QObject* parent = 0);
    bool filterAcceptsRow(int source_row,
                          const QModelIndex &source_parent) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;

public slots:
    void setBand(bool show5G, bool show24G);

private:
    bool showBand5G_;
    bool showBand24G_;
};

#endif // _DATAPROXYMODEL_H
