CANShield 2017
Buildt for the purpose of sending and receiving CAN-messages 
from a computer or Raspberry Pi to the CANbus of the FuelFighter vehicle. 

To use the shield you must format your messages in a certain way. 
It is only built to receive and transmit CAN-messages. 
A CAN-message has an ID field, Length of message field and 8bytes of date. 

The message format should be on this form:
"[7FF:8:CAFEBABE12345678]\n" 

Received CAN-messages will be transmittet to the Pi through the serial port in the same format.

One character in the message is 4 bits. The address field has a maximum of 11bits, 
with 7FF as the highest and 0 as the lowest address. The lower the address the higher the priority of 
the message. 

The second information is the length of the packet. This can be from 0 to 8. 
The rest of the message can go from blank if the length is 0, to FFFFFFFFFFFFFFFF if the length is 8.

The messages must be transmittet as ASCII characters.

The current baudrate of the serial connection is set to 19200. 8 bit characters with 1 end bit and no pairity. 
