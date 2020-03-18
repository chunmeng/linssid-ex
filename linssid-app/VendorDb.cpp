
#include "VendorDb.h"
#include "Custom.h"
#include <fstream>
#include <iostream>

using namespace std;

class VendorDb::Impl {
public:
    struct vendorStruct {
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
    int numVendors = 0;
    int maxVendorRecL = 0;
    vendorStruct* vendor;
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
        // @TODO: Move this to a debug logging class
        std::cout << "Error opening " << VENDOR_FILE_NAME << endl;
        return;
    }
    vendorFile >> numVendors >> maxVendorRecL;
    string tempString;
    // load vendor array with ID and name
    vendor = new vendorStruct[numVendors];
    int vRecNo = 0;
    getline(vendorFile, tempString); // clear the end of line above
    while (getline(vendorFile, tempString)) {
        vendor[vRecNo].ID = strtol(tempString.substr(0,9).c_str(), nullptr, 16);
        vendor[vRecNo].blockMode = tempString[9];
        vendor[vRecNo].name = tempString.substr(10);
        vRecNo++;
    }
    vendorFile.close();
}

std::string VendorDb::Impl::lookup(const string& mac)
{
    if (numVendors <= 0) {
        return "<unrecognized>";
    }
    int left = 0;
    int right = numVendors - 1;
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
