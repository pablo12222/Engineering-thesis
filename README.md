# Development of an electronic optimizer to control pump operation.

## Introduction

This repository contains the work and resources related to my engineering thesis titled **"Development of an electronic optimizer to control pump operation."** 

---
## Abstract 

The engineering thesis focuses on the development and construction of an electronic
(microprocessor-based) model of a setpoint controller for regulating pump performance.
The device will be implemented using a selected microcontroller with appropriately
designed input/output interfaces to interact with external devices and facilitate user
communication. The system’s operating principle can be described as follows: the
controller receives an analog 0 − 10 V signal from a regulating valve, representing the
position of the valve ball. It then compares this value with the pump’s output setpoint.
If the signal from the valve is lower than the setpoint, the device reduces the output
signal by 0.1 V at regular time intervals determined by an encoder until the target value
is reached. Conversely, if the valve ball position value exceeds the setpoint, the
controller increases the output signal by 0.1 V at the specified time intervals. The
system is equipped with three encoders for configuring operating parameters, two analog
inputs 0 − 10 V, and one analog output 0 − 10 V, ensuring precise control and
adaptation of pump performance to operational requirements. The device operates in
two modes: automatic and manual. As part of the thesis, functional tests will be
conducted using available measurement instruments and voltage generators to verify the
correct operation and compliance of the device with the design assumptions. The task is
carried out in accordance with the guidelines and for the purposes of BELIMO -
BELIMO Siłowniki S.A.

--- 

## Keywords
- transmitter,
- electronic device,
- pump control,
- microprocessor system,
- building automation.


---

## Device
This is how the developed prototype looks like:

<div style="display: flex; justify-content: center; align-items: center; gap: 10px;">
    <img src="https://github.com/user-attachments/assets/22b89496-8368-42be-8161-844a3c092a8c" alt="Experimental Setup" width="45%">
    <img src="https://github.com/user-attachments/assets/7c2dbeb4-6ff1-4f2d-930f-6fe0be8f5013" alt="Experimental Result" width="45%">
</div>


---


## Systems hardware

Ideological diagram of the operation of the transmitter:

![schemat_ideowy](https://github.com/user-attachments/assets/c2d1209e-85c5-4ffe-b2f0-f4ce78c95f2d)

---

The PCB was made with autodesk EAGLE software:

![image](https://github.com/user-attachments/assets/13723be6-2ca8-4146-8920-11aca1bb97d5)
<div style="display: flex; justify-content: center; align-items: center; gap: 10px;">
    <img src="https://github.com/user-attachments/assets/67ad95a6-791a-4fd4-aaa4-e96194c8f045" alt="Experimental Result1" width="28%">
    <img src="https://github.com/user-attachments/assets/fd7f85c7-38e4-475d-854b-63d1b326d045" alt="Experimental Result2" width="35%">
    <img src="https://github.com/user-attachments/assets/9ef2dc24-f0ac-489c-9a90-6f6c858d2810" alt="Experimental Result3" width="33%">
</div>

---

The PCB enclosure was made in Autodesk Inventor:
<div style="display: flex; justify-content: center; align-items: center; gap: 10px;">
    <img src="https://github.com/user-attachments/assets/d58c9f94-f928-4e89-831e-e9584cb7545a" alt="Experimental Result1" width="28%">
    <img src="https://github.com/user-attachments/assets/674e85e5-04da-4aaa-bf7f-2dd34a674c00" alt="Experimental Result2" width="34%">
    <img src="https://github.com/user-attachments/assets/777b5f98-1d50-46c1-81da-2029f9b132ef" alt="Experimental Result3" width="33%">
</div>

---


