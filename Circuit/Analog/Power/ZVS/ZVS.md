#电流型逆变电路

#单相电流型逆变电路
![ni_1](picture\ni_1.jpg)
反馈：
* VT1,VT2,VT3,VT4同时开通
* 电流LT1,LT4降,LT2,LT3升
* LT1与LT4电流与LT2,LT3相等时，因为电容，继续下降
* 当LT1与LT4小于0时，VT1与VT4关断，因此UVt出现跳变
![ni_2](picture\ni_2.jpg)
![ni_3](picture\ni_3.jpg)

#ZVS
##原理图
![zvs_0](picture\ZVS_0.jpg)


###上电时
上电时L1通入的电流为零，电源通过R1、R2使Q1、Q2导通，L1电流逐渐增加，由于两个开关管特性差异，将导致流入两个开关管的电流不同，假设Q1电流大于Q2电流，T1将产生b为正，a为负的感应电压，于是通过T1形成正反馈，使Q1导通，Q2截止。完成启动过程。

![zvs_1](picture\ZVS_1.jpg)


2.      （t0~t1时间）稳态Q1导通时，由于上个周期T1电流为a到c，并且C1两端电压为零。由于电流不能突变，T1电流将对C1充电，C1逐渐为a负c正的电压，并且正弦变大，T1电流正弦变小。此时a电压被Q1下拉到0V，所以C点电压正弦变大，Q1栅极电压被D3稳压管钳位，Q1时钟保持导通。
3.      （t1时间）当T1中电流下降为零，其能量全部释放到C1，此时C1电压达到最大值。
4.      （t1~t2时间）C1开始通过T1由c到a放电，C1电压即c点电压正弦变小，T1电流由c到a正弦变大。
5.      （t2时间）当C1能力基本放完时，c点电压下降到MOS管阀值电压左右，将通过D2使Q1进入放大区。此时C1对T1绕组由c到a放电电流达到最大值。同时由于Q1进入放大区，a点电压逐渐上升，同时通过D1使Q2也进入放大区。
6.      （t2时间）C1放电完毕，T1绕组由c到a电流达到最大值，将像C 1充电，使C1充电为a正c负的电压，同时C1两端电压正弦变大。此时两个MOS管同时进入放大区。
7.      （图3）由于T1对C1的持续充电，C1上电压为a正c负，通过两个二极管使Q2栅极电压升高，Q1栅极逐渐下降，同时正反馈形成，Q2导通，Q1截止。
8.      Q2导通与Q1导通过程类似。

9.      L1电感值比T1大，整个震荡周期中L1电流基本不变。震荡过程中L1持续为LC振荡器补充电能。
