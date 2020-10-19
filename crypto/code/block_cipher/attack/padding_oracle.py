# # | iv[-1]  |  c[-1]   |
#       \        |
#        \      M[-1]
#         \_____ ^
#                01     <== Padding

# # | iv[-2:]  |  c[-2:]   |
#       \        |
#        \       M[-2:]
#         \_____ ^
#                02 02   <== Padding

# # | iv[-3:]  |  c[-3:]   |
#       \        |
#        \       M[-3:]
#         \_____ ^
#             03 03 03   <== Padding

# 假设从最后一个padding开始，即追求构造一个解密后最后一位为01的密文块
# 方法是爆破IV的最后一位，一定有一个数iv[-1]=m满足 iv[-1] ^ M[-1] = 01
# 所以可以反推 M[-1] = m ^ 01，M为中间值
# 
# 此后爆破倒数第二位，由于只有当后两位为02 02时padding才正确，首先先设置好最后一位
# 因为 M[-1] ^ x = 02，因此设置IV[-1]为 M[-1] ^ 02
# 可以爆破第二位
# 
# 以此类推，可以爆破全部block的内容

# 要求：可以控制IV，服务端没有指定长度，且会返回解密是否正确（若Padding不正确则返回解密错误）

from Crypto.Cipher import AES
import random

class Server:
    block_size = 16
    def __init__(self):
        pass

    def RecvText(self, text):
        try:
            text_tmp = self.Decrypt(text[:self.block_size], text[self.block_size:])
            #print((int.from_bytes(text[:self.block_size],"little") ^ int.from_bytes(text_tmp,"little")).to_bytes(self.block_size, "little").hex())
            plaintext = self.DelPadding(text_tmp)
        except Exception as e:
            #print(e)
            plaintext = "Error"
        return plaintext

    def DHExchangeSend(self):
        self.p = 0x10000000000000000000000000000001
        g = 997
        #self.a = random.randint(256, 511)
        self.a = 256
        return (g**self.a) % self.p

    def DHExchangeRecv(self, gmodb):
        key = (gmodb ** self.a) % self.p
        self.key = key.to_bytes(self.block_size, "little")

    def DelPadding(self, text):
        length = text[-1]
        if text[-length:] != bytes(chr(length), "latin1") * length:
            raise ValueError("Decrypt Error")
        return text[:-length]

    def Decrypt(self, iv, ciphertext):
        mode = AES.MODE_CBC
        crypto = AES.new(self.key, mode, iv)
        plain_text = crypto.decrypt(ciphertext)
        return plain_text

class Client:
    block_size = 16
    def __init__(self):
        self.GenIV()
        pass

    def GenIV(self):
        tmp = []
        for i in range(16):
            #tmp.append(random.randint(0, 255))
            tmp.append(10)
        self.iv = bytes(tmp)

    def Send(self, str1):
        text = self.Padding(str1)
        ciphertext = self.Encrypt(self.iv, text)
        return self.iv + ciphertext

    def DHExchangeSend(self):
        self.p = 0x10000000000000000000000000000001
        g = 997
        #self.a = random.randint(256, 511)
        self.a = 256
        return (g**self.a) % self.p

    def DHExchangeRecv(self, gmodb):
        key = (gmodb ** self.a) % self.p
        self.key = key.to_bytes(self.block_size, "little")

    def Padding(self, text):
        length = self.block_size - len(text) % self.block_size
        text += chr(length) * length
        return text

    def Encrypt(self, iv, plaintext):
        mode = AES.MODE_CBC
        crypto = AES.new(self.key, mode, iv)
        cipher_text = crypto.encrypt(plaintext)
        return cipher_text

class MidAttacker:
    block_size = 16
    def __init__(self, server, cipher_text):
        self.iv = cipher_text[:self.block_size]
        self.cipher_text = cipher_text[self.block_size:]
        self.server = server
        self.inter = b""

    def SetIV(self, iv):
        self.iv = iv

    def OraclePaddingOneBlock(self, i):
        ori_iv = b""
        if i == 0:
            ori_iv = self.iv
        else:
            ori_iv = self.cipher_text[(i-1)*self.block_size:i*self.block_size]
        block_ciphertxt = self.cipher_text[i*self.block_size:(i+1)*self.block_size]
        self.inter = b""
        for n in range(self.block_size):
            iv_prev = b"\x00" * (self.block_size-n-1)
            iv_tmp = b""
            for s in self.inter:
                iv_tmp += (s ^ (n+1)).to_bytes(1, "little")
            for m in range(256):
                iv = iv_prev + m.to_bytes(1, "little") + iv_tmp
                ret = self.server.RecvText(iv+block_ciphertxt)
                if ret != "Error":
                    #print(n)
                    #print(iv.hex())
                    tmp = m ^ (n+1)
                    self.inter = tmp.to_bytes(1, "little") + self.inter
                    break
        return (int.from_bytes(self.inter, "little") ^ int.from_bytes(ori_iv, "little")).to_bytes(self.block_size, "little")

server = Server()
client = Client()

tmp1 = server.DHExchangeSend()
tmp2 = client.DHExchangeSend()

client.DHExchangeRecv(tmp1)
server.DHExchangeRecv(tmp2)

send = "abcdef1234567890"*3 + "abcd"
#send = "abcdef1234567890"
cipher_text = client.Send(send)
#plain_text = server.RecvText(cipher_text)

block_size = 16
midattacker = MidAttacker(server, cipher_text)

size = (len(send)+block_size-1) // block_size
for i in range(size):
    ret = midattacker.OraclePaddingOneBlock(i)
    print(ret)
