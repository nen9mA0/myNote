## PinTool基本原理



## 实例

```c
FILE *trace;

void RecordMemWrite(void *ip, void *addr, uint32_t size)
{
    fprintf(trace, "%p: W %p %d\n", ip, addr, size);
}

void Instruction(INS ins, void *v)
{
    if(INS_IsMemoryWrite(ins))
        	INS_InsertPredicatedCall(
        		ins, IPOINT_BEFORE, AFUNPTR(RecordMemWrite),
        		IARG_INST_PTR, IARG_MEMORYWRITE_EA,
        		IARG_MEMORYWRITE_SIZE, IARG_END);
}

int main(int argc, char *argv[])
{
    PIN_Init(argc, argv);
   	trace = fopen("atrace.out", "w");
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_StartProgram();
    return 0;
}
```

