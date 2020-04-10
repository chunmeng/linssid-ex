#include "DataProxyModel.h"
#include "Custom.h"
#include <iostream>

using namespace std;

DataProxyModel::DataProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , showBand5G_(true)
    , showBand24G_(true)
{
}

void DataProxyModel::setBand(bool show5G, bool show24G)
{
    showBand5G_ = show5G;
    showBand24G_ = show24G;
    invalidateFilter();
}

// \return false to filter out row after checking criteria
bool DataProxyModel::filterAcceptsRow(int sourceRow,
                                  const QModelIndex &sourceParent) const
{
    QModelIndex channelIndex = sourceModel()->index(sourceRow, CHANNEL, sourceParent);
    int channel = sourceModel()->data(channelIndex).toInt();
    if (channel <= 14 && !showBand24G_) return false;
    if (channel > 14 && !showBand5G_) return false;
    return true;
}

QVariant DataProxyModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    return sourceModel()->headerData(section, orientation,
                                     role);
}
