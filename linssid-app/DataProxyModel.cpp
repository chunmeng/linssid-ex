#include "DataProxyModel.h"
#include "Custom.h"
#include <iostream>
#include <vector>

using namespace std;

class DataProxyModel::Impl {
public:
    Impl() = default;

public:
    bool isChannelIncluded(int channel) const;
    bool acceptChannel(int channel) const;

public:
    FilterState state;
    vector<pair<int, int>> channelBuckets;
};

bool DataProxyModel::Impl::isChannelIncluded(int channel) const
{
    // @TODO: Convert the channel string to internal format for better comparison?
    return false;
}

bool DataProxyModel::Impl::acceptChannel(int channel) const
{
    if (state.byBand) {
        if (channel <= 14 && !state.showBand24G) return false;
        if (channel > 14 && !state.showBand5G) return false;
    }
    if (state.byChannel) {
        if (!isChannelIncluded(channel)) return false;
    }
    return true;
}

DataProxyModel::DataProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , impl_(new Impl())
{
}

DataProxyModel::~DataProxyModel() = default;

void DataProxyModel::setFilter(const FilterState& state)
{
    impl_->state = state; // copy whole
    invalidateFilter();
}

// \return false to filter out row after checking criteria
bool DataProxyModel::filterAcceptsRow(int sourceRow,
                                  const QModelIndex &sourceParent) const
{
    QModelIndex channelIndex = sourceModel()->index(sourceRow, CHANNEL, sourceParent);
    int channel = sourceModel()->data(channelIndex).toInt();
    if (!impl_->acceptChannel(channel)) return false;
    // @TODO: Filter by mac, ssid, etc
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
