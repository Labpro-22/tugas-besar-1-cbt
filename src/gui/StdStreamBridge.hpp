#ifndef GUI_STD_STREAM_BRIDGE_HPP
#define GUI_STD_STREAM_BRIDGE_HPP

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <streambuf>
#include <string>

class QueueInputBuffer : public std::streambuf {
public:
    QueueInputBuffer();

    void submitLine(const std::string& line);
    void close();

protected:
    int_type underflow() override;

private:
    std::mutex mutex;
    std::condition_variable condition;
    std::deque<char> queuedChars;
    char currentChar;
    bool closed;
};

class CallbackOutputBuffer : public std::streambuf {
public:
    using Callback = std::function<void(const std::string&)>;

    CallbackOutputBuffer();

    void setCallback(Callback callback);

protected:
    int_type overflow(int_type ch) override;
    int sync() override;

private:
    void flushPending();

    std::mutex mutex;
    Callback callback;
    std::string pending;
};

class StdStreamRedirector {
public:
    StdStreamRedirector(QueueInputBuffer& inputBuffer,
                        CallbackOutputBuffer& outputBuffer);
    ~StdStreamRedirector();

    StdStreamRedirector(const StdStreamRedirector&) = delete;
    StdStreamRedirector& operator=(const StdStreamRedirector&) = delete;

private:
    std::streambuf* oldCin;
    std::streambuf* oldCout;
    std::streambuf* oldCerr;
    std::streambuf* oldClog;
};

#endif
