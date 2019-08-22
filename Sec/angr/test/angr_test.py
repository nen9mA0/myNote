import angr

'''
proj = angr.Project("0pack.elf")
state = proj.factory.entry_state()
print state
print state.solver
one = state.solver.BVV(1,64)
print one
print one.op
print one.args
x = state.solver.BVS("x",64)
print x
print x.op
print x.args
y = x+one
print y
print y.op
print y.args

yes = one == 1
print yes
print state.solver.is_true(yes)

x = state.solver.BVS("x",64)
y = state.solver.BVS("y",64)
state.solver.add(x>y)
state.solver.add(y>2)
state.solver.add(10>x)
print state.solver.eval(x)

a = state.solver.FPV(3.2,state.solver.fp.FSORT_DOUBLE)
print a
b = state.solver.FPS('b',state.solver.fp.FSORT_DOUBLE)
print b
print a+b
print b+2<0
one = state.solver.FPV(1.1,state.solver.fp.FSORT_DOUBLE)
print state.solver.fpAdd(state.solver.fp.RM_RNE,one,a)
'''

#============fake firmware test==============
proj = angr.Project('fake_firmware')
state = proj.factory.entry_state(stdin=angr.SimFile)  # ignore that argument for now - we're disabling a more complicated default setup for the sake of education
while True:
    succ = state.step()
    if len(succ.successors) == 2:
        break
    state = succ.successors[0]

state1,state2 =  succ.successors
print state1,state2

input_data = state1.posix.stdin.load(0,state.posix.stdin.size)
print state1.solver.eval(input_data,cast_to=bytes)
print state2.solver.eval(input_data,cast_to=bytes)

print state1.history.bbl_addrs
succ = state2.step()
if len(succ.successors) == 1:
    state = succ.successors[0]
print state.history.descriptions