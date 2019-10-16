import copy

class no_terminal:
    def __init__(self,expr):
        self.tree = []
        self.add(expr)

    def add(self,expr):
        expr_tree = []
        



class syntax_tree:
    def __init__(self,syntax,translate):
        self.build_syntax(syntax)

    def build_syntax(self,syntax):
        self.no_term = {}
        symbols = []
        a = self.getlines(syntax)
        for i in a:
            symbols.append( self.get_symbol(i) )
        for i in symbols:
            if not self.no_term.has_key(i[0]):
                self.no_term[i[0]] = no_terminal(i)
            else:
                self.no_term[i[0]].add(i)


    def get_symbol(self,text):
        name_symbol = [chr(i) for i in ( range(ord('A'),ord('Z')+1) + range(ord('a'),ord('z')+1) + range(ord('0'),ord('9')+1) )] + ['_']
        op_list = [ '+','-','*','/','(',')','^','&','|','%','<','>' ]
        op2_list = ["->"]

        symbol_list = []
        tmp = text.strip()
        mylen = len(tmp)
        i = 0
        begin = 0
        flag = True
        while i<mylen and flag:
            begin = i
            if tmp[i] in name_symbol:
                j = begin
                while tmp[i] in name_symbol:
                    if i < mylen-1:
                        i += 1
                        j += 1
                    else:
                        j = i+1
                        flag = False
                        break
                tmp_str = copy.deepcopy(tmp[begin:j])
                symbol_list.append(tmp_str)
            elif tmp[i] in op_list:
                if i < mylen-1 and tmp[i:i+2] in op2_list:
                    symbol_list.append(tmp[i:i+2])
                    i += 2
                else:
                    symbol_list.append(tmp[i])
                    i += 1
            else:
                    i += 1

        return symbol_list



    def getlines(self,text):
        ret = []
        mylen = len(text)
        i = 0
        begin = 0
        flag = True
        while flag:
            begin = i
            j = begin
            while True:
                if text[i] == '\n' or text[i] == '\r':
                    j = i
                    while text[i] == '\n' or text[i] == '\r':       # skip \n and \r
                        if i < mylen-1:
                            i += 1
                        else:
                            flag = False
                            break
                    break
                elif i < mylen-1:
                    i += 1
                else:
                    j = i+1
                    flag = False
                    break
            tmp_str = copy.deepcopy(text[begin:j])
            ret.append(tmp_str)

        return ret
 


if __name__ == "__main__":
    f1 = open("1.txt")
    tmp1 = f1.read()
    f1.close()

    f2 = open("2.txt")
    tmp2 = f2.read()
    f2.close()

    my_tree = syntax_tree(tmp1,tmp2)