package com.example.paxsz.handwritedigitrecognize;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.net.Uri;
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

import java.io.BufferedOutputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Locale;

import paxsz.ai.HwDr;

public class MainActivity extends Activity {
    private static final String TAG = "Hand_writing";
    //权限检查
    private final static int REQUEST_EXTERNAL_STORAGE = 0x222;
    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };
    private static final int FILE_SELECT_CODE = 1000;
    //手写区域视图
    HandWriteView mHandWriteView;
    //结果显示视图
    TextView mResultView;
    //JNI方法调用类
    HwDr mHwDr;
    //图片路径方式
    Uri selectUri;
    Bitmap fileImgBitmap;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        verifyStoragePermissions(this);

        mHandWriteView = (HandWriteView) findViewById(R.id.handWriteView);
        mResultView = (TextView) findViewById(R.id.resultShow);

        Button mInitBtn = (Button) findViewById(R.id.btnInitModel);
        mInitBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //初始化模型
                initModels();
            }
        });

        Button mRecognizeBtn = (Button) findViewById(R.id.btnRecognize);
        mRecognizeBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //手写识别
                buttonRecognizeOnClick(view);
            }
        });

        Button mSelectBtn = (Button) findViewById(R.id.btnSelect);
        mSelectBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //选择图片识别
                chooseLocalFile();
            }
        });

        Button mClearDrawBtn = (Button) findViewById(R.id.btnClear);
        mClearDrawBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                //清空画布
                buttonClearDrawOnClick(view);
            }
        });

    }

    //初始化模型
    public void initModels(){
        //模型初始化
        File sdDir = Environment.getExternalStorageDirectory();//获取跟目录
        String sdPath = sdDir.toString() + "/Mnist/models/";//人脸检测模型
//        if(mHwDr == null) mHwDr = new HwDr(sdPath);//普通加载
        if(mHwDr == null) mHwDr = new HwDr(getAssets());//加密加载
    }

    //反初始化模型
    private void unInitModel(){
        //人脸检测模型
        if (mHwDr != null) {
            mHwDr.MnistModelUnInit();
            mHwDr = null;
        }
    }
    int input_num = 0;
    private void buttonRecognizeOnClick(View v){
        long timeRecognizeAll = System.currentTimeMillis();
        Bitmap tmpBitmap = mHandWriteView.returnBitmap();
        Log.i(TAG, "HandWriteView_W_H = " + tmpBitmap.getWidth() + " " + tmpBitmap.getHeight());
        if(null == tmpBitmap)
            return;
        //预处理
//        byte[] tmpBytes = getPixelsRGBA(tmpBitmap);
//        Bitmap waitRecognizeBitmap = Utils.scaleBitmap(tmpBitmap, 28, 28, 0, false);

        long timeLeNetPredict = System.currentTimeMillis();
        //推理
//        float[] response = mHwDr.HwDigitRecog(tmpBytes, tmpBitmap.getWidth(), tmpBitmap.getHeight());
        float[] response = mHwDr.HwDigitRecogFromBitmap(tmpBitmap, tmpBitmap.getWidth(), tmpBitmap.getHeight());

        timeLeNetPredict = System.currentTimeMillis() - timeLeNetPredict;
        Log.i(TAG, "调用模型时间：" + timeLeNetPredict);
        //推理结果处理
        String[] resposeStrs = new String[response.length];
        for(int i=0;i<response.length;i++){
            resposeStrs[i] = String.format("%.3f", response[i]);
        }
        int prindict_num = Utils.getMaxIndex(response);
        //结果显示
        mResultView.setText(mResultView.getText()+Arrays.toString(resposeStrs) +
                "\n结果为：" + String.valueOf(prindict_num));
        //Toast.makeText(getApplicationContext(),"The predict label is "+String.valueOf(response),Toast.LENGTH_SHORT).show();
        timeRecognizeAll = System.currentTimeMillis() - timeRecognizeAll;
        Log.i(TAG, "总识别处理时间：" + timeRecognizeAll);
    }

    private void recognizeFromPath(Intent data){
        mHandWriteView.clearDraw();
        selectUri = data.getData();
        try {
            try {
                fileImgBitmap = Utils.decodeUri(MainActivity.this, selectUri, 720, 720);
                Canvas canvas = new Canvas(Bitmap.createBitmap(720, 720, Bitmap.Config.ARGB_8888));
                mHandWriteView.drawBitmap(canvas, fileImgBitmap);
                Log.d(TAG, "获取图片显示");
                //推理
                long timeLeNetPredict = System.currentTimeMillis();
                float[] response = mHwDr.HwDigitRecogFromPath(Utils.getPhotoPathFromContentUri(this, selectUri));
                String[] resposeStrs = new String[response.length];
                for(int i=0;i<response.length;i++){
                    resposeStrs[i] = String.format("%.3f", response[i]);
                }
                int prindict_num = Utils.getMaxIndex(response);
                timeLeNetPredict = System.currentTimeMillis() - timeLeNetPredict;
                Log.i(TAG, "调用模型时间：" + timeLeNetPredict);
                mResultView.setText(mResultView.getText()+Arrays.toString(resposeStrs) + "\n结果为：" + String.valueOf(prindict_num));
            }catch (FileNotFoundException e){
                e.getStackTrace();
            }
        }catch (NullPointerException ne){
            Log.e(TAG, "Can't find uri");
        }
    }

    private void buttonClearDrawOnClick(View v){
        mResultView.setText("The recognition result is: ");
        mHandWriteView.clearDraw();
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

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        if (resultCode != Activity.RESULT_OK) {
            Log.e(TAG, "onActivityResult() error, resultCode: " + resultCode);
            super.onActivityResult(requestCode, resultCode, data);
            return;
        }
        //接收图片选择回调结果
        if (requestCode == FILE_SELECT_CODE) {
            recognizeFromPath(data);
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    //本地图片选择
    private void chooseLocalFile() {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("image/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        try {
            startActivityForResult(Intent.createChooser(intent, "选择文件"), FILE_SELECT_CODE);
        } catch (android.content.ActivityNotFoundException ex) {
            Toast.makeText(this, "亲，木有文件管理器啊-_-!!", Toast.LENGTH_SHORT).show();
        }
    }

    void InitModelFiles()
    {
        try {
            //ncnn
            String assetPath = "Mnist";
            String sdcardPath = Environment.getExternalStorageDirectory()
                    + File.separator + assetPath;
            Utils.copyFilesFromAssets(this, assetPath, sdcardPath);
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
        finish();
    }
}
