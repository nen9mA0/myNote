import re
import time

tape_len = 100

class TuringEqu(object):
    def __init__(self, curr, myinput, mynext, write, move):
        self.curr = curr
        self.input = myinput
        if not len(myinput) == 1:
            raise ValueError("Input should be one byte")

        self.next = mynext
        self.write = write
        if not len(write) == 1:
            raise ValueError("write should be one byte")

        self.move = move
        if not self.move in ("L", "R", "0"):
            raise ValueError("Move should be L, R or 0")

    def GetNext(self, p):
        if self.move == "L":
            new_p = p - 1
        elif self.move == "R":
            new_p = p + 1
        else:
            new_p = p

        global tape_len
        if new_p < 0 or new_p > tape_len*2:
            raise ValueError("tape out of range")

        return (self.next, self.write, new_p)

    def __str__(self):
        return ("(%s, %s) = (%s, %s, %s)" %(self.curr, self.input, self.next, self.write, self.move))

    def __repr__(self):
        return str(self)


class Turing(object):
    q_pattern = re.compile(r"Q:(.*)")
    input_sym_pattern = re.compile(r"input_sym:(.*)")
    stack_sym_pattern = re.compile(r"stack_sym:(.*)")
    empty_pattern = re.compile(r"empty:(.*)")
    start_pattern = re.compile(r"start:(.*)")
    recv_pattern = re.compile(r"recv:(.*)")
    reject_pattern = re.compile(r"reject:(.*)")

    equ_pattern = re.compile(r"\((?P<curr>(.*)), (?P<input>(.*))\)\s*=\s*\((?P<next>(.*)), (?P<write>(.*)), (?P<move>(.*))\)")

    timeout = 1.0

    def __init__(self, cfg_lines, func_lines):
        self.DefaultInit()
        for line in cfg_lines:
            p = self.q_pattern.match(line)
            if p:
                self.Q = [i.strip() for i in p.group(1).split(",")]
                continue

            p = self.input_sym_pattern.match(line)
            if p:
                self.input_sym = [i.strip() for i in p.group(1).split(",")]
                continue

            p = self.stack_sym_pattern.match(line)
            if p:
                self.stack_sym = [i.strip() for i in p.group(1).split(",")]
                continue

            p = self.empty_pattern.match(line)
            if p:
                self.empty = p.group(1).strip()
                continue

            p = self.start_pattern.match(line)
            if p:
                self.start = [i.strip() for i in p.group(1).split(",")]
                continue

            p = self.recv_pattern.match(line)
            if p:
                self.recv = [i.strip() for i in p.group(1).split(",")]
                continue

            p = self.reject_pattern.match(line)
            if p:
                self.reject = [i.strip() for i in p.group(1).split(",")]
                continue

        self.CheckCfg()

        for line in func_lines:
            curr, input_sym, equ_tmp = self.CreateOneEqu(line)
            if not curr in self.equ_dict:
                self.equ_dict[curr] = {}
            if not input_sym in self.equ_dict[curr]:
                self.equ_dict[curr][input_sym] = equ_tmp
            else:
                raise ValueError("Cannot support multi equation for the same state and input now")

    def CreateOneEqu(self, line):
        p = self.equ_pattern.match(line)
        if p:
            curr = p.group("curr")
            myinput = p.group("input")
            if myinput == "emp":
                myinput = self.empty
            mynext = p.group("next")
            mywrite = p.group("write")
            if mywrite == "emp":
                mywrite = self.empty
            move = p.group("move")
            equ = TuringEqu(curr, myinput, mynext, mywrite, move)
            return (curr, myinput, equ)
        else:
            raise ValueError("Format Error")


    def DefaultInit(self):
        self.empty = " "
        self.recv = []
        self.reject = []
        self.start = []
        self.equ_dict = {}

    def CheckCfg(self):
        if len(self.recv) == 0 or len(self.start) == 0:
            raise ValueError("Empty recv or start symbol")
        for sym in self.recv:
            if not sym in self.Q:
                raise ValueError("Recv sym %s not in input sym" %sym)
        for sym in self.start:
            if not sym in self.Q:
                raise ValueError("Start sym %s not in input sym" %sym)

        if len(self.reject) != 0:
            for sym in self.reject:
                if not sym in self.Q:
                    raise ValueError("Reject sym %s not in input sym" %sym)

        if not len(self.empty) == 1:
            raise ValueError("Empty should be one byte")

    def Emu(self, x, iscalc=False, debug=True):
        global tape_len
        self.tape = [self.empty for i in range(tape_len)]
        self.tape.extend([i for i in x])
        self.tape.extend([self.empty for i in range(len(x), tape_len)])
        curr_state = self.start[0]  # TODO: Support for multi start symbol
        p = tape_len

        top_p = tape_len + len(x)

        start_time = time.time()
        while (curr_state not in self.reject) and (curr_state not in self.recv):
            # print state string
            state_str = ""
            for i in range(tape_len, top_p):
                if i != p:
                    state_str += "%s " %(self.tape[i])
                else:
                    state_str += "%s %s " %(curr_state, self.tape[i])
            if p == top_p:
                state_str += "%s" %(curr_state)


            new_input = self.tape[p]
            try:
                equ = self.equ_dict[curr_state][new_input]
            except:
                break
            new_state, write, new_p = equ.GetNext(p)
            curr_state = new_state
            self.tape[p] = write
            p = new_p
            if p > top_p:
                top_p = p

            if not debug and (time.time() - start_time > self.timeout):     # timeout
                break

        if curr_state in self.recv:
            if iscalc:
                output_str = ""
                num = 0
                for i in range(tape_len, top_p):
                    if not self.tape[i] == self.empty:
                        num += 1
                        output_str += "%s " %(self.tape[i])
                return output_str + "\t\t" + "Calc Result: %d" %num
            return "accept"
        else:
            return "reject"


with open("exam.txt") as f:
    lines = f.readlines()

cfg_linenum = int(lines[0])
turing = Turing(lines[1:1+cfg_linenum], lines[1+cfg_linenum:])

test_str = input()
# res = turing.Emu(test_str)
res = turing.Emu(test_str, True)
print(res)