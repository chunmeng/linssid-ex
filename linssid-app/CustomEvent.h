
#ifndef CUSTOM_EVENT_H
#define	CUSTOM_EVENT_H

#include <QEvent>

enum class CustomEvent {
    DataWanted = QEvent::User + 1,
    DataReady,
    DialogClosed,
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

const int FILTER_DIALOG_ID = 1;

class DialogClosedEvent : public QEvent {
public:
    static auto Type() { return static_cast<QEvent::Type>(CustomEvent::DialogClosed); }

    DialogClosedEvent(const int id)
        : QEvent(Type()), id_(id) {
    }

    int id() const {
        return id_;
    }
private:
    int id_;
};

#endif	/* CUSTOM_EVENT_H */