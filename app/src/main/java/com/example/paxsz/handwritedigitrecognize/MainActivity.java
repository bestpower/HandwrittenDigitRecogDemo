package com.example.paxsz.handwritedigitrecognize;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.example.paxsz.handwritedigitrecognize.HandWriteView;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;

import paxsz.ai.HwDr;

public class MainActivity extends Activity {
    private static final String TAG = "Hand_writing";
    private final static int REQUEST_EXTERNAL_STORAGE = 0x222;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };
    HandWriteView mHandWriteView;
//    CvSVM mClassifier;
//    File mSvmModel;
    TextView mResultView;

    HwDr mHwDr;

//    static{ System.loadLibrary("opencv_java"); }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        verifyStoragePermissions(this);

        mHandWriteView = (HandWriteView) findViewById(R.id.handWriteView);
        mResultView = (TextView) findViewById(R.id.resultShow);

        Button mInitBtn = (Button) findViewById(R.id.btnInitModel);
        mInitBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                initModels();
            }
        });

        Button mRecognizeBtn = (Button) findViewById(R.id.btnRecognize);
        mRecognizeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                buttonRecognizeOnClick(view);
            }
        });

        Button mClearDrawBtn = (Button) findViewById(R.id.btnClear);
        mClearDrawBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                buttonClearDrawOnClick(view);
            }
        });

//        mClassifier = new CvSVM();

        //
//        try {
//            // load cascade file from application resources
//            InputStream is = getResources().openRawResource(R.raw.mnist);
//            File mnist_modelDir = getDir("mnist_model", Context.MODE_PRIVATE);
//            mSvmModel = new File(mnist_modelDir, "mnist.xml");
//            FileOutputStream os = new FileOutputStream(mSvmModel);
//
//            byte[] buffer = new byte[4096];
//            int bytesRead;
//            while ((bytesRead = is.read(buffer)) != -1) {
//                os.write(buffer, 0, bytesRead);
//            }
//            is.close();
//            os.close();
//
////            mClassifier.load(mSvmModel.getAbsolutePath());
//
//            mnist_modelDir.delete();
//
//        } catch (IOException e) {
//            e.printStackTrace();
//            Log.e(TAG, "Failed to load cascade. Exception thrown: " + e);
//        }
    }

    //初始化模型
    public void initModels(){
        //模型初始化
        File sdDir = Environment.getExternalStorageDirectory();//获取跟目录
        String sdPath = sdDir.toString() + "/Mnist/models/";//人脸检测模型
        if(mHwDr == null) mHwDr = new HwDr(sdPath);
    }

    //反初始化模型
    private void unInitModel(){
        //人脸检测模型
        if (mHwDr != null) {
            mHwDr.MnistModelUnInit();
            mHwDr = null;
        }
    }

    private void buttonRecognizeOnClick(View v){
        long timeRecognizeAll = System.currentTimeMillis();
        Bitmap tmpBitmap = mHandWriteView.returnBitmap();
        Log.i(TAG, "HandWriteView_W_H = " + tmpBitmap.getWidth() + " " + tmpBitmap.getHeight());
        if(null == tmpBitmap)
            return;
        //todo 预处理
        byte[] tmpBytes = getPixelsRGBA(tmpBitmap);

//        Mat tmpMat = new Mat(tmpBitmap.getHeight(),tmpBitmap.getWidth(),CvType.CV_8UC3);
//        Mat saveMat = new Mat(tmpBitmap.getHeight(),tmpBitmap.getWidth(),CvType.CV_8UC1);
//
//        Utils.bitmapToMat(tmpBitmap,tmpMat);
//
//        Imgproc.cvtColor(tmpMat, saveMat, Imgproc.COLOR_RGBA2GRAY);

//        final String galleryPath = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES).toString();
//        String picturePath = galleryPath+"/handwrite.bmp";
//        if(!Highgui.imwrite(picturePath,saveMat)){
//            Toast.makeText(getApplicationContext(),"Failed to save the picture",Toast.LENGTH_SHORT).show();
//        }
//        else
//        {
//            Toast.makeText(getApplicationContext(),"Succeed to save the picture",Toast.LENGTH_SHORT).show();
//        }
        //
//        int imgVectorLen = 28 * 28;
//        Mat dstMat = new Mat(28,28,CvType.CV_8UC1);
//        Mat tempFloat = new Mat(28,28,CvType.CV_32FC1);
//
//        Imgproc.resize(saveMat, dstMat, new Size(28,28));
//        dstMat.convertTo(tempFloat, CvType.CV_32FC1);
//
//        Mat predict_mat = tempFloat.reshape(0,1).clone();
//        Core.normalize(predict_mat,predict_mat,0.0,1.0,Core.NORM_MINMAX);
//
        long timeSvmPredict = System.currentTimeMillis();
        //todo 推理
//        int response = (int)mClassifier.predict(predict_mat);
        float[] response = mHwDr.HwDigitRecog(tmpBytes, tmpBitmap.getWidth(), tmpBitmap.getHeight());

        timeSvmPredict = System.currentTimeMillis() - timeSvmPredict;
        Log.i(TAG, "调用SVM模型时间：" + timeSvmPredict);

        mResultView.setText(mResultView.getText()+Arrays.toString(response));
        //Toast.makeText(getApplicationContext(),"The predict label is "+String.valueOf(response),Toast.LENGTH_SHORT).show();
        timeRecognizeAll = System.currentTimeMillis() - timeRecognizeAll;
        Log.i(TAG, "总识别处理时间：" + timeRecognizeAll);
    }

    private void buttonClearDrawOnClick(View v){
        mResultView.setText("The recognition result is: ");
        mHandWriteView.clearDraw();
    }

    //提取位图像素点
    private byte[] getPixelsRGBA(Bitmap image) {
        int bytes = image.getByteCount();
        ByteBuffer buffer = ByteBuffer.allocate(bytes); // Create a new buffer
        image.copyPixelsToBuffer(buffer); // Move the byte data to the buffer
        byte[] byteTemp = buffer.array(); // Get the underlying array containing the
        return byteTemp;
    }

    //存储权限检查
    void verifyStoragePermissions(Activity activity) {
        try {
            //检测是否有写的权限
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                int permission_1 = ActivityCompat.checkSelfPermission(activity,
                        PERMISSIONS_STORAGE[0]);
                int permission_2 = ActivityCompat.checkSelfPermission(activity,
                        PERMISSIONS_STORAGE[1]);
                if (permission_1 != PackageManager.PERMISSION_GRANTED ||
                        permission_2 != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE, REQUEST_EXTERNAL_STORAGE);
                }else{
                    InitModelFiles();
                }
            }else{
                InitModelFiles();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case REQUEST_EXTERNAL_STORAGE: {
                // 如果请求被拒绝，那么通常grantResults数组为空
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    //申请成功，进行相应操作
                    Log.i(TAG, "存储权限申请成功");
                    //拷贝
                    InitModelFiles();
                } else {
                    //申请失败，可以继续向用户解释。
                    Toast.makeText(this, "存储权限申请失败，应用将无法正常使用", Toast.LENGTH_SHORT).show();
                }
                return;
            }
        }
    }

    void copyFilesFromAssets(Context context, String oldPath, String newPath) {
        try {
            String[] fileNames = context.getAssets().list(oldPath);
            if (fileNames.length > 0) {
                // directory
                File file = new File(newPath);
                if (!file.mkdir())
                {
                    Log.d("mkdir","can't make folder");
                }

                for (String fileName : fileNames) {
                    if(!file.mkdir()){
                        new File(newPath + "/" + fileName).deleteOnExit();
                    }
                    copyFilesFromAssets(context, oldPath + "/" + fileName,
                            newPath + "/" + fileName);
                }
            } else {
                // file
                InputStream is = context.getAssets().open(oldPath);
                FileOutputStream fos = new FileOutputStream(new File(newPath));
                byte[] buffer = new byte[1024];
                int byteCount;
                while ((byteCount = is.read(buffer)) != -1) {
                    fos.write(buffer, 0, byteCount);
                }
                fos.flush();
                is.close();
                fos.close();
            }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    void InitModelFiles()
    {
        try {
            //ncnn
            String assetPath = "Mnist";
            String sdcardPath = Environment.getExternalStorageDirectory()
                    + File.separator + assetPath;
            copyFilesFromAssets(this, assetPath, sdcardPath);
        }catch (Exception e){
            Log.e(TAG, "模型拷贝失败：" + e.getMessage());
        }
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unInitModel();
    }
}
