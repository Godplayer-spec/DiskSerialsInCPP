#include <iostream>
#include <windows.h>
#include <winioctl.h>
#include <string>
#include <iomanip>
#include <curl/curl.h>
void DiskSerials() {
    for (int driveNumber = 0; driveNumber < 10; ++driveNumber) { // Check multiple drives
        std::wstring drivePath = L"\\\\.\\PhysicalDrive" + std::to_wstring(driveNumber);

        HANDLE hDevice = CreateFile(drivePath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (hDevice == INVALID_HANDLE_VALUE) {
            break;
        }

        STORAGE_PROPERTY_QUERY query = {};
        query.PropertyId = StorageDeviceProperty;
        query.QueryType = PropertyStandardQuery;

        STORAGE_DESCRIPTOR_HEADER header = { 0 };
        DWORD bytesReturned = 0;

        if (!DeviceIoControl(hDevice,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &query, sizeof(query),
            &header, sizeof(header),
            &bytesReturned, NULL)) {
            std::wcerr << L"Failed to get buffer size for " << drivePath << L": " << GetLastError() << std::endl;
            CloseHandle(hDevice);
            continue;
        }

        std::unique_ptr<BYTE[]> buffer(new BYTE[header.Size]);
        if (!DeviceIoControl(hDevice,
            IOCTL_STORAGE_QUERY_PROPERTY,
            &query, sizeof(query),
            buffer.get(), header.Size,
            &bytesReturned, NULL)) {
            std::wcerr << L"Failed to get device descriptor for " << drivePath << L": " << GetLastError() << std::endl;
            CloseHandle(hDevice);
            continue;
        }

        STORAGE_DEVICE_DESCRIPTOR* deviceDescriptor = reinterpret_cast<STORAGE_DEVICE_DESCRIPTOR*>(buffer.get());

        if (deviceDescriptor->SerialNumberOffset) {
            char* serialNumber = reinterpret_cast<char*>(buffer.get() + deviceDescriptor->SerialNumberOffset);
            std::wcout << L"Drive " << drivePath << L" Serial Number: " << serialNumber << std::endl;
        }
        else {
            std::wcout << L"Drive " << drivePath << L" has no serial number." << std::endl;
        }

        CloseHandle(hDevice);
    }
}
void MonSerials() {
    // Get a handle to the primary monitor
    HWND hwnd = GetConsoleWindow(); // Get a handle to the console window
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

    if (hMonitor == NULL) {
        std::cerr << "Failed to get monitor handle." << std::endl;
        return;
    }

    // Create a MONITORINFO structure
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO); // Set the size of the structure

    // Retrieve information about the monitor
    if (GetMonitorInfo(hMonitor, &mi)) {
        // Successfully retrieved monitor information
        std::cout << "Monitor Information:" << std::endl;
        std::cout << "Monitor Width: " << (mi.rcMonitor.right - mi.rcMonitor.left) << std::endl;
        std::cout << "Monitor Height: " << (mi.rcMonitor.bottom - mi.rcMonitor.top) << std::endl;
        std::cout << "Monitor Position: (" << mi.rcMonitor.left << ", " << mi.rcMonitor.top << ")" << std::endl;
        std::cout << "Monitor is Primary: " << ((mi.dwFlags & MONITORINFOF_PRIMARY) ? "Yes" : "No") << std::endl;

        // If you want to use MONITORINFOEX to get the monitor name
        MONITORINFOEX miEx;
        miEx.cbSize = sizeof(MONITORINFOEX);
        if (GetMonitorInfo(hMonitor, &miEx)) {
            std::wcout << "Monitor Name: " << miEx.szDevice << std::endl;
        }
    }
    else {
        // Failed to retrieve monitor information
        std::cerr << "GetMonitorInfo failed. Error code: " << GetLastError() << std::endl;
    }
}

void DumpSMBIOSTable() {
    // dynamic allocated buffer size. Don't want to lose precious data if it's too small or cause system crashes. If used too much space.

    DWORD bufferSize = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
    if (bufferSize == 0) {
        std::cerr << "Failed to retrieve SMBIOS table size." << std::endl;
        return;
    }

    // Initializing the buffer dynamically based on the retrieved size
    BYTE* buffer = new BYTE[bufferSize];
    if (buffer == nullptr) {
        std::cerr << "Failed to allocate memory for SMBIOS table." << std::endl;
        return;
    }

    // Now, retrieve the actual SMBIOS table data
    if (GetSystemFirmwareTable('RSMB', 0, buffer, bufferSize) == 0) {
        std::cerr << "Failed to retrieve SMBIOS table data." << std::endl;
        delete[] buffer; // Don't forget to free the memory if it fails
        return;
    }


    if (bufferSize > 0) {

        std::cout << "SMBIOS Table Contents (Hex Dump):" << std::endl;
        for (DWORD i = 0; i < bufferSize; ++i) {
            if (i % 16 == 0) {
                std::cout << std::endl << std::setw(4) << std::setfill('0') << std::hex << i << ": ";
            }
            std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buffer[i]) << " ";
        }
        std::cout << std::endl;

        
    }
    else {
        std::cerr << "Failed to retrieve SMBIOS table or buffer size exceeded." << std::endl;
    }
}



int main() {
    DumpSMBIOSTable();
    system("pause");
    return 0;
}


