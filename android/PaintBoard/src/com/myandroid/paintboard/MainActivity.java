package com.myandroid.paintboard;

import java.util.Stack;

import android.app.Activity;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SparseIntArray;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

public class MainActivity extends Activity {

	public static final String LOGTAG = "PaintBoard";
	
	private final int colorPanleLine = 2;
	private final int colorPanleColumn = 8;
	
	private SparseIntArray colorMap;
	//private int mChosenColor = 0xffff0000;
		
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
        
		mImageView = (MyImageView)findViewById(R.id.imageview);
		//mImageView.setOnTouchListener(this);
		
		//mImageView.setImageResource(R.drawable.simpson01);
		//mImageView.setImageIndex(0);
		
		initColorPanle();
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
		
		colorMap = new SparseIntArray();
		
		
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
			//mChosenColor = color;
			mImageView.setChosenColor(color);
		}
	}
	
	
	public int getPixelColor(int x, int y) {
		return 0;//mImageView.getPixelColor(x, y);
	}
	
	public void setPixelColor(int x, int y, int color) {		
		//mImageView.setPixelColor(x, y, color);
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
	
	protected boolean isPixelValid(int w, int h, int x, int y, int old_color, int new_color, int boundary_color) {
		boolean bValid = false;

		if (x < 0 || y < 0 || x >= w || y >= h)
			return false;
		
	    int color = getPixelColor(x, y);
	    boolean bSimilarTarget = ColorComp(color, old_color);
	    boolean bSimilarBoundary = ColorComp(color, boundary_color);
	    if (color != new_color && bSimilarTarget && !bSimilarBoundary) {
	        bValid = true;
	    } else {
	        bValid = false;
	    }
	    return bValid;
	}
	
	protected boolean IsPixelValid(int x, int y, int old_color, int new_color, int boundary_color) {
		boolean bValid = false;

//		if (x < 0 || y < 0 || x >= mImageView.getBitmapWidth() || y >= mImageView.getBitmapHeight())
//			return false;
		
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
	
	protected final class Seed {
		private int y;
		private int xLeft;
		private int xRight;
		private int prevLeft;
		private int prevRight;
		private int direction;
		public Seed(int y, int xLeft, int xRight, int prevLeft, int prevRight, int direction) {
			this.y = y;
			this.xLeft = xLeft;
			this.xRight = xRight;
			this.prevLeft = prevLeft;
			this.prevRight = prevRight;
			this.direction = direction;
		}
	}
	
	protected void scanLineSeedFill(int w, int h, int x, int y, int new_color,
			int boundary_color) {

		final int DIR_UP = 1;
		int old_color = getPixelColor(x, y);

		int L, R;

		L = R = x;
		while (++R < w
				&& isPixelValid(w, h, R, y, old_color, new_color,
						boundary_color)) {
			setPixelColor(R, y, new_color);
		}

		while (--L > 0
				&& isPixelValid(w, h, L, y, old_color, new_color,
						boundary_color)) {
			setPixelColor(L, y, new_color);
		}

		--R;
		++L;

		Stack<Seed> stk = new Stack<Seed>();
		stk.push(new Seed(y, L, R, R + 1, R, DIR_UP));

		Seed seed;
		while (!stk.empty()) {
			seed = stk.pop();

			int YC = seed.y;
			L = seed.xLeft;
			R = seed.xRight;
			int PL = seed.prevLeft;
			int PR = seed.prevRight;
			int dir = seed.direction;
			int _8_connectivity = 1;
			int data[][] = {
					{ -dir, L - _8_connectivity, R + _8_connectivity },
					{ dir, L - _8_connectivity, PL - 1 },
					{ dir, PR + 1, R + _8_connectivity } };
			
			for (int k = 0; k < 3; k++) {
				dir = data[k][0];
				int left = data[k][1];
				int right = data[k][2];
				
				if (YC + dir >= h) 
					continue;
				
				for (int i = left; i <= right; i++) {
					if (i < w && 
							isPixelValid(w, h, i, YC+dir, old_color, new_color, boundary_color)) {
						int j = i;
						setPixelColor(i, YC+dir, new_color);
						
						while (--j >=0 && 
								isPixelValid(w, h, j, YC+dir, old_color, new_color, boundary_color)) {
							setPixelColor(j, YC+dir, new_color);
						}
						
						while (++i < w &&
								isPixelValid(w, h, i, YC+dir, old_color, new_color, boundary_color)) {
							setPixelColor(i, YC+dir, new_color);
						}
						
						stk.push(new Seed(YC+dir, j+1, i-1, L, R, -dir));
					}
				}
			}
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
	        int x_right = seed.x + count - 1;
	        
	        // 4. fill left-hand
	        count = FillLineLeft(seed.x-1, seed.y, old_color, new_color, boundary_color);
	        int x_left = seed.x - count;

	        // 5. scan adjacent line
	        if (seed.y > 0)
	        {
	            SearchLineNewSeed(stk, seed.y-1, x_left, x_right, old_color, new_color, boundary_color);
	        }
	        int imageHeight = 0;//mImageView.getBitmapHeight();
	        if (seed.y < imageHeight-1)
	        {
	            SearchLineNewSeed(stk, seed.y+1, x_left, x_right, old_color, new_color, boundary_color);
	        }
	    }
	}
	
	protected int FillLineRight(int x, int y, int old_color, int new_color, int boundary_color) {
	    int count = 0;
	    
	    int imageWidth = 0;//mImageView.getBitmapWidth();
	    while (x < imageWidth && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
	        setPixelColor(x, y, new_color);
	        x++;
	        count++;
	    }
	    return count;
	}
	
	protected int FillLineLeft(int x, int y, int old_color, int new_color, int boundary_color) {
	    int count = 0;
	    while (x >= 0 && IsPixelValid(x, y, old_color, new_color, boundary_color)) {
	    	setPixelColor(x, y, new_color);
	        x--;
	        count++;
	    }
	    return count;
	}
	
	protected int SkipInvalidInLine(int y, int x, int x_right, int old_color, int new_color, int boundary_color) {
	    int count = 0;

	    while (x <= x_right && !IsPixelValid(x, y, old_color, new_color, boundary_color)) {
	        x++;
	        count++;
	    }

	    return count;
	}
	
	protected void SearchLineNewSeed(Stack<Point> stk, int y, int x_left, int x_right,
			int old_color, int new_color, int boundary_color) {
		int xt = x_left;
		boolean findNewSeed = false;

		while (xt <= x_right) {
			findNewSeed = false;
			while (IsPixelValid(xt, y, old_color, new_color, boundary_color)
					&& (xt <= x_right)) {
				findNewSeed = true;
				xt++;
			}
			if (findNewSeed) {
				if (xt == x_right
						&& IsPixelValid(xt, y, old_color, new_color,
								boundary_color)) {
					stk.push(new Point(xt, y));
				} else {
					stk.push(new Point(xt-1, y));
				}
			}

			// skip invalid point in line
			int xspan = SkipInvalidInLine(y, xt, x_right, old_color, new_color,
					boundary_color);
			xt += (xspan == 0) ? 1 : xspan;
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
