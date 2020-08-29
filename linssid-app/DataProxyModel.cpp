#include "DataProxyModel.h"
#include "Custom.h"
#include "Logger.h"
#include "Utils.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>

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
    return sourceModel()->headerData(section, orientation, role);
}

bool DataProxyModel::isFiltered(int sourceRow) const
{
    if (!mapFromSource(sourceModel()->index(sourceRow, 0)).isValid()) // source model row is filtered
        return true;
    return false;
}

void DataProxyModel::load(const string& file)
{
    InfoLog(AppLogger) << "Load filters from " << file << endl;
    fstream prefs;
    prefs.open(file, ios::in);
    if (!prefs.is_open()) { // no prefs file, so return default
        InfoLog(AppLogger) << "Filter setting doesn't exist, default will be used" << endl;
        return;
    }
    string line;
    while (getline(prefs, line)) {
        // split line into key=val - this allow = in the filter text (e.g filter.ssid.text=SomeSsid=)
        vector<string> kv;
        auto pos = line.find('=');
        if (pos == string::npos) continue;    // Default value for key without value
        if (pos + 1 >= line.length()) continue;
        kv.push_back(line.substr(0, pos));
        kv.push_back(line.substr(pos+1));

        DebugLog(AppLogger) << "Load " << kv[0] << "=" << kv[1] << endl;
        // @TODO: Use tag dispatcher to set corresponding value
        if (kv[0] == "filter.band") impl_->state.byBand = (kv[1] == "0") ? false : true;
        else if (kv[0] == "filter.band.2g") impl_->state.showBand24G = (kv[1] == "0") ? false : true;
        else if (kv[0] == "filter.band.5g") impl_->state.showBand5G = (kv[1] == "0") ? false : true;
        else if (kv[0] == "filter.channel") impl_->state.byChannel = (kv[1] == "0") ? false : true;
        else if (kv[0] == "filter.channel.text") impl_->state.channels = kv[1];
        else if (kv[0] == "filter.ssid") impl_->state.bySsid = (kv[1] == "0") ? false : true;
        else if (kv[0] == "filter.ssid.text") impl_->state.ssid = kv[1];
        else if (kv[0] == "filter.mac") impl_->state.byMac = (kv[1] == "0") ? false : true;
        else if (kv[0] == "filter.mac.text") impl_->state.mac = kv[1];
    }
    prefs.close();
}

// Simple serialization format
// filter.band=1
// filter.band.2g=1
// filter.band.5g=1
// filter.ssid=1
// filter.ssid.text="abc"...
void DataProxyModel::save(const std::string& file)
{
    InfoLog(AppLogger) << "Save filters to " << file << endl;
    extern struct passwd *realUser;
    ofstream prefs;
    prefs.open(file, ios::out);
    Utils::waste(chown(file.c_str(), realUser->pw_uid, realUser->pw_gid));
    chmod(file.c_str(), 00644);
    prefs << "filter.band=" << impl_->state.byBand << endl;
    prefs << "filter.band.2g=" << impl_->state.showBand24G << endl;
    prefs << "filter.band.5g=" << impl_->state.showBand5G << endl;
    prefs << "filter.channel=" << impl_->state.byChannel << endl;
    prefs << "filter.channel.text=" << impl_->state.channels << endl;
    prefs << "filter.ssid=" << impl_->state.bySsid << endl;
    prefs << "filter.ssid.text=" << impl_->state.ssid << endl;
    prefs << "filter.mac=" << impl_->state.byMac << endl;
    prefs << "filter.mac.text=" << impl_->state.mac << endl;
    prefs.close();
}