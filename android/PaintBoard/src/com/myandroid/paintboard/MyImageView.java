package com.myandroid.paintboard;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.util.AttributeSet;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

public class MyImageView extends ImageView {
	private Bitmap mBitmap;
	public boolean mRepaint = false;
	private int mChosenColor = 0xffff0000;
	
	private int mImageIndex = 0;
	
	public MyImageView(Context context, AttributeSet attrs) {
		super(context, attrs);
		
		Log.v(MainActivity.LOGTAG,
				"MyImageView constructor(,), " + System.currentTimeMillis() + 
				", width = " + this.getWidth() + 
				", height = " + this.getHeight());
		
		//mPaint = new Paint();
		
	}
	
	public MyImageView(Context context) {
		super(context);
		
		Log.v(MainActivity.LOGTAG,
				"MyImageView constructor(), " + System.currentTimeMillis() + 
				", width = " + this.getWidth() + 
				", height = " + this.getHeight());
		
	}
	
	public boolean setImageIndex(int index) {
		final int[] drawables = {
			R.drawable.simpson01,
			R.drawable.simpson02,
			R.drawable.simpson03,
			R.drawable.simpson04,
			R.drawable.simpson05,
		};
		if (index > drawables.length - 1)
			return false;
		
		//this.setImageResource(drawables[index]);
		
		if (mBitmap != null)
			mBitmap.recycle();
		mBitmap = BitmapFactory.decodeResource(getResources(), drawables[index]);
		if (mBitmap != null) {
			mBitmap = Bitmap.createScaledBitmap(mBitmap, this.getWidth(), this.getHeight(), true);
		}
		return true;
	}
	
	public void setChosenColor(int color) {
		mChosenColor = color;
		Log.v(MainActivity.LOGTAG, String.format("setChosenColor=%08x", mChosenColor));
	}
	
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		Log.v(MainActivity.LOGTAG, "MyImageView onMeasure called, " + System.currentTimeMillis());
	}
	
	@Override
	protected void onDraw(Canvas canvas) {
		//super.onDraw(canvas);
		Log.v(MainActivity.LOGTAG, "onDraw");
		if (mBitmap == null)
			this.setImageIndex(0);
		
		//this.setImageBitmap(mBitmap);
		canvas.drawBitmap(mBitmap, 0, 0, null);
		
//		if (mBitmap == null) {
//			mBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.simpson01);
//			if (mBitmap != null) {
//				mBitmap = Bitmap.createScaledBitmap(mBitmap, this.getWidth(), this.getHeight(), true);
//				if (mBitmap != null) {
//					mRepaint = true;
//				}
//			}
//		}
//		
//		if (mRepaint) {
//			if (mBitmap != null) {
//				this.setImageBitmap(mBitmap);
//				//canvas.drawBitmap(mBitmap, 0, 0, mPaint);
//				Log.v(MainActivity.LOGTAG, "MyImageView onDraw called, scaled bitmap width = " + mBitmap.getWidth()
//						+ ",height = " + mBitmap.getHeight());
//			}
//			mRepaint = false;
//		}
		
//		Bitmap srcBitmap = ((BitmapDrawable)this.getDrawable()).getBitmap();
//		//Bitmap srcBitmap = ((BitmapDrawable)this.getBackground()).getBitmap();
//		
//		Log.v(MainActivity.LOGTAG,
//				"MyImageView onDraw called, " + System.currentTimeMillis() 
//				+ ", width = " + this.getWidth() + ", height = " + this.getHeight()
//				+ ", bitmapWidth = " + srcBitmap.getWidth() + ", bitmapHeight = " + srcBitmap.getHeight());	
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {			
        if(event.getAction() == MotionEvent.ACTION_UP) {
            float x = event.getX();
            float y = event.getY();

            long startTime = System.currentTimeMillis();
            //Bitmap srcBitmap = ((BitmapDrawable)this.getDrawable()).getBitmap();
            //Bitmap dstBitmap = Bitmap.createBitmap(srcBitmap.getWidth(), srcBitmap.getHeight(), Config.ARGB_8888);
            // image process by native code 
            int curColor = mBitmap.getPixel((int)x, (int)y);
            
//            for (int i=0; i<mBitmap.getWidth(); i++) {
//            	Log.v(MainActivity.LOGTAG, String.format("[%d, %d] = %08x", i, (int)y, mBitmap.getPixel(i, (int)y)));
//            }
            
            Log.v(MainActivity.LOGTAG, String.format("x=%d, y=%d, curColor=%08x", (int)x, (int)y, curColor));
            ImageProc.imgproc(mBitmap, (int)x, (int)y, mChosenColor);
            this.invalidate();
            long costTime = System.currentTimeMillis() - startTime;
            
            Log.v(MainActivity.LOGTAG, "onTouch fill cost time = " + costTime);
        }
        return true;
    }
}