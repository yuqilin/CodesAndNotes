package com.myandroid.paintboard;

import android.graphics.Bitmap;

public class ImageProc {
	
	public native static boolean imgproc(Bitmap src, int x, int y, int color);
	
	static {
		System.loadLibrary("ImageProc");
	}
}