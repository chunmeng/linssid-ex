
#ifndef CUSTOM_EVENT_H
#define	CUSTOM_EVENT_H

#include <QEvent>

enum class CustomEvent {
    DataWanted = QEvent::User + 1,
    DataReady,
};

// Define the custom event subclass
class DataWantedEvent : public QEvent {
public:
    static auto Type() { return static_cast<QEvent::Type>(CustomEvent::DataWanted); }

    DataWantedEvent(const int blockNo)
        : QEvent(Type()), wantedBlockNo(blockNo) {
    }

    int getWantedBlockNo() const {
        return wantedBlockNo;
    }
private:
    int wantedBlockNo;
};

class DataReadyEvent : public QEvent {
public:
    static auto Type() { return static_cast<QEvent::Type>(CustomEvent::DataReady); }

    DataReadyEvent(const int blockNo)
        : QEvent(Type()), readyBlockNo(blockNo) {
    }

    int getReadyBlockNo() const {
        return readyBlockNo;
    }
private:
    int readyBlockNo;
};


#endif	/* CUSTOM_EVENT_H */