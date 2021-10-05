import re

class Parser(object):
    def __init__(self):
        self.var = {}

    def Parse(self, exp):
        stack = []
        i = 0
        while True:
            if exp[i] == '(':
                stack.append(i)
            elif exp[i] == ')':
                stack.pop()

            i += 1


class lambda_exp(parser):
    lambda_pattern = re.compile(r"\(\s*lambda\s*\((.*?)\)(.*)\)")

    def __init__(self, exp):
        super.__init__()
        r = self.lambda_pattern.match(exp)
        self.father = None
        if r:
            self.param = r.group(1).strip().split()
            self.body = r.group(2).strip()
            tmp = lambda_exp(self.body)
            if tmp.body != None:
                self.child = tmp
                tmp.father = self
            else:
                self.child = None
        else:
            self.param = None
            self.body = None

    def alpha(self, change_dict):
        pass

    def Beta(self, func_param):
        a = 0


class Func(Parser):
    def __init__(self, exp):
        super.__init__()





add_1 = "( lambda (f) (lambda (x) (f ((n f) x))) )"
zero = "(lambda (f) (lambda (x) x))"

a = lambda_exp(add_1)
b = lambda_exp(zero)

a.exec(b)