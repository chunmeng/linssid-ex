# linssid-ex

This is LinSSID with extended functionalities.
The initial source is based on LinSSID 3.6. (https://sourceforge.net/projects/linssid/).

## New Features Added
- New columns for BSS load and station count
- Status bar with counts of total, hidden and open AP
- Show ESSID as label on channel plot, align to the marker
- Show ESSID as label on time plot, align to last data point
- Fea: Added filtering to BSS list by band, plots based on filter result
- Fea: Added filter by channel
- Fea: Added filter by SSID
- Fea: Added filter by MAC
- Fea: Save filter setting to preference

![ScreenShot](/screenshots/latest.png?raw=true "Current Application View")

## TODO
- Fea: Add column for AP capability
- Fea: Add user preference/option to allow plotting of filtered row
- Fea: Stats - Busiest channels
- Enhancement: Look into channel filter string regex to prevent entries like (1-1-1,111111)
- Enhancement: textbox editing with validator is arkward (only allow 1 char change at a time)
- Enhancement: Add a button to flush/reset history (to clear out AP not seen for a long time)
- Enhancement: List BSSID as dropdown in MAC filter?

## Build Requirement

### Debian/Ubuntu

Install deps and build according to instruction in linssid-app/INSTALL.txt

```
sudo apt-get install -y \
	build-essential \
	libboost-regex-dev \
	qt5-default \
	qt5-qmake \
	libqt5svg5-dev \
	libqt5opengl5-dev \
	libqwt-qt5-6 \
	libqwt-headers
```

### Fedora

Install deps (no need to install weak deps):

```
sudo dnf -y --setopt=install_weak_deps=False install \
	boost-devel \
	qt5-qtbase-devel \
	qt5-qtsvg-devel \
	qwt-qt5-devel
```

Build:

```
qmake-qt5
make
sudo make install
```

## UI change
For UI layout change, consider using Qt designer

```
sudo apt-get install qttools5-dev-tools
```

## Packaging

### Unsigned
To package the unsigned deb, without consideration for release to any public ppa.
```
debuild -B -us -uc
```
The package is generated under ../ and can be installed by `sudo dpkg -i <PKG_DEB>` or via Software center.

__Note__ The package will conflict with original linssid package.
If failed to install, the existing linssid should be removed first.
`sudo apt remove linssid`

### Signed

To build a signed package as a maintainer, update `debian/changelog` with the entries for the new version.
You must have the pgp key for the latest email in the changelog.
Use `pgp --generate-key` to generate one if needed.

```
debuild -B
```
The package is output to: `../<version>_ubuntu1_amd64.deb`

### Clean up

Clean up intermediate files
```
dh clean
```

### Reference
- https://www.debian.org/doc/manuals/maint-guide/dreq.en.html
- https://www.debian.org/doc/manuals/maint-guide/build.en.html
- https://help.launchpad.net/Packaging/PPA/BuildingASourcePackage
- https://askubuntu.com/questions/737884/debuild-secret-key-not-available-someone-elses-key
