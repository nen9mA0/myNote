# 代码结构

simplefoc的整体架构其实很简单，由下列几个组件组成：

* FOC控制算法

* 电流传感器

* 角度传感器

在common中定义了基础的算法，以及基类，其中对应三个组件提供了三个基类

* FOCMotor

* CurrentSense

* Sensor

# common

该文件夹主要存放一些基类定义和通用的算法实现

## foc_utils

cpp文件主要是定义了几种数学算法的底层实现

sin是打整数表实现的，sqrt是用快速反向平方根的算法实现的

头文件则定义了一些常数，如PI等。此外定义了几个FOC算法的结构体

```c
// dq current structure
struct DQCurrent_s
{
    float d;
    float q;
};
// phase current structure
struct PhaseCurrent_s
{
    float a;
    float b;
    float c;
};
// dq voltage structs
struct DQVoltage_s
{
    float d;
    float q;
};
```

## lowpass_filter

实现的是一个低通滤波器类

算法为一阶低通滤波，[一阶数字低通滤波器-C语言/matlab实现_你的青春我的梦的博客-CSDN博客](https://blog.csdn.net/yuyan7045/article/details/120696576)

公式为：

$$
Y' = aX + (1-a) Y
$$

其中X为新的输入量，Y为上一个时刻的值

这里的常数a选择的是 $\frac{\Delta t}{t+\Delta t}$

所以总的公式为

$$
Y' = \frac{\Delta t}{t+\Delta t} X + \frac{t}{t+\Delta t} Y
$$

其中每0.3秒以上才会刷新时间戳（即公式里的t）

## PID

比例微分积分

PID算法实现，公式如下

$$
\begin{aligned}
u(t) &= K_p [ e(t) + \frac{1}{T_i} \int_0^t e(t) dt + T_d \frac{d e(t)}{dt}]
\\
&= K_p e(t) + K_i \int_0^t e(t) dt + K_d \frac{d e(t)}{dt}
\end{aligned}
$$

传递函数如下 [比例积分微分控制_百度百科](https://baike.baidu.com/item/%E6%AF%94%E4%BE%8B%E7%A7%AF%E5%88%86%E5%BE%AE%E5%88%86%E6%8E%A7%E5%88%B6/22010564?fr=aladdin)

$$
G(s) = K_p + K_i \frac{1}{s} + K_d s
$$

其中u(t)为t时刻的调节量，e(t)为t时刻的error，即实测量与调节量的差值

#### P

比例项

$$
u_P(t) = P e(t)
$$

#### I

积分项

$$
u_I(t) = u_I(t-\Delta t) + \frac{1}{2} I \Delta t [e(t) + e(t-\Delta t)] 
$$

这里的积分项采取的近似方法是梯形法，见 https://zhuanlan.zhihu.com/p/518202321?utm_id=0

#### D

微分项

$$
u_D(t) = D \frac{e(t) - e(t-\Delta t)}{\Delta t}
$$

#### ramp

该项是为了防止调节量过高设置的

其会将调节率限制在小于ramp且大于-ramp

$$
-ramp \leq \frac{u(t) - u(t-\Delta t)}{\Delta t} \leq ramp
$$

因此调节量会被限制在

$$
u(t-\Delta t) - ramp\Delta t \leq u(t) \leq u(t-\Delta t) + ramp\Delta t 
$$

## base class

该文件夹下定义了一系列基类

### CurrentSense

#### CurrentSense类

电流传感器

##### 属性

```cpp
class CurrentSense{
    public:

    // variables
    bool skip_align = false; //!< variable signaling that the phase current direction should be verified during initFOC()
                    //
    BLDCDriver* driver = nullptr; //!< driver link
                    // 指向传感器对应的BLDCDriver类
    bool initialized = false; // true if current sense was successfully initialized
                    // 是否成功初始化 
    void* params = 0; //!< pointer to hardware specific parameters of current sensing
                    // 硬件相关参数
}
```

##### 虚函数

* init  初始化
  
  ```cpp
  /**
   *  Function intialising the CurrentSense class
   *   - All the necessary intialisations of adc and sync should be implemented here
   *   
   * @returns -  0 - for failure &  1 - for success 
  */
  virtual int init() = 0;
  ```

* linkDriver  将类与对应的BLDCDriver类相关联，这一步是当电机与电流采样操作有同步要求时才需要
  
  ```cpp
  /**
   * Linking the current sense with the motor driver
   * Only necessary if synchronisation in between the two is required
   */
  void linkDriver(BLDCDriver *driver);
  ```

* driverAlign
  
  ```cpp
  /**
   * Function intended to verify if:
   *   - phase current are oriented properly 
   *   - if their order is the same as driver phases
   * 
   * This function corrects the alignment errors if possible ans if no such thing is needed it can be left empty (return 1)
   * @returns -  0 - for failure &  positive number (with status) - for success 
   */
  virtual int driverAlign(float align_voltage) = 0;
  ```

* getPhaseCurrents  获取相电流
  
  ```cpp
  /**
   *  Function rading the phase currents a, b and c
   *   This function will be used with the foc control throught the function 
   *   CurrentSense::getFOCCurrents(electrical_angle)
   *   - it returns current c equal to 0 if only two phase measurements available
   * 
   *  @return PhaseCurrent_s current values
   */
  virtual PhaseCurrent_s getPhaseCurrents() = 0;
  ```

* getDCCurrent  获取控制器设置的电流值
  
  ```cpp
  /**
   * Function reading the magnitude of the current set to the motor
   *  It returns the abosolute or signed magnitude if possible
   *  It can receive the motor electrical angle to help with calculation
   *  This function is used with the current control  (not foc)
   *  
   * @param angle_el - electrical angle of the motor (optional) 
   */
  virtual float getDCCurrent(float angle_el = 0);
  ```

##### 方法

* getFOCCurrents
  
  ```cpp
  /**
   * Function used for FOC contorl, it reads the DQ currents of the motor 
   *   It uses the function getPhaseCurrents internally
   * 
   * @param angle_el - motor electrical angle
   */
  DQCurrent_s getFOCCurrents(float angle_el); 
  ```

#### Clark和Park变换算法

主要算法在CurrentSense的**getFOCCurrents**和**getDCCurrents**方法中，前者返回FOC算法中的Id和Iq值，后者返回电流的绝对值

##### Clark变换

因为对于clark变换，在很多驱动板上只测量两个相的相电流，因此这里计算分为两种

![](pic/1.png)

根据clark变换公式，有

$$
\begin{aligned}
I_{\alpha} &= I_a - cos(\frac{2 \pi}{3}) I_b - cos(\frac{2 \pi}{3}) I_c
\\
I_{\beta} &= sin(\frac{2 \pi}{3}) I_b - sin(\frac{2 \pi}{3}) I_c
\end{aligned}
$$

注意，这里为了计算方便，对上式取得的基底进行了缩放，缩放系数为2/3，所以有

$$
\begin{aligned}
I_{\alpha} &= \frac{2}{3} [I_a - \frac{1}{2} I_b - \frac{1}{2} I_c]
\\
&= \frac{2}{3} I_a - \frac{1}{3} I_b - \frac{1}{3} I_c
\\
I_{\beta} &= \frac{2}{3} [\frac{\sqrt{3}}{2} I_b - \frac{\sqrt{3}}{2} I_c]
\\
&= \frac{1}{\sqrt 3} I_b - \frac{1}{\sqrt 3} I_c
\end{aligned}
$$

###### 只测量两相

由基尔霍夫电流定律，有

$$
\vec I_a + \vec I_b + \vec I_c = 0
$$

* 若不测量C相电流，则 $I_c = - I_a - I_b$
  
  $$
  \begin{aligned}
I_\alpha &= \frac{2}{3} I_a - \frac{1}{3} I_b - \frac{1}{3} (-I_a - I_b)
\\
&= I_a
\\
I_\beta &= \frac{1}{\sqrt 3} I_b - \frac{1}{\sqrt 3} (- I_a - I_b)
\\
&= \frac{1}{\sqrt 3} I_a + \frac{2}{\sqrt 3} I_b
\end{aligned}
  $$

* 若不测量A相电流，则 $I_a = -I_b - I_c$
  
  $$
  \begin{aligned}
I_{\alpha} &= \frac{2}{3} [(-I_b-I_c) - \frac{1}{2} I_b - \frac{1}{2} I_c]
\\
&= - I_b - I_c
\\
I_{\beta} &= \frac{2}{3} [\frac{\sqrt{3}}{2} I_b - \frac{\sqrt{3}}{2} I_c]
\\
&= \frac{1}{\sqrt 3} I_b - \frac{1}{\sqrt 3} I_c
\end{aligned}
  $$

* 若不测量B相电流，则 $I_b = -I_a-I_c$
  
  $$
  \begin{aligned}
I_{\alpha} &= \frac{2}{3} [I_a - \frac{1}{2} (-I_a-I_c) - \frac{1}{2} I_c]
\\
&= I_a
\\
I_{\beta} &= \frac{2}{3} [\frac{\sqrt{3}}{2} (-I_a-I_c) - \frac{\sqrt{3}}{2} I_c]
\\
&= - \frac{1}{\sqrt 3} I_a - \frac{2}{\sqrt 3} I_c
\end{aligned}
  $$

###### 测量三相

在测量三相的情况下，实际上有一相是冗余的，但可以用来修正测量结果

修正方法为

* 首先计算中点的电流，理论上该值应该为0
  
  $$
  I_{mid} = \frac{1}{3} (I_a + I_b + I_c)
  $$

* 将 $I_{mid}$ 作为测量误差，补偿A B两相的测量值
  
  $$
  \begin{aligned}
I_a' &= I_a - I_{mid}
\\
I_b' &= I_b - I_{mid}
\end{aligned}
  $$

再使用修正的A B相电流计算clark变换

$$
\begin{aligned}
I_{\alpha} &= I_a'
\\
I_{\beta} &= \frac{1}{\sqrt 3} I_a' + \frac{2}{\sqrt 3} I_b'
\end{aligned}
$$

##### Park变换

park变换就很简单了，直接按公式来算，其中θ为电机测量到的转角

$$
\begin{aligned}
I_d &= I_{\alpha} cos \theta + I_{\beta} sin \theta
\\
I_q &= -I_{\alpha} sin \theta + I_{\beta} cos \theta
\end{aligned}
$$

### FOCMotor

定义了一系列FOC控制器中关于电机的参数与限制

#### FOCMotor类

##### 属性

```cpp
class FOCMotor
{
  public:
    // state variables
    // == 状态变量 ==
    float target; //!< current target value - depends of the controller
        // 电流目标值
    float shaft_angle;//!< current motor angle
        // 当前电机的旋转角度，该角度是编码器测的
    float electrical_angle;//!< current electrical angle
        // 当前电机的电气角度，该角度是FOC算法计算得到的
    float shaft_velocity;//!< current motor velocity
        // 当前电机速度 
    float current_sp;//!< target current ( q current )
        // 电机的目标电流（暂且不知道跟前一个目标值有什么区别）
    float shaft_velocity_sp;//!< current target velocity
        // 电机的目标速度
    float shaft_angle_sp;//!< current target angle
        // 电机的目标角度
    DQVoltage_s voltage;//!< current d and q voltage set to the motor
        // 当前设定的Ud和Uq
    DQCurrent_s current;//!< current d and q current measured
        // 当前测得的Id和Iq
    float voltage_bemf; //!< estimated backemf voltage (if provided KV constant)
        // 反电动势的估计值，需要提供KV值才能计算

    // motor configuration parameters
    // == 电机配置参数 ==
    float voltage_sensor_align;//!< sensor and motor align voltage parameter
        // 电机与传感器对齐时使用的电压
    float velocity_index_search;//!< target velocity for index search 

    // motor physical parameters
    // == 电机物理参数 ==
    float    phase_resistance; //!< motor phase resistance
        // 相电阻
    int pole_pairs;//!< motor pole pairs number
        // 极对数
    float KV_rating; //!< motor KV rating
        // KV值

    // limiting variables
    // == 限制参数 ==
    float voltage_limit; //!< Voltage limitting variable - global limit
        // 电压限制
    float current_limit; //!< Current limitting variable - global limit
        // 电流限制
    float velocity_limit; //!< Velocity limitting variable - global limit
        // 速度限制

    // motor status vairables
    // == 电机状态变量 ==
    int8_t enabled = 0;//!< enabled or disabled motor flag
        // 使能
    FOCMotorStatus motor_status = FOCMotorStatus::motor_uninitialized; //!< motor status
        // 电机初始化状态

    // pwm modulation related variables
    // == PWM相关变量 ==
    FOCModulationType foc_modulation;//!<  parameter derterniming modulation algorithm
        // pwm算法类型
    int8_t modulation_centered = 1;//!< flag (1) centered modulation around driver limit /2  or  (0) pulled to 0
        // 

    // configuration structures
    // == 配置结构 ==
    TorqueControlType torque_controller; //!< parameter determining the torque control type
    MotionControlType controller; //!< parameter determining the control loop to be used

    // controllers and low pass filters
    // == 控制器和低通滤波器 ==
    PIDController PID_current_q{DEF_PID_CURR_P,DEF_PID_CURR_I,DEF_PID_CURR_D,DEF_PID_CURR_RAMP, DEF_POWER_SUPPLY};//!< parameter determining the q current PID config
        // Iq的PID控制
    PIDController PID_current_d{DEF_PID_CURR_P,DEF_PID_CURR_I,DEF_PID_CURR_D,DEF_PID_CURR_RAMP, DEF_POWER_SUPPLY};//!< parameter determining the d current PID config
        // Id的PID控制
    LowPassFilter LPF_current_q{DEF_CURR_FILTER_Tf};//!<  parameter determining the current Low pass filter configuration 
        // Iq的低通滤波器
    LowPassFilter LPF_current_d{DEF_CURR_FILTER_Tf};//!<  parameter determining the current Low pass filter configuration 
        // Id的低通滤波器
    PIDController PID_velocity{DEF_PID_VEL_P,DEF_PID_VEL_I,DEF_PID_VEL_D,DEF_PID_VEL_RAMP,DEF_PID_VEL_LIMIT};//!< parameter determining the velocity PID configuration
        // 电压的PID
    PIDController P_angle{DEF_P_ANGLE_P,0,0,0,DEF_VEL_LIM};    //!< parameter determining the position PID configuration 
        // 位置PID
    LowPassFilter LPF_velocity{DEF_VEL_FILTER_Tf};//!<  parameter determining the velocity Low pass filter configuration 
        // 电压的低通滤波器
    LowPassFilter LPF_angle{0.0};//!<  parameter determining the angle low pass filter configuration 
        // 位置的低通滤波器
    unsigned int motion_downsample = DEF_MOTION_DOWNSMAPLE; //!< parameter defining the ratio of downsampling for move commad
    unsigned int motion_cnt = 0; //!< counting variable for downsampling for move commad

    // sensor related variabels
    // == 传感器相关变量 ==
    float sensor_offset; //!< user defined sensor zero offset
    float zero_electric_angle = NOT_SET;//!< absolute zero electric angle - if available
    int sensor_direction = NOT_SET; //!< if sensor_direction == Direction::CCW then direction will be flipped to CW

    unsigned int monitor_downsample = DEF_MON_DOWNSMAPLE; //!< show monitor outputs each monitor_downsample calls 
    // initial monitoring will display target, voltage, velocity and angle
    uint8_t monitor_variables = _MON_TARGET | _MON_VOLT_Q | _MON_VEL | _MON_ANGLE; //!< Bit array holding the map of variables the user wants to monitor

    /** 
      * Sensor link:
      * - Encoder 
      * - MagneticSensor*
      * - HallSensor
    */
    Sensor* sensor; 
    /** 
      * CurrentSense link
    */
    CurrentSense* current_sense; 

    // monitoring functions
    Print* monitor_port; //!< Serial terminal variable if provided
  private:
    // monitor counting variable
    unsigned int monitor_cnt = 0 ; //!< counting variable
};
```

##### 虚函数

* init()  初始化

* enable()  disable()  使能

* initFOC()  初始化FOC算法，对齐传感器与电机的零点

* loopFOC()  实时运行FOC算法

* move()  根据BLDCMotor设置的参数自动运行FOC控制循环

* setPhaseVoltage()  根据电机的电气角度设置相电压

##### 方法

* linkSensor()  设置电机与传感器相连接

* linkCurrentSense()  设置电机与电流采样相连接

* shaftAngle()  计算轴倾角

* shaftVelocity()  计算角速度

* electricalAngle()  计算电气角度

* useMonitoring()  monitor()  设置串口监视

### Sensor

#### Sensor类

##### 属性

```cpp
class Sensor{
    public:
        float min_elapsed_time = 0.000100; // default is 100 microseconds, or 10kHz
            // 
    protected:
        // velocity calculation variables
        // == 速度计算 ==
        float velocity=0.0f;
            // 速度
        float angle_prev=0.0f; // result of last call to getSensorAngle(), used for full rotations and velocity
            // 当前的角度
        long angle_prev_ts=0; // timestamp of last call to getAngle, used for velocity
            // 当前采样角度的时间戳
        float vel_angle_prev=0.0f; // angle at last call to getVelocity, used for velocity
            // 先前的角度
        long vel_angle_prev_ts=0; // last velocity calculation timestamp
            // 先前采样角速度的时间戳
        int32_t full_rotations=0; // full rotation tracking
            // 当前转了几圈
        int32_t vel_full_rotations=0; // previous full rotation value for velocity calculation
            // 先前的圈数
};
```

##### 虚函数

* getMechanicalAngle()  获取机械角度

* getAngle()  获取当前旋转的总角度

* getPreciseAngle()  获取当前旋转的总角度（较精确）

* getVelocity()  获取角速度

* getFullRotations()  获取当前圈数

* update()  对角度进行一次采样

* needsSearch()

* getSensorAngle()  获取传感器采样的角度

* init()  初始化

##### 方法

Sensor类的全部方法都是虚函数，在sensor.cpp中提供了一套实现

* getMechanicalAngle()  直接返回angle_prev属性

* getAngle()  `当前转的圈数乘2π+angle_prev`

* getPreciseAngle()  与getAngle()一样，只不过返回的是double

* getVelocity()  `[ (当前圈数-先前的圈数)*2π+(当前角度-先前角度) ] / 时间戳长度`

* getFullRotations()  直接返回full_rotations属性

* update()  getSensorAngle获取传感器角度，计算是否溢出，若溢出则记录圈数。最后将angle_prev更新为新的采样值

* init()  该函数主要是通过进行4次采样，对类内部的一些参数进行初始化，实现平滑的启动
