#ifndef PLOTTERDEVICE_H
#define PLOTTERDEVICE_H

#include <string>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <Windows.h>

class PlotterDevice {
  public:
    // Constructor & Destructor
    PlotterDevice(std:: function < void(std::string) > dataReceivedCallback);
    ~PlotterDevice();

  // Public Methods
  void Open();
  void Open(std::wstring & port);
  void Close();
  void SendCommand(const std::string & command);

  private:
    // Private Methods
    void PerformOverlappedIo();
    void HandleRead(uint32_t bytes);
    void HandleResponse(std::string response);
    std::wstring FindPort() const;
    void Write(const std::string & data);

    // Private Members
    bool _connected;
    std:: function < void(std::string) > _dataReceivedCallback;
    HANDLE _portHandle;
    HANDLE _killEvent;
    HANDLE _readEvent;
    HANDLE _writeEnqueuedEvent;
    HANDLE _writeEvent;
    std::thread _workerThread;
    std::vector < BYTE > _readBuffer;
    std::vector < BYTE > _responseBuffer;
    std::queue < std::string > _writeQueue;
    std::mutex _writeQueueMutex;
};

#endif // PLOTTERDEVICE_H