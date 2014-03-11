package com.myandroid.paintboard;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.util.AttributeSet;
import android.util.Log;
import android.widget.ImageView;

public class MyImageView extends ImageView {
	
	//private Bitmap mBitmap;
	private int first = 0;
	
	public MyImageView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}
	
	public MyImageView(Context context) {
		super(context);
	}
	
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		Log.v(MainActivity.LOGTAG + " MyImageView", "onMeasure called, " + System.currentTimeMillis());
	}
	
	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		Log.v(MainActivity.LOGTAG + " MyImageView",
				"onDraw called, " + System.currentTimeMillis() + 
				", width = " + this.getWidth() + 
				", height = " + this.getHeight());
		
		if (first == 0)
		{
			++first;
			this.setImageResource(R.drawable.simpson01);
		}
		
	}
}