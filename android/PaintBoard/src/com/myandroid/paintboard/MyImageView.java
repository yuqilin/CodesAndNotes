package com.myandroid.paintboard;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.drawable.BitmapDrawable;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.ImageView;

public class MyImageView extends ImageView {
	
	private Bitmap mBitmap = null;
	//private Paint mPaint;
	public boolean mRepaint = false;
	
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
	
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		Log.v(MainActivity.LOGTAG, "MyImageView onMeasure called, " + System.currentTimeMillis());
	}
	
	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		
		if (mBitmap == null) {
			mBitmap = BitmapFactory.decodeResource(getResources(), R.drawable.simpson01);
			if (mBitmap != null) {
				mBitmap = Bitmap.createScaledBitmap(mBitmap, this.getWidth(), this.getHeight(), true);
				if (mBitmap != null) {
					mRepaint = true;
				}
			}
		}
		
		if (mRepaint) {
			if (mBitmap != null) {
				this.setImageBitmap(mBitmap);
				//canvas.drawBitmap(mBitmap, 0, 0, mPaint);
				Log.v(MainActivity.LOGTAG, "MyImageView onDraw called, scaled bitmap width = " + mBitmap.getWidth()
						+ ",height = " + mBitmap.getHeight());
			}
			mRepaint = false;
		}
		
//		Bitmap srcBitmap = ((BitmapDrawable)this.getDrawable()).getBitmap();
//		//Bitmap srcBitmap = ((BitmapDrawable)this.getBackground()).getBitmap();
//		
//		Log.v(MainActivity.LOGTAG,
//				"MyImageView onDraw called, " + System.currentTimeMillis() 
//				+ ", width = " + this.getWidth() + ", height = " + this.getHeight()
//				+ ", bitmapWidth = " + srcBitmap.getWidth() + ", bitmapHeight = " + srcBitmap.getHeight());	
	}
	
	public void setPixelColor(int x, int y, int color) {
		if (mBitmap != null) {
			mBitmap.setPixel(x, y, color);
		}
	}
	
	public int getPixelColor(int x, int y) {
		if (mBitmap != null) {
			return mBitmap.getPixel(x, y);
		}
		return 0;
	}
	
	public int getBitmapWidth() {
		return mBitmap.getWidth();
	}
	
	public int getBitmapHeight() {
		return mBitmap.getHeight();
	}
}