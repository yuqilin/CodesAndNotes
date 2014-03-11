package com.myandroid.paintboard;

import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import android.app.Activity;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.ImageView;

public class MainActivity extends Activity implements OnTouchListener{

	public static final String LOGTAG = "PrintBoard";
	
	private final int colorPanleLine = 2;
	private final int colorPanleColumn = 8;
	
	private Map<Integer, Integer> colorMap;
	private int mChosenColor = 0xffff0000;
		
	private MyImageView mImageView;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		int screenWidth = 0, screenHeight = 0;
		// get screen size
		DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);		
		
		float density = dm.density;
		int densityDpi = dm.densityDpi;
		float xdpi = dm.xdpi;
		float ydpi = dm.ydpi;
		screenWidth = dm.widthPixels;
		screenHeight = dm.heightPixels;
		int dpScreenWidth = (int)(dm.widthPixels / dm.density);
		int dpScreenHeight = (int)(dm.heightPixels / dm.density);
		
		Log.v(LOGTAG, "DisplayMetrics desity = " + density + ", densityDpi = " + densityDpi);
		Log.v(LOGTAG, "DisplayMetrics xdpi = " + xdpi + ", ydpi = " + ydpi);
		Log.v(LOGTAG, "DisplayMetrics screenWidth = " + screenWidth + ", screenHeight = " + screenHeight);
		Log.v(LOGTAG, "DisplayMetrics dpScreenWidth = " + dpScreenWidth + ", dpScreenHeight = " + dpScreenHeight);
        
		mImageView = (MyImageView)findViewById(R.id.imageview);//new ImageView(this);
		
//		int viewWidth = getPx(dpScreenWidth);
//		int viewHeight = getPx(dpScreenHeight * 9 / 11);
//		Log.e(LOGTAG + " DisplayMetrics", 
//				"viewWidth = " + viewWidth + ", viewHeight = " + viewHeight);
//		imgView.setLayoutParams(new LinearLayout.LayoutParams(viewWidth, viewHeight));
		//imgView.setImageResource(R.drawable.simpson01);
		//imgView.setBackgroundResource(R.drawable.simpson01);
		
		
//		String imageType = options.outMimeType;
//		
//		Log.v(LOGTAG + " onCreate", "imageViewHeight = " + imgView.getHeight() + " imageViewWidth = " + imgView.getWidth());
//		
		//Log.v(LOGTAG + " onCreate", "imageHeight = " + imageHeight + " imageWidth = " + imageWidth);
		
		
		
		//imgView.setImageBitmap(bitmap);
		mImageView.setOnTouchListener(this);
		
		initColorPanle();
		
//		final ImageView chosenColor = (ImageView)findViewById(R.id.color_chosen);
//		GradientDrawable bkground = (GradientDrawable)chosenColor.getBackground();
//		bkground.getPadding(padding)
		
		
		//final LinearLayout btnsRow1 = (LinearLayout)findViewById(R.id.btns_row01);
		//final LinearLayout btnsRow2 = (LinearLayout)findViewById(R.id.btns_row02);
		//int btnRowWidth = viewWidth;
		//int btnRowHeight = (screenHeight - viewHeight - 75) / 2;
		//Log.e(LOGTAG + " DisplayMetrics", "btnRowWidth = " + btnRowWidth + ", btnRowHeight = " + btnRowHeight);
        
		//final ImageView chosenColor = (ImageView)findViewById(R.id.color_chosen);
		//GradientDrawable shape = (GradientDrawable)chosenColor.getBackground();
		//shape.setColor(Color.BLUE);
		
		//btnsRow1.setLayoutParams(new LinearLayout.LayoutParams(btnRowWidth, btnRowHeight));
		//btnsRow2.setLayoutParams(new LinearLayout.LayoutParams(btnRowWidth, btnRowHeight));
		
		/*
		int w = View.MeasureSpec.makeMeasureSpec(0,View.MeasureSpec.UNSPECIFIED);  
		int h = View.MeasureSpec.makeMeasureSpec(0,View.MeasureSpec.UNSPECIFIED);  
		imgView.measure(w, h);
		viewWidth =imgView.getMeasuredWidth();  
		viewHeight =imgView.getMeasuredHeight();
		
		ViewTreeObserver observer = imgView.getViewTreeObserver();
		observer.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
			
			@SuppressWarnings("deprecation")
			@Override
			public void onGlobalLayout() {
				imgView.getViewTreeObserver().removeGlobalOnLayoutListener(this);
				viewWidth = imgView.getWidth();
				viewHeight = imgView.getHeight();
				Log.e(LOGTAG + " ViewTreeObserver", "viewWidth = " + viewWidth + ", viewHeight = " + viewHeight);
			}
		}); 
		Log.e(LOGTAG + " ImageView", "viewWidth = " + viewWidth + ", viewHeight = " + viewHeight);  
		
		//imgWidth = imgView.getLayoutParams().width = screenWidth;
		//imgHeight = imgView.getLayoutParams().height = (int)(screenHeight * 4 / 5);
		/*
		TableRow tblRow = (TableRow)findViewById(R.id.tblrow01);
		
		int tblRowWidth = tblRow.getLayoutParams().width;
		int tblRowHeight = tblRow.getLayoutParams().height;
		
		tblRowWidth = tblRow.getLayoutParams().width = screenWidth;
		tblRowHeight = tblRow.getLayoutParams().height = screenHeight - imgView.getLayoutParams().height;
		
		
		Button btn01 = (Button)findViewById(R.id.btn_color01);
		int btnWidth = btn01.getLayoutParams().width;
		int btnHeight = btn01.getLayoutParams().height;
		
		btnWidth = btn01.getLayoutParams().width = (int)(tblRow.getLayoutParams().width / 10);
		btnHeight = btn01.getLayoutParams().height = (int)(tblRow.getLayoutParams().height / 2);
		
		Log.e(LOGTAG + " ImageView", "width = " + imgWidth + ", height = " + imgHeight);
		Log.e(LOGTAG + " TableRow", "width = " + tblRowWidth + ", height = " + tblRowHeight);
		Log.e(LOGTAG + " Button01", "width = " + btnWidth + ", height = " + btnHeight);
		
		Log.v(LOGTAG + " MainActivity", "onCreate" + System.currentTimeMillis());
		//*/
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	public int getPx(int dimensionDp) {
	    float density = getResources().getDisplayMetrics().density;
	    return (int) (dimensionDp * density + 0.5f);
	}
	
	private void initColorPanle() {
		int i = 0, j = 0;
		
		colorMap = new HashMap<Integer, Integer>();
		
		
		for (i = 0; i < colorPanleLine; i++) {
			for(j = 0; j < colorPanleColumn; j++) {
				final String prefix = "btn_color";
				String btnIdName= prefix + i + j;
				String btnColorName = prefix + i + j;
				int btnId = this.getResources().getIdentifier(btnIdName, "id", this.getPackageName());
				int colorId = this.getResources().getIdentifier(btnColorName, "color", this.getPackageName());
				int color = this.getResources().getColor(colorId);
				
				colorMap.put(btnId, color);
				
				Button btn = (Button)findViewById(btnId);
				((GradientDrawable)btn.getBackground()).setColor(color);
			}
		}
	}
	
	public void onColorPanleClickEvent(View view) {
		Integer color = colorMap.get(view.getId());
		if (color == null) {
			Log.v(LOGTAG, "onClickEvent onColorPanleClickEvent, get view color null");
		} else {
			Log.v(LOGTAG, "onClickEvent onColorPanleClickEvent, button clicked, color = " + String.format("%x", color));
			final ImageView chosenColor = (ImageView)findViewById(R.id.color_chosen);
			((GradientDrawable)chosenColor.getBackground()).setColor(color);
			mChosenColor = color;
		}
	}
	
	@Override
	public boolean onTouch(View v, MotionEvent event) {
			
		///*
        if(event.getAction() == MotionEvent.ACTION_UP) {
            float screenX = event.getX();
            float screenY = event.getY();
            float viewX = screenX;// - v.getLeft();
            float viewY = screenY;// - v.getTop();
            
            
            
            long startTime = System.currentTimeMillis();
            ScanLineSeedFill((int)viewY, (int)viewX, mChosenColor, 0x0);
            long costTime = System.currentTimeMillis() - startTime;
            
            Log.v(LOGTAG, "onTouch fill cost time = " + costTime);
            
            //mImageView.invalidate();
            mImageView.mRepaint = true;
            mImageView.invalidate();
        }
        //*/
        return true;
    }
	
	public int getPixelColor(int x, int y) {
		return mImageView.getPixelColor(y, x);
	}
	
	public void setPixelColor(int x, int y, int color) {		
		mImageView.setPixelColor(y, x, color);
	}
	
	protected boolean ColorComp(int a, int b) {
		    int absR = Math.abs(Color.red(a) - Color.red(b));
		    int absG = Math.abs(Color.green(a) - Color.green(b));
		    int absB = Math.abs(Color.blue(a) - Color.blue(b));
		    //float distance = ((float)(absR * absR + absG * absG + absB * absB));
		    int distance = absR + absG + absB;
		    //if (distance - 10000.0f < 0.01)
		    if (distance < 50)
		        return true;
		    else
		        return false;
	}
	
	protected boolean IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color) {
		boolean bValid = false;

		if (x < 0 || y < 0 || x >= mImageView.getBitmapHeight() || y >= mImageView.getBitmapWidth())
			return false;
		
	    int color = getPixelColor(x, y);
	    boolean bSimilarTarget = ColorComp(color, old_color);
	    boolean bSimilarBoundary = ColorComp(color, boundary_color);
	    if (bSimilarTarget && color != new_color && !bSimilarBoundary) {
	        bValid = true;
	    }
	    else
	    {
	        bValid = false;
	    }
	    return bValid;
	}
	
	protected final class Point {
		private int x;
		private int y;
		public Point(int x, int y) {
			this.x = x;
			this.y = y;
		}
	}
	
	protected void ScanLineSeedFill(int x, int y, int new_color, int boundary_color) {
		
		int old_color = getPixelColor(x, y);
		
	    Stack<Point> stk = new Stack<Point>();

	    // 1. push seed into stack
	    stk.push(new Point(x, y));
	    
	    Point seed;
	    while (!stk.empty()) {
	        // 2. current seed
	        seed = stk.peek();
	        stk.pop();

	        // 3. fill right-hand
	        int count = FillLineRight(seed.x, seed.y, old_color, new_color, boundary_color);
	        
	        int y_right = seed.y + count - 1;
	        // 4. fill left-hand
	        count = FillLineLeft(seed.x, seed.y-1, old_color, new_color, boundary_color);
	        int y_left = seed.y - count;

	        // 5. scan adjacent line
	        if (seed.x > 0)
	        {
	            SearchLineNewSeed(stk, y_left, y_right, seed.x-1, old_color, new_color, boundary_color);
	        }
	        int imageHeight = mImageView.getBitmapHeight();
	        if (seed.x < imageHeight-1)
	        {
	            SearchLineNewSeed(stk, y_left, y_right, seed.x+1, old_color, new_color, boundary_color);
	        }
	    }
	}
	
	protected int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color) {
	    int count = 0;
	    
	    int imageWidth = mImageView.getBitmapWidth();
	    while (y < imageWidth && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
	        setPixelColor(x, y, new_color);
	        y++;
	        count++;
	    }
	    return count;
	}
	
	protected int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color) {
	    int count = 0;
	    while (y >= 0 && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
	    	setPixelColor(x, y, new_color);
	        y--;
	        count++;
	    }
	    return count;
	}
	
	protected int SkipInvalidInLine(int x, int y, int y_right, int old_color, int new_color, int boundary_color) {
	    int count = 0;

	    while (y <= y_right && !IsPixelValid(x, y, old_color, new_color, boundary_color)) {
	        y++;
	        count++;
	    }

	    return count;
	}
	
	protected void SearchLineNewSeed(Stack<Point> stk, int y_left, int y_right, int x,
			int old_color, int new_color, int boundary_color) {
		int yt = y_left;
		boolean findNewSeed = false;

		while (yt <= y_right) {
			findNewSeed = false;
			while (IsPixelValid(x, yt, old_color, new_color, boundary_color)
					&& (yt <= y_right)) {
				findNewSeed = true;
				yt++;
			}
			if (findNewSeed) {
				if (yt == y_right
						&& IsPixelValid(x, yt, old_color, new_color,
								boundary_color)) {
					stk.push(new Point(x, yt));
				} else {
					stk.push(new Point(x, yt - 1));
				}
			}

			// skip invalid point in line
			int yspan = SkipInvalidInLine(x, yt, y_right, old_color, new_color,
					boundary_color);
			yt += (yspan == 0) ? 1 : yspan;
		}
	}

	public static int calculateInSampleSize(BitmapFactory.Options options,
			int reqWidth, int reqHeight) {
		// Raw height and width of image
		final int height = options.outHeight;
		final int width = options.outWidth;
		int inSampleSize = 1;

		if (height > reqHeight || width > reqWidth) {

			final int halfHeight = height / 2;
			final int halfWidth = width / 2;

			// Calculate the largest inSampleSize value that is a power of 2 and
			// keeps both
			// height and width larger than the requested height and width.
			while ((halfHeight / inSampleSize) > reqHeight
					&& (halfWidth / inSampleSize) > reqWidth) {
				inSampleSize *= 2;
			}
		}

		return inSampleSize;
	}
	
	public static Bitmap decodeSampledBitmapFromResource(Resources res, int resId,
	        int reqWidth, int reqHeight) {

	    // First decode with inJustDecodeBounds=true to check dimensions
	    final BitmapFactory.Options options = new BitmapFactory.Options();
	    options.inJustDecodeBounds = true;
	    BitmapFactory.decodeResource(res, resId, options);

	    // Calculate inSampleSize
	    options.inSampleSize = calculateInSampleSize(options, reqWidth, reqHeight);

	    // Decode bitmap with inSampleSize set
	    options.inJustDecodeBounds = false;
	    return BitmapFactory.decodeResource(res, resId, options);
	}

}
