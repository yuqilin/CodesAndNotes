package com.myandroid.paintboard;

import android.app.Activity;
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

	private MyImageView mImageView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		int screenWidth = 0, screenHeight = 0;

		/*
		 * 1. get screen size
		 */
		DisplayMetrics dm = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);

		float density = dm.density;
		int densityDpi = dm.densityDpi;
		float xdpi = dm.xdpi;
		float ydpi = dm.ydpi;
		screenWidth = dm.widthPixels;
		screenHeight = dm.heightPixels;
		int dpScreenWidth = (int) (dm.widthPixels / dm.density);
		int dpScreenHeight = (int) (dm.heightPixels / dm.density);

		Log.v(LOGTAG, "DisplayMetrics desity = " + density + ", densityDpi = "
				+ densityDpi);
		Log.v(LOGTAG, "DisplayMetrics xdpi = " + xdpi + ", ydpi = " + ydpi);
		Log.v(LOGTAG, "DisplayMetrics screenWidth = " + screenWidth
				+ ", screenHeight = " + screenHeight);
		Log.v(LOGTAG, "DisplayMetrics dpScreenWidth = " + dpScreenWidth
				+ ", dpScreenHeight = " + dpScreenHeight);

		mImageView = (MyImageView) findViewById(R.id.imageview);

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
			for (j = 0; j < colorPanleColumn; j++) {
				final String prefix = "btn_color";
				String btnIdName = prefix + i + j;
				String btnColorName = prefix + i + j;
				int btnId = this.getResources().getIdentifier(btnIdName, "id",
						this.getPackageName());
				int colorId = this.getResources().getIdentifier(btnColorName,
						"color", this.getPackageName());
				int color = this.getResources().getColor(colorId);

				colorMap.put(btnId, color);

				/*
				 * 2. get button color && GradientDrawable
				 */
				Button btn = (Button) findViewById(btnId);
				((GradientDrawable) btn.getBackground()).setColor(color);
			}
		}
	}

	public void onColorPanleClickEvent(View view) {
		Integer color = colorMap.get(view.getId());
		if (color == null) {
			Log.v(LOGTAG,
					"onClickEvent onColorPanleClickEvent, get view color null");
		} else {
			Log.v(LOGTAG,
					"onClickEvent onColorPanleClickEvent, button clicked, color = "
							+ String.format("%x", color));
			final ImageView chosenColor = (ImageView) findViewById(R.id.color_chosen);
			
			/*
			 * ImageView set background color && GradientDrawable
			 */
			((GradientDrawable) chosenColor.getBackground()).setColor(color);
			
			mImageView.setChosenColor(color);
		}
	}

}
