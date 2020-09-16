package paxsz.ai;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

import com.demo.pax.face.CpuUtils;

public class HwDr {
    private static final String TAG = "HwDrNativeLib";
    static {
        System.loadLibrary("HwDr");
    }
    //模型初始化
    public native boolean MnistModelInit(String mnistModelPath);
    public native boolean MnistAssetModelInit(AssetManager amgr);//加密模型加载
    //线程设置
    public native boolean SetThreadsNumber(int threadsNumber);
    //模型反初始化
    public native boolean MnistModelUnInit();
    //模型推理
    public native float[] HwDigitRecog(byte[] digitImgData, int w, int h);//字节数组输入
    public native float[] HwDigitRecogFromBitmap(Bitmap digitImgBitmap, int w, int h);//位图输入
    public native float[] HwDigitRecogFromPath(String imgPath);//图片路径输入

    private boolean init;//模型初始化标志
    private int availableThreadsNum = 0;//可调用线程数

    //从本地路径初始化模型
    public HwDr(String modelPath){
        init = MnistModelInit(modelPath);
        if(init) {
            availableThreadsNum = CpuUtils.getNumberOfCPUCores();
            SetThreadsNumber(availableThreadsNum);
        }
    }

    //加密方式初始化模型
    public HwDr(AssetManager assetManager){
        init = MnistAssetModelInit(assetManager);
        if(init) {
            availableThreadsNum = CpuUtils.getNumberOfCPUCores();
            SetThreadsNumber(availableThreadsNum);
        }
    }
}
