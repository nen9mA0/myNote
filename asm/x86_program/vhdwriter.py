from cgitb import reset
import sys
import getopt
import os
import struct
import time,datetime

feature_type = ["None", "Temporary", "Reserved"]
disk_type = ["None","Reserved","Fixed Hard Disk","Dynamic Hard Disk","Differencing Hard Disk"]


class vhd:
    def __init__(self, footer):
        tss1 = '2000-1-1 12:00:00'
        timeArray = time.strptime(tss1, "%Y-%m-%d %H:%M:%S")
        timeStart = int(time.mktime(timeArray))
        try:
            s = struct.Struct(">8sIIQI4sIIQQHBBII4IB")
            unpacked = s.unpack(footer[0:85])
            self.cookie = unpacked[0]
            self.feature = feature_type[unpacked[1]]
            self.file_format = unpacked[2]
            self.offset = unpacked[3]
            timeArray = time.localtime(unpacked[4] + timeStart - 14400)    #magic number,because the time is always 4h in advance
            self.time_stamp = time.strftime("%Y--%m--%d %H:%M:%S", timeArray)
            self.creator_app = unpacked[5]
            if unpacked[6] == 0x10000:
                self.creator_ver = "Virtual Server 2004"
            elif unpacked[6] == 0x50000:
                self.creator_ver = "Virtual PC 2004"
            else:
                self.creator_ver = ""
            if unpacked[7] == 0x5769326B:
                self.creator_host = "Wi2k"
            elif unpacked[7] == 0x4D616320:
                self.creator_host = "Mac"
            else:
                self.creator_host = ""
            self.origin_size = unpacked[8]
            self.current_size = unpacked[9]
            self.disk_geometry = [unpacked[10],unpacked[11],unpacked[12]]
            self.disk_type = disk_type[unpacked[13]]
            self.check_sum = unpacked[14]
            self.unique_id = "%X%X%X%X" %(unpacked[15],unpacked[16],unpacked[17],unpacked[18])
            self.saved_state = unpacked[19]
        except:
            print("Cannot Identify Features")
            pass
        my_checksum = self.checksum(footer,len(footer))
        if my_checksum + self.check_sum != 0xffffffff:
            print("Checksum Failed")

    def checksum(self,str,len):
        check_sum = 0
        for i in range(len):
            if i<64 or i>67:
                # check_sum += ord(str[i])
                check_sum += str[i]
        return check_sum


    def vhd_print(self):
        print("cookie:\t %s" % self.cookie)
        print("Create Time:\t %s" % self.time_stamp)
        print("Creator App:\t %s" % self.creator_app)
        print("Creator Host OS: %s" % self.creator_host)
        print("Disk_ID: %s" %self.unique_id)
        print("Disk Size: %d KB" %(self.current_size / 1024 ))
        print("Cylinder: %d \tHeads: %d \tSectors: %d " %(self.disk_geometry[0],self.disk_geometry[1],self.disk_geometry[2]))
    

def usage():
    print("Usage")
    print("python vhdwriter.py |opt| vhd_file_path [bin_file_path]")
    print("-n number  LBA block number")
    print("-c x       fill x blocks with 00")
    print("WARNING: ONLY SUPPORT FIXED VHD IMAGE NOW")
    exit()


def read_vhd_file(file_path):
    if os.path.exists(file_path):
        footer = ""
        try:
            f = open(file_path, "rb")
            f.seek(-512, 2)
            footer = f.read(512)
            f.close()
        except IOError:
            print("Read vhd file fail")
            f.close()
            exit(-1)

        vhd_footer = vhd(footer)
        vhd_footer.vhd_print()
    else:
        print("vhd file no exist")
        exit()
    return vhd_footer

def get_bin_size(file_path):        #Assime that size of binary file is not too large
    if os.path.exists(file_path):
        end = 0
        try:
            f = open(file_path, "r")
            f.seek(0, 2)
            end = f.tell()
            f.close()
        except IOError:
            print("Read bin file fail")
            f.close()
            exit(-1)
    else:
        print("bin file no exist")
        exit()
    return end

def read_bin_file(file_path):
    if os.path.exists(file_path):
        try:
            f = open(file_path, "rb")
            filetmp = f.read()
            f.close()
        except IOError:
            print("Read bin file fail")
            f.close()
            exit(-1)
    else:
        print("bin file no exist")
        exit()
    
    if len(filetmp) == size:
        return filetmp
    else:
        return ""

def write_vhd_file(file_path,offset,bin_tmp,size):
    if os.path.exists(file_path):
        end = 0
        try:
            f = open(file_path, "rb+")
            f.seek(offset, 0)
            tmp = bytes(bin_tmp)
            f.write(tmp)
            end = f.tell()
            f.close()
        except IOError:
            print("Write vhd file fail")
            f.close()
            exit(-1)
    else:
        print("vhd file no exist")
        exit()
    write_size = end - offset
    if write_size == size:
        return True
    else:
        return False

if __name__ == '__main__':
    argv = sys.argv[1:]
    vhd_file_path = ""
    bin_file_path = ""
    block = 0
    if len(argv) > 0:
        try:
            opts, args = getopt.getopt(argv, "n:i:o:c:")
        except getopt.GetoptError:
            usage()
        if len(args) >= 2:
            vhd_file_path = args[0]
            bin_file_path = args[1]
        elif len(args) == 1:        # only have vhd_file_path arg
            vhd_file_path = args[0]
        else:
            usage()

        nflag = False
        block = 0
        clr_block = False
        clr_block_num = 0
        for opt, arg in opts:
            if "-n" in opt:
                block = int(arg)
                nflag = True
            if "-c" in opt:
                clr_block_num = int(arg)
                clr_block = True
        if nflag == False:
            print("No block number specify,0 will be used")

        vhd_footer = read_vhd_file(vhd_file_path)
        # get binary size
        if clr_block:
            size = 512 * clr_block_num
        else:
            size = get_bin_size(bin_file_path)
        sector = size / 512 + (1 if (size % 512) else 0)
        if size >= vhd_footer.current_size:
            print("Binary File Too big")
            exit()
        else:
            print("Write Will Begin At %d Sector(LBA), Binary File Size is %d, %d Sector(s) Will be Changed" %(block,size,sector))
            print("Press Any Key To Continue")
        input()

        if clr_block:
            binfile = b"\x00" * size
        else:
            binfile = read_bin_file(bin_file_path)
        if len(binfile) != size:
            print("Read Binary File Error")
            exit()
        offset = block * 512
        ret = write_vhd_file(vhd_file_path,offset,binfile,size)
        if ret == True:
            print("Write Success")
        else:
            print("Write Failed")
    else:
        usage()
