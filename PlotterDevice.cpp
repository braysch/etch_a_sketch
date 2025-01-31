#include "PlotterDevice.h"
#include <array>
#include <functional>
#include <stdexcept>
#include "Setupapi.h"
const size_t ReadBufferSize = 1024;
static GUID GUID_DEVCLASS_PORTS = { 0x4d36e978, 0xe325, 0x11ce, 0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1,
,→ 0x03, 0x18 };
PlotterDevice::PlotterDevice(std::function<void(std::string)> dataReceivedCallback) :
_connected(false),
_dataReceivedCallback(dataReceivedCallback),
_portHandle(INVALID_HANDLE_VALUE),
_readBuffer(ReadBufferSize, 0),
_responseBuffer()
{
_killEvent = CreateEvent(nullptr, true, false, nullptr);
_readEvent = CreateEvent(nullptr, false, false, nullptr);
_writeEnqueuedEvent = CreateEvent(nullptr, false, false, nullptr);
_writeEvent = CreateEvent(nullptr, false, false, nullptr);
}
PlotterDevice::~PlotterDevice()
{
if (_connected)
{
Close();
}
}
void PlotterDevice::Close()
{
if (_portHandle != nullptr && _portHandle != INVALID_HANDLE_VALUE)
{
// Kill worker thread
SetEvent(_killEvent);
_workerThread.join(); // TODO This should probably be able to time out?
CloseHandle(_portHandle);
_portHandle = INVALID_HANDLE_VALUE;
}
}
void PlotterDevice::HandleRead(uint32_t bytes)
{
// Split the input into lines
for (auto i = 0u; i < bytes;)
{
// Check for a newline
const auto newline = static_cast<BYTE*>(memchr(&_readBuffer[i], '\n', bytes - i));
if (newline != nullptr)
{
// Since we found a newline, copy the section prior to the newline into the response
,→ buffer,
// then handle the response.
auto newline_index = static_cast<uint32_t>(newline - &_readBuffer[0]);
// If there's a '\r', ignore it
if (newline_index > 0 && _readBuffer[newline_index - 1] == '\r')
newline_index--;
_responseBuffer.insert(_responseBuffer.end(), &_readBuffer[i], newline);
if (!_responseBuffer.empty())
{
// Convert the response buffer to a string
const auto response_str = std::string(_responseBuffer.begin(),
,→ _responseBuffer.end());
HandleResponse(response_str);
}
// Consume the newline(s)
i = newline_index + 1;
while (i < bytes && (_readBuffer[i] == '\r' || _readBuffer[i] == '\n'))
i++;
// Reset the response buffer
_responseBuffer.clear();
}
else
{
// No newline found, so just append it to the response buffer.
_responseBuffer.insert(_responseBuffer.end(), &_readBuffer[i], &_readBuffer[bytes]);
i = bytes;
}
}
}
void PlotterDevice::HandleResponse(std::string response)
{
printf("[In] %s\r\n", response.c_str());
if (_dataReceivedCallback) _dataReceivedCallback(response);
}
void PlotterDevice::Open()
{
auto port_name = FindPort();
Open(port_name);
}
void PlotterDevice::Open(std::wstring& port)
{
    // Open the port
_portHandle = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE,
0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
if (_portHandle == nullptr || _portHandle == INVALID_HANDLE_VALUE)
throw std::runtime_error("Failed to open port.");
// Configure the serial port parameters (baudrate, data bits, etc...)
DCB dcb;
if (!GetCommState(_portHandle, &dcb)) throw std::runtime_error("GetCommState failed.");
dcb.BaudRate = CBR_115200;
dcb.ByteSize = 8;
dcb.StopBits = ONESTOPBIT;
dcb.Parity = NOPARITY;
if (!SetCommState(_portHandle, &dcb)) throw std::runtime_error("SetCommState failed.");
// Configure timeouts
COMMTIMEOUTS timeouts;
if (!GetCommTimeouts(_portHandle, &timeouts)) throw std::runtime_error("GetCommTimeouts
,→ failed.");
// Wait indefinitely for a byte, return immediately with any data
timeouts.ReadIntervalTimeout = MAXDWORD;
timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
timeouts.ReadTotalTimeoutConstant = MAXDWORD - 1;
timeouts.WriteTotalTimeoutConstant = 0;
timeouts.WriteTotalTimeoutMultiplier = 0;
if (!SetCommTimeouts(_portHandle, &timeouts)) throw std::runtime_error("SetCommTimeouts
,→ failed.");
// Start the worker thread
ResetEvent(_killEvent);
_workerThread = std::thread([this] {this->PerformOverlappedIo(); });
}
void PlotterDevice::SendCommand(const std::string& command)
{
Write(command + "\r\n");
}
void PlotterDevice::Write(const std::string& data)
{
std::lock_guard<std::mutex> guard(_writeQueueMutex);
_writeQueue.emplace(data);
SetEvent(_writeEnqueuedEvent);
}
void PlotterDevice::PerformOverlappedIo()
{
OVERLAPPED read_overlapped;
ZeroMemory(&read_overlapped, sizeof(OVERLAPPED));
read_overlapped.hEvent = _readEvent;
OVERLAPPED write_overlapped;
ZeroMemory(&write_overlapped, sizeof(OVERLAPPED));
write_overlapped.hEvent = _writeEvent;
auto read_pending = false;
auto write_pending = false;
std::array<HANDLE, 4> wait_handles =
{
_killEvent,
_writeEnqueuedEvent,
_readEvent,
_writeEvent
};
while (true)
{
// If there's not a read or write operation happening, start them
if (!read_pending)
{
const auto completed_synchronously = ReadFile(_portHandle, &_readBuffer[0],
,→ _readBuffer.size(), nullptr, &read_overlapped);
const auto error = GetLastError();
if (!completed_synchronously && error == ERROR_IO_PENDING)
{
read_pending = true;
}
else if (completed_synchronously)
{
}
else
{
throw std::runtime_error("Failed to start read.");
}
}
{
std::lock_guard<std::mutex> guard(_writeQueueMutex);
if (!write_pending && !_writeQueue.empty())
{
auto write_data = _writeQueue.front();
_writeQueue.pop();
// TODO We could release the lock here
// TODO Better error handling
WriteFile(_portHandle, write_data.c_str(), write_data.length(), nullptr,
,→ &write_overlapped);
const auto error = GetLastError();
if (error == ERROR_IO_PENDING)
{
write_pending = true;
}
else
{
throw std::runtime_error("Failed to start write.");
}
}
}
// Wait for something to happen
const auto wait_result = WaitForMultipleObjectsEx(wait_handles.size(), &wait_handles[0],
,→ false, INFINITE, true);
if (wait_result == WAIT_OBJECT_0) // Kill event, exit the thread
{
break;
}
else if (wait_result == WAIT_OBJECT_0 + 1) // Write enqueued
{
// We don't actually need to do anything here, looping around will trigger another read
}
else if (wait_result == WAIT_OBJECT_0 + 2) // Read complete
{
DWORD bytes_read;
if (!GetOverlappedResult(_portHandle, &read_overlapped, &bytes_read, false))
throw std::runtime_error("Read failed.");
HandleRead(bytes_read);
read_pending = false;
}
else if (wait_result == WAIT_OBJECT_0 + 3) // Write complete
{
DWORD bytes_written;
if (!GetOverlappedResult(_portHandle, &write_overlapped, &bytes_written, false))
throw std::runtime_error("Write failed.");
// printf("[Out] Wrote %d bytes\r\n", bytes_written);
write_pending = false;
}
}
}
std::wstring PlotterDevice::FindPort() const
{
auto found_device = false;
std::wstring device_path;
// Look for a port named "STMicroelectronics STLink Virtual COM Port"
const auto info_set = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, nullptr, nullptr,
,→ DIGCF_PRESENT);
if (!info_set) throw std::runtime_error("Failed to get class devices.");
SP_DEVINFO_DATA info_data;
ZeroMemory(&info_data, sizeof(info_data));
info_data.cbSize = sizeof(SP_DEVINFO_DATA);
// Enumerate devices in the set
for (auto i = 0u; SetupDiEnumDeviceInfo(info_set, i, &info_data); i++)
{
WCHAR property_data[100];
// Fetch the device FriendlyName
if (!SetupDiGetDeviceRegistryProperty(info_set, &info_data, SPDRP_FRIENDLYNAME, nullptr,
reinterpret_cast<BYTE*>(&property_data[0]), sizeof(property_data), nullptr))
throw std::runtime_error("SetupDIGetDeviceRegistryProperty failed.");
// Check for a match
if (wcsstr(property_data, L"STMicroelectronics STLink Virtual COM Port") != nullptr)
{
// Find the port number / device path
// TODO This should probably be replaced with a more elegant approach
WCHAR com_name[20];
auto com_start = wcsstr(property_data, L"(COM");
if (com_start == nullptr) throw std::runtime_error("Bad device name.");
com_start++; // Skip "(", though we need to search for (COM to prevent it from finding
,→ the other COM in the FriendlyName
const auto com_end = wcsstr(com_start, L")");
if (com_end == nullptr) throw std::runtime_error("Bad device name.");
wcsncpy_s(com_name, com_start, com_end - com_start);
device_path = L"\\\\.\\" + std::wstring(com_name);
found_device = true;
break;
}
}
const auto last_error = GetLastError();
if (last_error != 0 && last_error != ERROR_NO_MORE_DEVICES) throw
,→ std::runtime_error("SetupDiEnumDeviceInfo failed.");
if (!found_device) throw std::runtime_error("Failed to find STLink COM port.");
SetupDiDestroyDeviceInfoList(info_set);
return device_path;
}