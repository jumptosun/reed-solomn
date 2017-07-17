# Reed-Solomon

reed-solomon算法的C++实现，移植自reed-solomon的go语言实现 https://github.com/klauspost/reedsolomon　　　

## 使用
.   
|--- README.md   
|--- CMakeLists.txt   
|--- src   
|--- include   

在项目根目录下运行 make。在自己的项目里包含 include 下的 reed_solomon.hpp，并且链接生成的 ./lib/libvcrs.a 即可使用。
或者直接拷贝 src 下的源码至自己的工程，手动编译使用。

编码用例请阅读 src/example/demo.cpp

	// get new reed-solomon instance
	ReedSolomon* rs = new ReedSolomon();
	rs->Initialize(10,4);

	// encode
	rs->Encode(origin);
	// reconstruct
	rs->Reconstruct(origin,64);

## 配置
Reed-Solomon 默认是线程不安全的，并且默认日志输出为标准输出。通过修改rs_config.hpp　文件来确保线程安全，或修改日志输出。修改后，需要使用 cmake 重新编译。   
*NOTING* :　只有当 reconstruct 分布在多个线程时才用开启线程安全。   

	// whether turn on the thread safety
	#undef RS_CONFIG_OPTION_THREAD_SAFETY
	// change the log funtion if need
	#define rs_log(msg, ...) printf(msg, ##__VA_ARGS__)
