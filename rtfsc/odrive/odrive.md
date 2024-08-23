## nvm_config.hpp

### Config

可变参数模板类，用于保存一系列的配置

```cpp
template<typename T, typename ... Ts>
struct Config<T, Ts...> 
{
    // 获取配置长度
    static size_t get_size();
    
    // 加载配置
    // * offset  读取配置的偏移
    // * crc16   返回计算的配置文件crc16
    // * val0    读取的第一个配置
    // * vals    读取的其他配置
    static int load_config(size_t offset, uint16_t* crc16, T* val0, Ts* ... vals);

    // 保存配置
    // * offset  保存配置的偏移
    // * crc16   返回计算的配置文件crc16
    // * val0    保存的第一个配置
    // * vals    保存的其他配置
    static int store_config(size_t offset, uint16_t* crc16, const T* val0, const Ts* ... vals);

    // load_config前对要读入的配置大小进行判断（计算val0 vals的长度是否大于NVM的大小），并且是从偏移0开始读取
    static int safe_load_config(T* val0, Ts* ... vals);

    // store_config前对写入的大小进行判断（计算val0 vals的长度是否大于NVM的大小），从偏移0写入，并且在最后写入crc16值
    static int safe_store_config(const T* val0, const Ts* ... vals);
}
```

## motor.cpp

