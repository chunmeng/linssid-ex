#ifndef _DATAPROXYMODEL_H
#define _DATAPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <memory>
#include <string>

// Currently this hold the default value and every instances starts
// with the same default.
// @TODO:
// The application should hold one copy of this instance, load from a config
// and pass it around
struct FilterState {
    bool byBand = true;
    bool showBand5G = true;
    bool showBand24G = true;
    bool byChannel = false;
    std::string channels = "1-165";
    bool bySsid = false;
    std::string ssid = "";
    bool byMac = false;
    std::string mac = "";
};

class DataProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    DataProxyModel(QObject* parent = 0);
    ~DataProxyModel();

public:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role) const;
    bool isFiltered(int sourceRow) const;
    const FilterState& getFilter();
    void load(const std::string& file);
    void save(const std::string& file);

public slots:
    void setFilter(const FilterState& state);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif // _DATAPROXYMODEL_H
