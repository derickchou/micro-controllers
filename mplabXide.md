  * MPLAB X [downloads](http://ww1.microchip.com/downloads/mplab/X/)
  * MPLAB [archives](http://www.microchip.com/stellent/idcplg?IdcService=SS_GET_PAGE&nodeId=1406&dDocName=en023073)
  * **`avrdude`** with **MPLAB X IDE**

in `project properties` of the project select `Building/Loading`, check `Execute this line after build` and paste this line:
```
/usr/bin/avrdude -C /home/PIC/mpide/hardware/tools/avrdude.conf \
                 -c stk500v2 -p ${Device} -D -b 115200\
                 -P /dev/ttyUSB0 -U flash:w:${ImagePath}:i
```
note avrdude gives an error about unknown `${Device}`, either change `-p ${Device}` to `-p pic32` or tweak `avrdude.conf`