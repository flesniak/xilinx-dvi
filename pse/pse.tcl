# Xilinx XPS TFT Controller
# OVP Model
# see datasheet Xilinx DS695

imodelnewperipheral -name xps_tft -vendor itiv.kit.edu -library peripheral -version 1.0 -constructor constructor -destructor destructor
iadddocumentation   -name Licensing   -text "Open Source Apache 2.0"
iadddocumentation   -name Description -text "Xilinx XPS TFT Controller"

  imodeladdbusslaveport -name BUS0 -size 16 -mustbeconnected
  imodeladdaddressblock -name AB0  -width 32 -offset 0 -size 16
  imodeladdmmregister   -name AR   -offset 0  -width 32 -access rw
    imodeladdfield        -name BASEADDR -bitoffset 20 -width 11 -access rw -mmregister BUS0/AB0/AR
      iadddocumentation -name Description -text "Base address of video memory on 2MB boundary"
  imodeladdmmregister   -name CR   -offset 4  -width 32 -access rw
    imodeladdfield        -name TDE -bitoffset 0  -width 1 -access rw -mmregister BUS0/AB0/CR
      iadddocumentation -name Description -text "TFT Display Enable"
    imodeladdfield        -name DPS -bitoffset 1  -width 1 -access rw -mmregister BUS0/AB0/CR
      iadddocumentation -name Description -text "Display Scan Control"
  imodeladdmmregister   -name IESR -offset 8  -width 32 -access rw
    imodeladdfield        -name ST  -bitoffset 0  -width 1 -access rw -mmregister BUS0/AB0/IESR
      iadddocumentation -name Description -text "Vsync and address latch status"
    imodeladdfield        -name IE  -bitoffset 3  -width 1 -access rw -mmregister BUS0/AB0/IESR
      iadddocumentation -name Description -text "Vsync interrupt enable"
  imodeladdmmregister   -name CCR  -offset 12 -width 32 -access rw
    imodeladdfield        -name CD  -bitoffset 0  -width 8 -access rw -mmregister BUS0/AB0/CCR
      iadddocumentation -name Description -text "Chrontel chip register data"
    imodeladdfield        -name CA  -bitoffset 8  -width 8 -access rw -mmregister BUS0/AB0/CCR
      iadddocumentation -name Description -text "Chrontel chip register address"
    imodeladdfield        -name ST  -bitoffset 31 -width 1 -access rw -mmregister BUS0/AB0/CCR
      iadddocumentation -name Description -text "Start transmission"

  imodeladdbusslaveport -name VMEMBUS -size 0x400000 -mustbeconnected
    iadddocumentation -name Description -text "Video memory, sized 2MB"
  imodeladdaddressblock -name VMEMAB  -size 0x400000 -port VMEMBUS -width 32 -offset 0
  imodeladdlocalmemory  -name VMEM    -size 0x400000 -addressblock VMEMBUS/VMEMAB -changefunction vmemChange
  