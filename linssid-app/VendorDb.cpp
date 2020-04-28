
#include "VendorDb.h"
#include "Custom.h"
#include "Logger.h"
#include <fstream>
#include <vector>

using namespace std;

extern Logger AppLogger;

class VendorDb::Impl {
public:
    struct VendorStruct {
        uint64_t ID;
        char blockMode;
        string name;
    };

public:
    Impl();

    std::string lookup(const string& mac);

private:
    void loadVendorDb();

private:
    int maxVendorRecL = 0;
    vector<VendorStruct> vendor;
};

VendorDb::Impl::Impl()
{
    loadVendorDb();
}

void VendorDb::Impl::loadVendorDb()
{
    // now deal with the vendor data array
    // first record is number of vendors then max record length
    ifstream vendorFile;
    vendorFile.open(VENDOR_FILE_NAME, ios::in);
    if (!vendorFile.is_open()) {
        ErrorLog(AppLogger) << "Error opening " << VENDOR_FILE_NAME << endl;
        return;
    }
    int numVendorsSink;
    vendorFile >> numVendorsSink >> maxVendorRecL;
    string tempString;
    // load vendor array with ID and name
    vendor.reserve(numVendorsSink + 2000);
    getline(vendorFile, tempString); // clear the end of line above
    while (getline(vendorFile, tempString)) {
        vendor.push_back({strtoul(tempString.substr(0,9).c_str(), nullptr, 16), tempString[9], tempString.substr(10)});
    }
    vendorFile.close();
    InfoLog(AppLogger) << "Loaded " << vendor.size() << " vendor entries" << endl;
}

std::string VendorDb::Impl::lookup(const string& mac)
{
    if (vendor.size() <= 0) {
        return "<nodb>";
    }
    int left = 0;
    int right = vendor.size() - 1;
    int mid;
    uint64_t key;
    uint64_t mask;
    char blockType;
    key = strtol((mac.substr(0, 2) + mac.substr(3, 2) + mac.substr(6, 2)
            + mac.substr(9,2) + mac.substr(12,1)).c_str(), nullptr, 16);
    while (left <= right) {
        mid = (int) ((left + right) / 2);
        blockType = vendor[mid].blockMode;
        if (blockType == 'L') mask = ~0x0000000000000FFF;
        else if (blockType == 'M') mask = ~0x00000000000000FF;
        else mask = ~0x0000000000000000; // block type Small
        if ((key & mask) == vendor[mid].ID) {
            return vendor[mid].name;
        } else if (key > vendor[mid].ID)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return "<unrecognized>";
}

VendorDb::VendorDb()
    : impl_(new Impl())
{
}

VendorDb::~VendorDb() = default;

std::string VendorDb::lookup(const string& mac)
{
    return impl_->lookup(mac);
}
