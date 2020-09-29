package paxsz.ai;

import android.content.res.AssetManager;
import android.graphics.Bitmap;

import com.demo.pax.face.CpuUtils;

/**
 * Created by wyu on 2020/8/27.
 */
public class HwDr {
    private static final String TAG = "HwDrNativeLib";
    static {
        System.loadLibrary("HwDr");
    }
    //模型初始化
    public native boolean MnistAssetModelInit(AssetManager amgr);
    //线程数设置
    public native boolean SetThreadsNumber(int threadsNumber);
    //模型反初始化
    public native boolean MnistModelUnInit();
    //模型推理（三种输入方式）
    public native float[] HwDigitRecog(byte[] digitImgData, int w, int h);//字节数组输入
    public native float[] HwDigitRecogFromBitmap(Bitmap digitImgBitmap);//位图输入
    public native float[] HwDigitRecogFromPath(String imgPath);//图片路径输入

    private boolean init;//模型初始化标志
    private int availableThreadsNum = 0;//可调用线程数

    //初始化模型
    public HwDr(AssetManager assetManager){
        init = MnistAssetModelInit(assetManager);
        if(init) {
            availableThreadsNum = CpuUtils.getNumberOfCPUCores();//获取cpu核心数
            SetThreadsNumber(availableThreadsNum);//按cpu核心数分配线程数
        }
    }
}
