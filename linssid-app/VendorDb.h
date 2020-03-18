#ifndef _VENDORDB_H
#define	_VENDORDB_H

#include <memory>

class VendorDb {
public:
    VendorDb();
    ~VendorDb();

    std::string lookup(const std::string& mac);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif	/* _VENDORDB_H */