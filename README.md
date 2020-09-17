# HandwrittenDigitRecogDemo
手写数字识别AI模型在安卓端的调用示例

### 开发环境

> OS:Windows 10

> IDE: Android Studio 3.2

> Java(JDK) 1.8

> Android SDK 28

> Android NDK 19

> Cmake 3.6

### AI模型部署工具

>  NCNN (https://github.com/Tencent/ncnn)

#### AI模型部署工具编译环境

> Ubuntu 18.04

> python 3.6.9

> cmake 3.10.2

> Android NDK(android-ndk-r19c)

> libopencv-dev

> protobuf 3.5.1
(https://github.com/protocolbuffers/protobuf)

#### AI模型编译及部署
##### 模型转换
###### pytorch转onnx

'''python

    device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
    input = torch.randn(1, 1, 28, 28, device=device)
    model = {Your_Net_Model_Class_Name}().to(device)
    net.load_state_dict(torch.load(input_pytorch_model_path))
    net.eval()
    input_names = ['data']
    output_names = ['prob']
    torch.onnx._export(model, input, output_onnx_model_path,
                       export_params=True, verbose=True, 
                       input_names=input_names, output_names=output_names)
'''

###### 精简化onnx

'''

    sudo pip3 install onnx-simplifier
    python3 -m onnxsim ./Models/LeNet_p27.onnx ./Models/LeNet_p27_sim.onnx
'''

###### 编译安装protobuf

'''

    tar -zxvf protobuf3.5.1.tar.gz 
    cd protobuf-3.5.1/
    ./autogen.sh
    ./configure
    make -j4
    sudo make install
    sudo ldconfig
    protoc --version
'''

###### 编译ncnn相关转换工具

'''

    cd {Your_Path}/ncnn/
    mkdir -p build
    cd build
    cmake ..
    make -j4

###### onnx转ncnn

'''

    cd ${ncnn_path}/nuild/tools/onnx/
    cp ${your_onnx_file_path} ./
    ./onnx2ncnn ${your_onnx_file} ${your_ncnn_param_file_name}.param ${your_ncnn_bin_file_name}.bin
'''

执行完以上命令，会得到 *.param和*.bin两个文件，可直接用于安卓应用中部署

'''

    ./ncnn2mem ${your_ncnn_param_file_name}.param ${your_ncnn_bin_file_name}.bin ${your_ncnn_file_name}.id.h ${your_ncnn_file_name}.mem.h
'''

执行完以上命令，会得到 *.param.bin、*.bin、*.id.h、*.mem.h四个文件，可用于安卓应用中加密部署（无法通过反编译窥探网络模型相关信息）

对于加密方式调用：

拷贝*.param.bin、*.bin两个文件到安卓应用工程中的asset文件夹下

拷贝*.id.h到安卓应用工程中的cpp/include文件夹下

##### 安卓端ncnn调用库编译

###### 32位 armV7 cpu

'''

    cd {ncnn_path}/
    mkdir -p build-android-armv7
    cd build-android-armv7/
    export ANDROID_NDK=${Your_ndk_dir_path}
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI="armeabi-v7a" -DANDROID_ARM_NEON=ON -DANDROID_PLATFORM=android-14 ..
    make -j4
    make install
'''

###### 32位 armv7 gpu vulkan

'''
    mkdir -p build-android-armv7-vk
    cd build-android-armv7-vk/
    export ANDROID_NDK=${Your_ndk_dir_path}
    export VULKAN_SDK=${Your_vulkan_sdk_dir_path}
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI="armeabi-v7a" -DANDROID_ARM_NEON=ON -DANDROID_PLATFORM=android-24 -DNCNN_VULKAN=ON ..
    make -j4
    make install
'''

###### 64位 armv8 cpu

'''

    mkdir -p build-android-aarch64
    cd build-android-aarch64/
    export ANDROID_NDK=${Your_ndk_dir_path}
    cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI="arm64-v8a" -DANDROID_PLATFORM=android-21 ..
    make -j4
    make install
'''

##### 安卓端ncnn库调用

###### 拷贝上一步编译生成的库文件和头文件到对应安卓工程文件夹中

'''

    cd ${ncnn_path}/${ncnn_android_build_path}/install
    cp -r include/ncnn ${Your_android_project_cpp_dir}/include/
    cp lib/libncnn.a ${Your_android_project_jniLibs_dir}/${ANDROID_ABI}/
'''

###### 配置app/build.gradle中ndk编译相关

'''gradle

    android {
        defaultConfig {
            ndk {
                    abiFilters "armeabi-v7a"
                    stl "gnustl_static"
            }
            externalNativeBuild {
                cmake {
                    arguments "-DANDROID_TOOLCHAIN=clang"
                    cFlags "-fopenmp -O2 -fvisibility=hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math "
                    cppFlags "-fopenmp -O2 -fvisibility=hidden -fvisibility-inlines-hidden -fomit-frame-pointer -fstrict-aliasing -ffunction-sections -fdata-sections -ffast-math "
                    arguments "-DANDROID_STL=c++_shared", "-DANDROID_CPP_FEATURES=rtti exceptions"
                    cppFlags ""
                    cppFlags "-std=c++11"
                    cppFlags "-frtti"
                    cppFlags "-fexceptions"
                }
            }
        }
        externalNativeBuild {
            cmake {
                path "CMakeLists.txt"
            }
        }
    }
'''



###### 配置CMakeLists.txt

'''txt

    添加ncnn库
    add_library(libncnn STATIC IMPORTED )
    set_target_properties(
                        libncnn
                        PROPERTIES IMPORTED_LOCATION
                        ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libncnn.a
                        )
    #添加工程所依赖的库
    find_library(log-lib log android)
    
    target_link_libraries( HwDr
                           libncnn
                           jnigraphics
                           z
                           ${log-lib}
                           android )
'''
                       
###### jni调用代码编写

'''cpp
        
    //模型加载不分关键代码
    ncnn::Option opt;
    opt.blob_allocator = &g_blob_pool_allocator;
    opt.workspace_allocator = &g_workspace_pool_allocator;
    DigitRecognet.opt = opt;
    // init param
    {
        int retp = DigitRecognet.load_param_bin(mgr, "Mnist/models/LeNet_p27_sim.param.bin");
        if (retp != 0)
        {
            LOGE("load_param_bin failed");
        }
    }
    // init bin
    {
        int retm = DigitRecognet.load_model(mgr, "Mnist/models/LeNet_p27_sim.bin");
        if (retm != 0)
        {
            LOGE("load_model_bin failed");
        }
    }
    
    //模型推理部分关键代码
    ncnn::Extractor ex = DigitRecognet.create_extractor();
    ex.set_num_threads(threadnum);
    ex.set_light_mode(true);
    ex.input(LeNet_p27_sim_param_id::BLOB_data, img_);
    ncnn::Mat out;
    ex.extract(LeNet_p27_sim_param_id::BLOB_prob, out);
    //输出数据概率化
    {
        ncnn::Layer* softmax = ncnn::create_layer("Softmax");
        ncnn::ParamDict pd;
        softmax->load_param(pd);
        softmax->forward_inplace(out, DigitRecognet.opt);
        delete softmax;
    }
    out = out.reshape(out.w * out.h * out.c);
    unsigned int out_size = (unsigned int)(out.w);
    feature_out.resize(out_size);
    //赋值输出数据
    for (int j = 0; j < out.w; j++)
    {
        feature_out[j] = out[j];
    }
    
    //jni数据输入与结果输出部分关键代码
    //字节数组预处理
    jbyte *digitImgData = env->GetByteArrayElements(digitImgData_, NULL);
    unsigned char *digitImgCharData = (unsigned char *) digitImgData;
    //转换图片数据格式
    ncnn::Mat ncnn_img = ncnn::Mat::from_pixels_resize(digitImgCharData, ncnn::Mat::PIXEL_RGBA2GRAY, w, h, 28, 28);
    //输入数据归一化
    const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
    ncnn_img.substract_mean_normalize(0, norm_vals);
    std::vector<float> feature;
    //数字手写识别推理
    mDigitRecog->start(ncnn_img, feature);
    //提取并赋值概率数组
    float *featureInfo = new float[10];
    for(int i = 0;i<10;i++){
        featureInfo[i] = feature[i];
    }
'''

###### java调用代码编写

'''java

    //native方法类部分关键代码
    //jni编译so库加载
    static {
        System.loadLibrary("HwDr");
    }
    //模型初始化
    public native boolean MnistAssetModelInit(AssetManager amgr);
    //模型反初始化
    public native boolean MnistModelUnInit();
    //模型推理
    public native float[] HwDigitRecog(byte[] digitImgData, int w, int h);
    //加密方式初始化模型
    boolean init = false;
    public HwDr(AssetManager assetManager){
        init = MnistAssetModelInit(assetManager);
        if(init) {
            Log.i(TAG, "模型初始化成功");
        }
    }
    
    //Bitmap(手写数字黑底白字图像)转Byte[]
    int bytes = image.getByteCount();
    ByteBuffer buffer = ByteBuffer.allocate(bytes);
    image.copyPixelsToBuffer(buffer);
    byte[] byteTemp = buffer.array();

'''


