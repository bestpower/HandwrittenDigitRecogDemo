package com.example.paxsz.handwritedigitrecognize;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
/**
 * Created by wyu on 2020/8/19.
 */

public class HandWriteView extends View{

    public Bitmap returnBitmap(){
        return mBitmap;
    }
    private Paint mPaint;
    private float degrees=0;
    private int mLastX, mLastY, mCurrX, mCurrY;
    private Bitmap mBitmap;

    public HandWriteView(Context context) {
        super(context);
        init();
    }

    public HandWriteView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public HandWriteView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }
    private void init(){
        //设置画笔属性
        mPaint = new Paint();
//        mPaint.setColor(Color.WHITE);
        mPaint.setARGB(255,255,255,255);
        mPaint.setStrokeWidth(70);
        mPaint.setAntiAlias(true);
        mPaint.setDither(true);
        mPaint.setStrokeCap(Paint.Cap.ROUND);
        mPaint.setStrokeJoin(Paint.Join.ROUND);

        if (mBitmap == null) {
            mBitmap = Bitmap.createBitmap(720, 720, Bitmap.Config.ARGB_8888);
            Canvas backGroundCanvas = new Canvas(mBitmap);
            backGroundCanvas.drawARGB(255,0,0,0);
        }
    }
    @Override
    public void draw(Canvas canvas) {
        super.draw(canvas);

        int width = getWidth();
        int height = getHeight();
        if (mBitmap == null) {
            mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        }
        canvas.drawBitmap(mBitmap, 0, 0, mPaint);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {

        mLastX = mCurrX;
        mLastY = mCurrY;
        mCurrX = (int) event.getX();
        mCurrY = (int) event.getY();

        switch (event.getAction()) {
            case MotionEvent.ACTION_DOWN:
                mLastX = mCurrX;
                mLastY = mCurrY;
                break;
            default:
                break;
        }
        updateDrawHandWrite();
        return true;
    }

    private void updateDrawHandWrite(){
//        int width = getWidth();
//        int height = getHeight();

//        if (mBitmap == null) {
//            mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
//        }

        Canvas tmpCanvas = new Canvas(mBitmap);

        tmpCanvas.drawLine(mLastX, mLastY, mCurrX, mCurrY, mPaint);
        invalidate();
    }

    public void clearDraw(){
        mBitmap = null;
        invalidate();
    }
}
