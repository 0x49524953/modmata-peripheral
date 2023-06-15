Modbus Library for Arduino
==========================

This library allows your Arduino to communicate via Modbus protocol. The Modbus is a Controller-Peripheral protocol
used in industrial automation and can be used in other areas, such as home automation.

The Modbus generally uses serial RS-232 or RS-485 as physical layer (then called Modbus Serial) and
TCP/IP via Ethernet or WiFi (Modbus IP).


http://pt.wikipedia.org/wiki/Modbus http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b.pdf
http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf

<br>

Features
========

<ul>
<li>Operates as a peripheral device </li>
<li>Supports Modbus Serial (RS-232 or RS485)</li>
<li>Reply exception messages for all supported functions</li>
<li>Modbus functions supported:</li>
<ul>
    <li>0x01 - Read Coil Registers</li>
    <li>0x02 - Read Discrete Registers</li>
    <li>0x03 - Read Holding Registers</li>
    <li>0x04 - Read Input Registers</li>
    <li>0x05 - Write Single Coil Register</li>
    <li>0x06 - Write Single Holding Register</li>
    <li>0x0F - Write Multiple Coil Registers</li>
    <li>0x10 - Write Multiple Holding Registers</li>
    <li>0x41 - Arduino's built-in <code>pinMode()</code> function</li>
    <li>0x42 - Arduino's built-in <code>digitalRead()</code> function</li>
    <li>0x42 - Arduino's built-in <code>digitalWrite()</code> function</li>
    <li>0x42 - Arduino's built-in <code>analogRead()</code> function</li>
    <li>0x42 - Arduino's built-in <code>analogWrite()</code> function</li>
</ul>
</ul>

---
<br>

How to
======


<h2>Modbus Jargon</h2>

In this library was decided to use the terms used in Modbus to the methods names, then is important clarify the names of
register types:

| Register type        | Data Type          | Access            | Library methods       |
| -------------------- | ------------------ | ----------------- | --------------------- |
| Coil Register        | Boolean Value      | Read/Write        |      |
| Holding Register     | 16-bit Word        | Read/Write        |      |
| Discrete Register    | Boolean Value      | Read Only         |      |
| Input Register       | 16-bit Word        | Read Only         |      |

---
<br>


Other Modbus libraries
======================


<b>Arduino Modbus RTU</b><br>
Author: Juan Pablo Zometa, Samuel and Marco Andras Tucsni<br>
Year: 2010<br>
Website: https://sites.google.com/site/jpmzometa/

<b>Simple Modbus</b><br>
Author: Bester.J<br>
Year: 2013 Website: https://code.google.com/p/simple-modbus/

<b>Arduino-Modbus slave</b><br>
Jason Vreeland [CodeRage]<br>
Year: 2010<br>
Website: http://code.google.com/p/arduino-modbus-slave/

<b>Mudbus (Modbus TCP)</b><br>
Author: Dee Wykoff<br>
Year: 2011<br>
Website: http://code.google.com/p/mudbus/

<b>ModbusMaster Library for Arduino</b><br>
Author: Doc Walker<br>
Year: 2012<br>
Website: https://github.com/4-20ma/ModbusMaster<br>
Website: http://playground.arduino.cc/Code/ModbusMaster

<br>

Contributions
=============
http://github.com/andresarmento/modbus-arduino<br>
prof (at) andresarmento (dot) com

<br>

License
=======
The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
