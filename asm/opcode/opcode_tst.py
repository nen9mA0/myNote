from keystone import *

ks = Ks(KS_ARCH_X86, KS_MODE_64)

while True:
    ins = input()
    if ins != "exit":
        try:
            encode = ks.asm(ins)
        except Exception as e:
            print(e)
            exit()
        ins_hex = encode[0]
        print("".join(["%02x "%i for i in ins_hex]))