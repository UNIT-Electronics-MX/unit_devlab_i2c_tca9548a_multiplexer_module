"""
Firware de prueba para multiplexor TCA9545A
 RP2040
 I2C1
         

        A0   A1  tcaAddr
        0     0   0x70
        1     0   0x71
        0     1   0x72
        1     1   0x73
        
"""


import machine
#objeto I2C
tcaAddr = const(0x70) #direccion del multiplexor TCA9545A


i2c = machine.I2C(1, sda=machine.Pin(6), scl=machine.Pin(7), freq=400000)

i2c.writeto(tcaAddr,b'\ x01')
devices = i2c.scan()
for d in devices:
    if d != tcaAddr:
     print(hex(d))
i2c.writeto(tcaAddr,b'\ x02')
devices = i2c.scan()
for d in devices:
    if d != tcaAddr:
     print(hex(d))
i2c.writeto(tcaAddr,b'\ x03')
devices = i2c.scan()
for d in devices:
    if d != tcaAddr:
     print(hex(d))
i2c.writeto(tcaAddr,b'\ x04')
devices = i2c.scan()
for d in devices:
    if d != tcaAddr:
     print(hex(d))     