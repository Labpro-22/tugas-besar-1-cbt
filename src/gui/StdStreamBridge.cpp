#include "gui/StdStreamBridge.hpp"

#include <iostream>

QueueInputBuffer::QueueInputBuffer() : currentChar('\0'), closed(false) {}

void QueueInputBuffer::submitLine(const std::string& line) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (char ch : line) {
            queuedChars.push_back(ch);
        }
        queuedChars.push_back('\n');
    }
    condition.notify_one();
}

bool QueueInputBuffer::readLine(std::string& line) {
    line.clear();
    std::unique_lock<std::mutex> lock(mutex);

    while (true) {
        condition.wait(lock, [this]() { return closed || !queuedChars.empty(); });

        while (!queuedChars.empty()) {
            const char ch = queuedChars.front();
            queuedChars.pop_front();
            if (ch == '\n') {
                return true;
            }
            line.push_back(ch);
        }

        if (closed) {
            return !line.empty();
        }
    }
}

void QueueInputBuffer::close() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        closed = true;
    }
    condition.notify_all();
}

QueueInputBuffer::int_type QueueInputBuffer::underflow() {
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this]() { return closed || !queuedChars.empty(); });

    if (queuedChars.empty()) {
        return traits_type::eof();
    }

    currentChar = queuedChars.front();
    queuedChars.pop_front();
    setg(&currentChar, &currentChar, &currentChar + 1);
    return traits_type::to_int_type(currentChar);
}

CallbackOutputBuffer::CallbackOutputBuffer() : callback(), pending() {}

void CallbackOutputBuffer::setCallback(Callback callbackFn) {
    std::lock_guard<std::mutex> lock(mutex);
    callback = std::move(callbackFn);
}

CallbackOutputBuffer::int_type CallbackOutputBuffer::overflow(int_type ch) {
    if (traits_type::eq_int_type(ch, traits_type::eof())) {
        return traits_type::not_eof(ch);
    }

    {
        std::lock_guard<std::mutex> lock(mutex);
        pending.push_back(traits_type::to_char_type(ch));
    }

    if (traits_type::to_char_type(ch) == '\n') {
        flushPending();
    }

    return ch;
}

int CallbackOutputBuffer::sync() {
    flushPending();
    return 0;
}

void CallbackOutputBuffer::flushPending() {
    Callback currentCallback;
    std::string chunk;

    {
        std::lock_guard<std::mutex> lock(mutex);
        if (pending.empty()) {
            return;
        }

        chunk.swap(pending);
        currentCallback = callback;
    }

    if (currentCallback) {
        currentCallback(chunk);
    }
}

StdStreamRedirector::StdStreamRedirector(QueueInputBuffer& inputBuffer,
                                         CallbackOutputBuffer& outputBuffer)
    : oldCin(std::cin.rdbuf(&inputBuffer)),
      oldCout(std::cout.rdbuf(&outputBuffer)),
      oldCerr(std::cerr.rdbuf(&outputBuffer)),
      oldClog(std::clog.rdbuf(&outputBuffer)) {}

StdStreamRedirector::~StdStreamRedirector() {
    std::cin.rdbuf(oldCin);
    std::cout.rdbuf(oldCout);
    std::cerr.rdbuf(oldCerr);
    std::clog.rdbuf(oldClog);
}
