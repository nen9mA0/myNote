## MSVC

### RTTICompleteObjectLocator

```
signature        DWORD
offset           DWORD
cdOffset         DWORD
pTypeDescriptor  DWORD
pClassDescriptor DWORD
pSelf            DWORD        若为64位
```

### RTTITypeDescriptor

```
pVFTable         PTR_TYPE
spare            PTR_TYPE
name             STRLIT
```


