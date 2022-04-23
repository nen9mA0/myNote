## MSDOS.asm

### 标准函数表

```asm
; Standard Functions
DISPATCH DW     ABORT           ;0
        DW      CONIN
        DW      CONOUT
        DW      READER
        DW      PUNCH
        DW      LIST            ;5
        DW      RAWIO
        DW      RAWINP
        DW      IN
        DW      PRTBUF
        DW      BUFIN           ;10
        DW      CONSTAT
        DW      FLUSHKB
        DW      DSKRESET
        DW      SELDSK
        DW      OPEN            ;15
        DW      CLOSE
        DW      SRCHFRST
        DW      SRCHNXT
        DW      DELETE
        DW      SEQRD           ;20
        DW      SEQWRT
        DW      CREATE
        DW      RENAME
        DW      INUSE
        DW      GETDRV          ;25
        DW      SETDMA
        DW      GETFATPT
        DW      GETFATPTDL
        DW      GETRDONLY
        DW      SETATTRIB       ;30
        DW      GETDSKPT
        DW      USERCODE
        DW      RNDRD
        DW      RNDWRT
        DW      FILESIZE        ;35
        DW      SETRNDREC
; Extended Functions
        DW      SETVECT
        DW      NEWBASE
        DW      BLKRD
        DW      BLKWRT          ;40
        DW      MAKEFCB
        DW      GETDATE
        DW      SETDATE
        DW      GETTIME
        DW      SETTIME         ;45
        DW      VERIFY
```

