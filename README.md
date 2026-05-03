# SerialFileTransferTool
Serial file transfer between two offline Windows computers.

## Introduction
This programme provides a bidirectional file sharing between two Windows computers connected with a serial protocol.
The software is built using only the core Win32 API to maximise the compatibility with legacy systems, such as Windows 7 32-bit.
The circuit required is also straightforward; it can operate using only two USB-to-UART converters and three wires.
This project could replace the repetitive process of physically plugging and unplugging USB flash drives to move files. This might be useful for transferring build artefacts from a development machine to a target machine running vulnerable version of Windows. 

## Circuit Diagram
```
     [Computer 1]                  [Computer 2]
USB-to-UART converter (A)    USB-to-UART converter (B)
          TX ------------------------> RX
          RX <------------------------ TX
         GND <-----------------------> GND
```
> [!WARNING]
> Safety Warning: it is highly recommended to measure the potential difference between two GND terminals to ensure that their potential difference is at 0V. A significant voltage offset can permanently damage the USB-to-UART converters or the computers.
