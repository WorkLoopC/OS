gdtr DW 0 
     DQ 0 

setGdt:
   MOV   [gdtr], DI
   MOV   [gdtr+2], RSI
   LGDT  [gdtr]
   RET