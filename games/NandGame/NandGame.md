### Opcodes

一个有意思且比较重要的公式
$$
X - Y = \overline{\overline X + Y}
$$
证明：
$$
\begin{aligned}
\overline{\overline{X} + Y}
&= \overline{X_补 - 1 + Y}
\\
&= \overline{Y - X - 1}
\\
&= (Y-X-1)_补 - 1
\\
&= -1 - (Y-X-1)
\\
&= X - Y
\end{aligned}
$$

### Programming

#### Escape Labyrinth

```
# Assembler code 

# We cannot read PC from this goddamn machine
# Which means that we cannot use any Functional programming

# Variables: 
#   0x7000  store whether robot has turn left

# Init Variables
A = 0x7000
*A = 0

Detect_Front_1:
    A = 0x7fff
    D = *A

    # Mask For Obstacle Detect
    A = 0x0100
    D = D & A

    A = Move_Forward_1
    D = D; JEQ

    # if robot has turned left
    A = 0x7000
    D = *A

    # if equal 1, turn right
    A = Turn_Right_1
    D - 1; JEQ

    A = Turn_Left_1
    JMP



# Case 1: Move Forward
Move_Forward_1:
    # Mask of Move Forward
    A = 0x0004
    D = A
    A = 0x7fff
    D = D | *A
    *A = D

Wait_Move_1:
    A = 0x7fff
    D = *A
    # Mask of Moving
    A = 0x0400
    D = D & A
    A = Detect_Front_1
    D = D; JEQ
    A = Wait_Move_1
    JMP

# Case 2: Turn Left
Turn_Left_1:
    # Mask of Turn Left
    A = 0x0008
    D = A
    A = 0x7fff
    D = D | *A
    *A = D

Wait_Left_Turn_1:
    A = 0x7fff
    D = *A
    # Mask of Turning
    A = 0x0200
    D = D & A
    A = Leave_Left_Turn_1
    D = D; JEQ
    A = Wait_Left_Turn_1
    JMP

Leave_Left_Turn_1:
    A = 0x7000
    *A = 1
    A = Detect_Front_1
    JMP

# Case 3: Right Turn X 2
Turn_Right_1:
    # Mask of Turn Right
    A = 0x0010
    D = A
    A = 0x7fff
    D = D | *A
    *A = D

Wait_Right_Turn_1:
    A = 0x7fff
    D = *A
    # Mask of Turning
    A = 0x0200
    D = D & A
    A = Turn_Right_2
    D = D; JEQ
    A = Wait_Right_Turn_1
    JMP

Turn_Right_2:
    # Mask of Turn Right
    A = 0x0010
    D = A
    A = 0x7fff
    D = D | *A
    *A = D

Wait_Right_Turn_2:
    A = 0x7fff
    D = *A
    # Mask of Turning
    A = 0x0200
    D = D & A
    A = Leave_Right_Turn_1
    D = D; JEQ
    A = Wait_Right_Turn_2
    JMP

Leave_Right_Turn_1:
    A = 0x7000
    *A = 0
    A = Detect_Front_1
    JMP
```

