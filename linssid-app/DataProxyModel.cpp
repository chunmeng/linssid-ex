#include "DataProxyModel.h"
#include "Custom.h"
#include "Logger.h"
#include "Utils.h"
#include <iostream>
#include <regex>
#include <stdexcept>
#include <tuple>
#include <vector>

using namespace std;
extern Logger AppLogger;

namespace {

std::tuple<bool, std::pair<int, int>> toBucket(const string& s1, const string& s2)
{
    try {
        int lowBound = std::stoi(s1);
        int highBound = std::stoi(s2);
        if (lowBound > highBound) {
            std::swap(lowBound, highBound);
        }
        return make_tuple(true, make_pair(lowBound, highBound));
    } catch (const exception& e) {
        // consume quitely
    }
    return make_tuple(false, make_pair(0, 0));
}

};

class DataProxyModel::Impl {
public:
    Impl() {
        channelBuckets.reserve(20);
        updateChannelBuckets(state.channels);
    };

public:
    void updateChannelBuckets(const string& channels);
    bool isChannelInBuckets(int channel) const;
    bool acceptChannel(int channel) const;
    bool acceptSsid(const string &ssid) const;
    bool acceptMac(const string &mac) const;

public:
    // The Proxy owns the real filter options
    FilterState state;
    vector<pair<int, int>> channelBuckets;
};

void DataProxyModel::Impl::updateChannelBuckets(const string& channels)
{
    // Format: m,n,m-n
    channelBuckets.clear();
    std::regex rangeRe("(\\d+)-(\\d+)");
    std::smatch match;
    auto tokens = Utils::split(channels, ',');
    for (auto& t : tokens) {
        tuple<bool, pair<int, int>> bucket;
        if (std::regex_match(t, match, rangeRe)) { // check if range
            // The first sub_match is the whole string; the next sub_match is the first parenthesized expression.
            if (match.size() == 3) {
                bucket = toBucket(match[1], match[2]);
            }
        } else if (!t.empty()) {
            bucket = toBucket(t, t);
        }
        if (std::get<0>(bucket)) {
            VerboseLog(AppLogger) << "Add bucket: " << t << " -> [" << std::get<1>(bucket).first << ", " << std::get<1>(bucket).second << "]";
            channelBuckets.push_back(move(std::get<1>(bucket)));
        }
    }
    // @TODO: Make buckets non-overlap for more efficient search? (Maybe not worth the complexity for a small size buckets...)
    std::sort(channelBuckets.begin(), channelBuckets.end());
}

bool DataProxyModel::Impl::isChannelInBuckets(int channel) const
{
    for (auto& p : channelBuckets) {
        // The buckets is sorted, so if less than low bound, then skip the rest
        if (channel < p.first) return false;
        if (channel >= p.first && channel <= p.second) return true; // Found the bucket with the channel, done
    }
    return false;
}

bool DataProxyModel::Impl::acceptChannel(int channel) const
{
    if (state.byBand) {
        if (channel <= 14 && !state.showBand24G) return false;
        if (channel > 14 && !state.showBand5G) return false;
    }
    if (state.byChannel) {
        if (!isChannelInBuckets(channel)) return false;
    }
    return true;
}

bool DataProxyModel::Impl::acceptSsid(const string& ssid) const
{
    if (state.bySsid) {
        if (state.ssid.empty()) return true;    // Accept every ssid
        if (ssid.find(state.ssid) != std::string::npos) return true; // Found in ssid, accept it
        return false;
    }
    return true;
}

bool DataProxyModel::Impl::acceptMac(const string& mac) const
{
    if (state.byMac) {
        if (state.mac.empty()) return true;    // Accept anything
        if (state.mac.size() != mac.size()) return false;   // Can't compare, bail
        // Accept if match
        for (std::string::size_type i = 0; i < state.mac.size(); ++i) {
            if (state.mac[i] == '?' || state.mac[i] == '-' || state.mac[i] == ':') continue;
            if (tolower(state.mac[i]) != tolower(mac[i])) return false;
        }
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
    if (impl_->state.channels != state.channels)
        impl_->updateChannelBuckets(state.channels);
    impl_->state = state; // copy whole
    invalidateFilter();
}

const FilterState& DataProxyModel::getFilter()
{
    return impl_->state;
}

// \return false to filter out row after checking criteria
bool DataProxyModel::filterAcceptsRow(int sourceRow,
                                  const QModelIndex &sourceParent) const
{
    QModelIndex channelIndex = sourceModel()->index(sourceRow, CHANNEL, sourceParent);
    int channel = sourceModel()->data(channelIndex).toInt();
    if (!impl_->acceptChannel(channel)) return false;

    QModelIndex ssidIndex = sourceModel()->index(sourceRow, SSID, sourceParent);
    auto ssid = sourceModel()->data(ssidIndex).toString();
    if (!impl_->acceptSsid(ssid.toStdString())) return false;

    QModelIndex macIndex = sourceModel()->index(sourceRow, MAC, sourceParent);
    auto mac = sourceModel()->data(macIndex).toString();
    if (!impl_->acceptMac(mac.toStdString())) return false;
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
