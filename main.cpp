//#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <tchar.h>
#include<conio.h>
#include<string>
#include<vector>

#define BUF_SIZE 2048


void GetComPortNames(std::vector<std::wstring>&comPortList )
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        TCHAR portDevice[256];
        TCHAR portName[256];
        DWORD portDeviceSize;
        DWORD portNameSize;
        DWORD type;
        DWORD index = 0;

        printf("Available COM ports and devices:\n");

        while (TRUE)
        {
            portDeviceSize = sizeof(portDevice);
            portNameSize = sizeof(portName);
            if (RegEnumValue(hKey, index++, portDevice, &portDeviceSize,
                NULL, &type, (LPBYTE)portName, &portNameSize) != ERROR_SUCCESS)
            {
                break;
            }

            if (type == REG_SZ)
            {
                //_tprintf(_T("%d). %s - %s\n"), index ,portName, portDevice);
                //comPortList.push_back(char (portName));
                comPortList.push_back(portName);
                
            }
        }
        RegCloseKey(hKey);
    }
}

void printComPortNames(std::vector<std::wstring> & comPortList){
    for (size_t i = 0; i < comPortList.size(); ++i)
    {
        #ifdef UNICODE
        // For Unicode builds (wchar_t)
            std::wcout << i + 1 << _T("). ") << comPortList[i].c_str() << std::endl;
        #else
        // For ANSI builds (char)
            std::cout << i + 1 << _T("). ") << comPortList[i].c_str() << std::endl;
        #endif
    }
}
int main(void)
{
    std::vector<std::wstring> comPortList;

    GetComPortNames(comPortList);
    printComPortNames(comPortList);
   



    HANDLE hComm;  // Handle to the Serial port
    BOOL   Status; // Status
    DCB dcbSerialParams = { 0 };  // Initializing DCB structure
    COMMTIMEOUTS timeouts = { 0 };  //Initializing timeouts structure

    char SerialBuffer[64] = { 0 }; //Buffer to send and receive data

    DWORD BytesWritten = 0;          // No of bytes written to the port
    DWORD dwEventMask;     // Event mask to trigger
    char  ReadData;        //temperory Character
    DWORD NoBytesRead;     // Bytes read by ReadFile()
    unsigned char loop = 0;
    wchar_t pszPortName[10] = { 0 }; //com port id
    wchar_t PortNo[20] = { 0 }; //contain friendly name


    //to read from serial com port

    char buffer[256];
    DWORD bytesRead;
    DWORD totalRead = 0;



    //Enter the com port id
    printf_s("Enter the Com Port: ");
    int option=0;
    scanf_s("%d", &option);
    if(option>comPortList.size()){
        std::cout << "give proper option";
        return 0;
    }
    else if (comPortList.size() > 0) {
        std::cout << "gonna create the handle";
        std::wstring port = L"\\\\.\\";
        port += comPortList[option - 1];

        LPCWSTR portName = port.c_str();
        portName;
        //Open the serial com port
        hComm = CreateFile(
            portName,                          //friendly name
            GENERIC_READ | GENERIC_WRITE,      // Read/Write Access
            0,                                 // No Sharing, ports cant be shared
            NULL,                              // No Security
            OPEN_EXISTING,                     // Open existing port only
            0,                                 // Non Overlapped I/O
            NULL);                             // Null for Comm Devices

        DWORD errMsg = GetLastError();
        if (hComm == INVALID_HANDLE_VALUE)
        {
            printf_s("\n Port can't be opened\n\n");
           

            if (errMsg == 2)
            {
                printf_s("\n nothinf is plugged in\n\n");
                goto Exit2;
            }
            else if (errMsg == 5)
            {
                printf_s("\nanother app is already using the port\n\n");
                goto Exit2;
            }
            goto Exit2;
        }
        else if (errMsg == 0) {

            COMMTIMEOUTS comm_timeouts;
            if (!GetCommTimeouts(hComm, &comm_timeouts)) {
                return 5;
            }

            comm_timeouts.ReadIntervalTimeout = 0;
            comm_timeouts.ReadTotalTimeoutMultiplier = 0;
            comm_timeouts.ReadTotalTimeoutConstant = 1;
            comm_timeouts.WriteTotalTimeoutMultiplier = 0;
            comm_timeouts.WriteTotalTimeoutConstant = 0;
            SetCommTimeouts(hComm, &timeouts);



            if (!SetCommTimeouts(hComm, &comm_timeouts)) {
                return 6;
            }


            //Setting the Parameters for the SerialPort
            dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
            Status = GetCommState(hComm, &dcbSerialParams); //retreives  the current settings
            if (Status == FALSE)
            {
                printf_s("\nError to Get the Com state\n\n");
                goto Exit1;
            }
            dcbSerialParams.BaudRate = CBR_9600;      //BaudRate = 9600
            dcbSerialParams.ByteSize = 8;             //ByteSize = 8
            dcbSerialParams.StopBits = ONESTOPBIT;    //StopBits = 1
            dcbSerialParams.Parity = NOPARITY;      //Parity = None
            Status = SetCommState(hComm, &dcbSerialParams);


            if (Status == FALSE)
            {
                printf_s("\nError to Setting DCB Structure\n\n");
                goto Exit1;
            }

            if (!SetCommMask(hComm, EV_RXCHAR | EV_ERR)) {
                printf("SetCommMask failed with error code: %ld\n", GetLastError());
                goto Exit2;
            }

            DWORD dwEvtMask;
         
           while(!(GetAsyncKeyState(VK_ESCAPE) & 0x01))
           {
               dwEvtMask = 0;// reset mask for each itereation

               // Wait for event with timeout
               if (!WaitCommEvent(hComm, &dwEvtMask, NULL))
               {
                   printf("WaitCommEvent failed: %d\n", GetLastError());
                   break;
               }

               if (dwEvtMask & EV_ERR)
               {
                   printf("Port error: %d\n", GetLastError());
                   break;
               }



               if (dwEvtMask & EV_RXCHAR)
               {
                   bytesRead = 0;
                   if (!ReadFile(hComm, buffer, sizeof(buffer), &bytesRead, NULL))
                   {
                       if (GetLastError() != ERROR_IO_PENDING)
                       {
                           printf("Read error: %d\n", GetLastError());
                           break;
                       }
                   }

                   if (bytesRead > 0)
                   {
                       // Process the new data immediately
                       printf("Received %lu bytes: %.*s\n", bytesRead, bytesRead, buffer);

                       // Clear buffer for next read
                       memset(buffer, 0, sizeof(buffer));
                       totalRead = 0;
                   }
               }
              

           }
        
           

        }
        Exit1:
            CloseHandle(hComm);//Closing the Serial Port
        Exit2:
            system("pause");
        }
    
    return 0;
}
