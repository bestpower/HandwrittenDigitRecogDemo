package paxsz.ai;

import android.content.res.AssetManager;
import com.demo.pax.face.CpuUtils;

public class HwDr {
    private static final String TAG = "HwDrNativeLib";
    static {
        System.loadLibrary("HwDr");
    }
    //模型初始化
    public native boolean MnistModelInit(String mnistModelPath);
//    public native boolean ModelsInit(AssetManager amgr);//加密模型加载
    //线程设置
    public native boolean SetThreadsNumber(int threadsNumber);
    //模型反初始化
    public native boolean MnistModelUnInit();
    //模型推理
    public native float[] HwDigitRecog(byte[] digitImgData, int w, int h);//字节数组输入

    private boolean init;
    private int availableThreadsNum = 0;

    public HwDr(String modelPath){
        init = MnistModelInit(modelPath);
        if(init) {
            availableThreadsNum = CpuUtils.getNumberOfCPUCores();
            SetThreadsNumber(availableThreadsNum);
        }
    }
}
