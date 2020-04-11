# linssid

This is a copy of the LinSSID (https://sourceforge.net/projects/linssid/) with intention to extend some functionalities.

## New Features Added
- New columns for BSS load and station count
- Status bar with counts of total, hidden and open AP
- Show ESSID as label on plot, align to the marker
- Filter the BSS list by band

![ScreenShot](/screenshots/latest.png?raw=true "Current Application View")

## Future Features
- Filter by MAC
- Filter by SSID
- Filter by channel
- Update chart/plots based on filter result
- Save filter setting to preference

## Build Requirement

Install deps and build according to insruction in linssid-app/INSTALL.txt

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

## UI change
For UI layout change, consider using Qt designer

```
sudo apt-get install qttools5-dev-tools
```
