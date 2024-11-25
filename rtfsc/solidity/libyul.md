## 调用链

```
solc到libyul优化的调用链
    solc/main.cpp -> solc/CommandLineInterface.cpp|CommandLineInterface::run -> ::processInput -> ::assembleYul -> 
        1. libyul/YulStack.h|yul::YulStack
        2. libyul/YulStack.cpp|yul::YulStack::optimize -> libyul/ObjectOptimizer.cpp|ObjectOptimizer::optimize -> 
           libyul/optimiser/Suite.cpp|OptimiserSuite::run -> libyul/optimiser//StackLimitEvader.cpp|StackLimitEvader::run
```

```
solc到语法解析的调用链
    solc/main.cpp -> solc/CommandLineInterface.cpp|CommandLineInterface::run -> ::processInput -> ::compile -> 
           libsolidity/interface/CompilerStack.cpp|CompilerStack::compile -> ::parseAndAnalyze -> 
               1. ::parse -> libsolidity/parsing/Parser.cpp|Parser::parse
```




















