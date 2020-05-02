#include "DataProxyModel.h"
#include "Custom.h"
#include <iostream>

using namespace std;

DataProxyModel::DataProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void DataProxyModel::setFilter(const FilterState& state)
{
    state_ = state; // copy whole
    invalidateFilter();
}

// \return false to filter out row after checking criteria
bool DataProxyModel::filterAcceptsRow(int sourceRow,
                                  const QModelIndex &sourceParent) const
{
    QModelIndex channelIndex = sourceModel()->index(sourceRow, CHANNEL, sourceParent);
    int channel = sourceModel()->data(channelIndex).toInt();
    if (state_.byBand) {
        if (channel <= 14 && !state_.showBand24G) return false;
        if (channel > 14 && !state_.showBand5G) return false;
    }
    return true;
}

QVariant DataProxyModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    return sourceModel()->headerData(section, orientation,
                                     role);
}

bool DataProxyModel::isFiltered(int sourceRow) const
{
    if (!mapFromSource(sourceModel()->index(sourceRow, 0)).isValid()) // source model row is filtered
        return true;
    return false;
}
