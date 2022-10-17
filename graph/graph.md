## 图形学

https://zhuanlan.zhihu.com/cgnotes

https://www.scratchapixel.com/

### 直线

#### DDA算法

直接计算直线方程 y = k*x+b，步进求解

#### Bresenham

##### 原理

核心思想：对于一条斜率k大于0且小于1的直线，(x,y)的下一个点可能是(x+1,y)或(x+1,y+1)

```
令
△x = | x2-x1 |
△y = | y2-y1 |

求y1的下一个点y2的值，由方程的差分可得
y2 = y1 + △y/△x
其中y1和y2均为直线坐标的真实值

这时假设y1_d和y2_d为直线四舍五入到整数的值，y1_r和y2_r为真实值，定义在y1的误差值err，且以y_r > y_d时的差值为正
err = y1_r - y1_d

下面求err的迭代公式
如果下一个点是(x+1,y)，则
err2 = err1 + △y/△x
如果下一个点是(x+1,y+1)，则
err2 = err1 + △y/△x - 1

则对于点选取(x+1,y)还是(x+1,y+1)的判断可以化为
err+k < 0.5  =>  (x+1,y)
err+k > 0.5  =>  (x+1,y+1)
```

##### 朴素实现代码

```c
int draw_line(int x0, int x1, int y0, int y1)
{
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);

    double k = (double)dy/dx;
    int err = -0.5;            //将大于/小于0.5的判断转化为0的判断
    int x = x0;
    int y = y0;

    for(int i=0; i<=dx; i++)
    {
        setpixel(x,y);
        x = x+1;
        e = e+k;
        if(e>=0)
        {
            y += 1;
            e -= 1;
        }
    }
}
```

##### 优化

```
判断条件
if err+k < 0.5:
  err = err + k
else:
  err = err + k - 1

可以转化为
2*(err+k) < 1
进一步，两边同乘dx
2*(err*dx + dy) < dx
```

由于(x0,y0)为整数，因此err初值为0
且err只作为y值是否加一的判断条件，实际值并无参与运算，因此可以改进err的迭代函数，转化为整数运算

```
令 e = dx * err
判断条件：2*(e+dy) < dx，即 e+dy < dx/2

e的迭代
若为(x+1,y)
因为 err2 = err1 + △y/△x
所以 e2 = e1 + dy
若为(x+1,y+1)
因为 err2 = err1 + △y/△x - 1
所以 e2 = e1 + dy - dx
```

##### 推广

进一步考虑k>1的情况

```
令err为x和y实际值与整数值的误差之和
此时以直线方程来考虑，则有
y = △y/△x x + b  =>  y△x - x△y + b△x = 0
即
y△x - x△y + C = 0
考虑err，有
y△x - x△y + C + err = 0
即
err = x△y - y△x - C
```

若只考虑第一象限（其他象限可以简单地通过修改步进值为1或-1实现，参考下面代码）

```
下一个点的坐标可能为(x,y+1) (x+1,y) (x+1,y+1)
(x,y+1): err2 = err1 + △x
(x+1,y): err2 = err1 - △y
(x+1,y+1): err2 = err1 + △x - △y
```

**判别条件：**

```
若err < -dx
ydx - xdy + C + err = 0
为了使abs(err)尽量小，应令y2取y1+1

同理，若err > dy，应令x2取x1+1
```

所以有下面一段经典代码

```c
void bresenham(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;        //设置步进方向
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;        //若dx>dy对应k<1的情况，否则反之

    while (setpixel(x0, y0), x0 != x1 || y0 != y1) {
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 <  dy) { err += dx; y0 += sy; }
    }
}
```

##### ref

[入门](https://www.cnblogs.com/feiquan/archive/2018/03/04/8506283.html)

[最后再看，对优化部分解释的很清楚](https://blog.csdn.net/cjw_soledad/article/details/78886117)

#### 胶囊体采样

### 旋转

#### 2D

##### 复数

$$
\begin{aligned}
z_1 &= a + b i
\\
z_2 &= c + d i
\\
z_1 z_2 &= (a + b i) (c + d i)
\\
 &= (ac-bd) + (bc + ad)i
\end{aligned}
$$

向量形式

$$
\begin{aligned}
z_1 &=
\begin{bmatrix}
a \\
b
\end{bmatrix}

\\
~
\\

z_2 &=
\begin{bmatrix}
c \\
d
\end{bmatrix}

\\
~
\\

z_1 z_2 &= 
\begin{bmatrix}
a & -b \\
b & a
\end{bmatrix}
\begin{bmatrix}
c \\
d
\end{bmatrix}
\\
&=
\begin{bmatrix}
ac - bd \\
ad + bc
\end{bmatrix}

\end{aligned}
$$

拓展：如果根据上式把z看做矩阵

矩阵形式：

$$
\begin{aligned}

z_1 &=
\begin{bmatrix}
a & -b \\
b & a
\end{bmatrix}

\\
~
\\

z_2 &=
\begin{bmatrix}
c & -d \\
d & c
\end{bmatrix}

\\
~
\\

z_1 z_2 &=
\begin{bmatrix}
a & -b \\
b & a
\end{bmatrix}
\begin{bmatrix}
c & -d \\
d & c
\end{bmatrix}

\\
&=
\begin{bmatrix}
ac-bd & -(ad+bc) \\
ad+bc & ac-bd
\end{bmatrix}

\end{aligned}
$$

因此乘法运算结果符合我们对向量矩阵形式的规定，因此可以写成矩阵形式
