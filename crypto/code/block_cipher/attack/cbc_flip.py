# | ci-1  |  ci   |
#       \     |
#        \  Mi-1 (= pi ^ ci-1)
#         \__ ^
#            pi

# | ci-1 ^ pi ^ new_pi  |  ci   |
#             \             |
#              \           Mi-1
#               \________ ^
#                          new_pi

# 通过修改第i-1块密文C[i-1]，控制第i块内容P[i]
# 注意：修改C[i-1]势必导致该块密文解密结果出错，导致P[i-1]出错
# 因此需要获取C[i-1]的解密结果M[i-1]，此后通过修改C[i-2]为M[i-1]^P[i-1]使其解密正确
# 此后循环此过程，直到最后改变iv的值

from Crypto.Cipher import AES
from binascii import b2a_hex, a2b_hex

class Server:
    block_size = 16
    key = b'dafkjdahdjalkdja'
    user = {"root":"abcdef1234567890"+"1234567890abcdef"*9, "user":"1234567890abcdef"*10}
    trash = "abcdef1234567890" * 10
    def __init__(self):
        self.trash_len = len(self.trash)
        pass

    def Check_Login(self, req):
        if len(req) > 0:
            plain_text = self.Decrypt(req[:self.block_size], req[self.block_size:])
            try:
                if plain_text[:self.trash_len].decode() != self.trash:
                    raise ValueError()
                plain_str = self.DelPadding(plain_text).decode()
                lst = plain_str.split(':')
                username = lst[0]
                auth = lst[1]
                if self.user[username] == auth:
                    if username == "root":
                        print("root login")
                    return "Login Success"
                else:
                    return "Login Failed"
            except:
                return "Cannot Decode %s" %(b2a_hex(plain_text))
        return "Login Failed"

    def DelPadding(self, text):
        length = text[-1]
        return text[self.trash_len: len(text) - length]

    def Decrypt(self, iv, ciphertext):
        mode = AES.MODE_CBC
        crypto = AES.new(self.key, mode, iv)
        plain_text = crypto.decrypt(ciphertext)
        return plain_text

class Client:
    key = b'dafkjdahdjalkdja'
    block_size = 16
    trash = "abcdef1234567890" * 10
    def __init__(self, iv):
        self.iv = iv
        self.auth = "1234567890abcdef" * 10

    def Padding(self, text):
        length = self.block_size - len(text) % self.block_size
        text += chr(length) * length
        return text

    def Login(self, username):
        if username == "root":
            print("cannot login as root")
            return ""
        self.plaintext = self.Padding(self.trash + username + ":" + self.auth)
        req_str = self.iv + self.Encrypt(self.iv, self.plaintext)
        return req_str

    def Encrypt(self, iv, plaintext):
        mode = AES.MODE_CBC
        crypto = AES.new(self.key, mode, iv)
        cipher_text = crypto.encrypt(plaintext)
        return cipher_text

class MidAttacker:
    block_size = 16
    def __init__(self, req, old_subplaintext, new_subplaintext):
        self.iv = req[:self.block_size]
        self.ciphertext = req[self.block_size:]
        if len(old_subplaintext) % self.block_size != 0:
            raise ValueError("req_subplaintext Must Be Multiple of block_size")
        
        self.old_subplaintext = old_subplaintext.encode()
        self.new_subplaintext = (new_subplaintext + old_subplaintext[len(new_subplaintext):]).encode()
        index = self.CmpIndex(old_subplaintext, new_subplaintext)
        if index != -1:
            self.reserved_index = (index+1) // self.block_size
        else:
            self.reserved_index = 0
        self.lastchangedblock = (len(new_subplaintext)-1) // self.block_size
        self.current_block = self.lastchangedblock

    def CmpIndex(self, str1, str2):
        length = min(len(str1), len(str2))
        index = -1
        for i in range(length):
            if str1[i] == str2[i]:
                index += 1
            else:
                break
        return index

    def FlipOneBlock(self, ret_plaintext):
        i = self.current_block
        if i > 0:
            old_pi = int.from_bytes(ret_plaintext[i*self.block_size:(i+1)*self.block_size], "little")
            new_pi = int.from_bytes(self.new_subplaintext[i*self.block_size:(i+1)*self.block_size], "little")
            ci_1 = int.from_bytes(self.ciphertext[(i-1)*self.block_size:i*self.block_size], "little")
            M_i = old_pi ^ ci_1
            new_ci_1 = M_i ^ new_pi
            tmp = new_ci_1.to_bytes(self.block_size, "little")
            self.ciphertext = self.ciphertext[:(i-1)*self.block_size] + tmp + self.ciphertext[i*self.block_size:]
        elif i == 0:
            old_pi = int.from_bytes(ret_plaintext[i*self.block_size:(i+1)*self.block_size], "little")
            new_pi = int.from_bytes(self.new_subplaintext[i*self.block_size:(i+1)*self.block_size], "little")
            iv = int.from_bytes(self.iv, "little")
            M_i = old_pi ^ iv
            new_iv = M_i ^ new_pi
            self.iv = new_iv.to_bytes(self.block_size, "little")
        self.current_block -= 1
        return self.iv + self.ciphertext


iv = b'aaaaaaaaaaaaaaaa'
trash = "abcdef1234567890" * 10

server = Server()
client = Client(iv)

tmp_cipher = client.Login("user")

midattacker = MidAttacker(tmp_cipher, client.plaintext, trash+"root"+":abcdef1234567890")

ret_plaintext = client.plaintext.encode()

for i in range(midattacker.lastchangedblock+1):
    tmp_cipher = midattacker.FlipOneBlock(ret_plaintext)
    tmp_plain = server.Check_Login(tmp_cipher)
    ret_plaintext = a2b_hex(tmp_plain[16:-1])
    pass
print(ret_plaintext)