import numpy as np

def RowLadderSimplify(A):       # 行阶梯型
    m = A.shape[0]
    n = A.shape[1]
    for i in range(n-1):
        for j in range(i+1, m):
            if abs(A[i][i]) < 1e-6:
                pass
            else:
                tmp = A[j][i] / A[i][i]
                A[j] -= A[j][i] / A[i][i] * A[i]
                print("r%d = r%d - %f * r%d" %(j+1, j+1, tmp, i+1))
    return A

def LineSimplestSimplify(A):    # 行最简型
    for i in range(min(A.shape[0], A.shape[1])):
        if abs(A[i][i]) > 1e-6:
            tmp = A[i][i]
            A[i] /= A[i][i]
            print("r%d = r%d / %f" %(i+1, i+1, tmp))
    return A

def Normalizing(A):             # 等价标准型
    minshape = min(A.shape[0], A.shape[1])
    for i in range(minshape):
        for j in range(i+1, minshape):
            tmp = A[i][j]
            A[i] -= A[i][j] * A[j]
            print("r%d = r%d - %f * r%d" %(i+1, i+1, tmp, j+1))
    return A

def PrintMatrix(A):
    for i in range(A.shape[0]):
        for j in range(A.shape[1]):
            print("%lf\t" %A[i][j], end='')
        print()

a = np.random.randint(1, 255, size=(4,4))
a = a.astype('float64')
PrintMatrix(a)
print()

a = RowLadderSimplify(a)
PrintMatrix(a)
print()

a = LineSimplestSimplify(a)
PrintMatrix(a)
print()

a = Normalizing(a)
PrintMatrix(a)
print()