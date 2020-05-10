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

![ScreenShot](/screenshots/latest.png?raw=true "Current Application View")

## TODO
- Fea: Save filter setting to preference
- Fea: Add column for AP capability
- Fea: Add user preference/option to allow plotting of filtered row
- Enchancement: Look into channel filter string regex to prevent entries like (1-1-1,111111)
- Enchancement: Align the view filter dialog to main form
- Enchancement: textbox editing with validator is arkward (only allow 1 char change at a time)
- Release: Try make a deb package

## Build Requirement

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

## UI change
For UI layout change, consider using Qt designer

```
sudo apt-get install qttools5-dev-tools
```
