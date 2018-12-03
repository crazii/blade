package com.blade.games;
import com.blade.foundation.*;

import java.io.File;
import java.io.IOException;
import java.util.EnumSet;
import java.util.Locale;
import java.nio.charset.Charset;


import android.content.res.Configuration;
import android.view.inputmethod.InputMethodManager;

import android.view.View;
import android.app.ActionBar;
import android.view.Window;
import android.view.KeyEvent;
import android.view.WindowManager;
import android.view.WindowManager.LayoutParams;

import android.app.Activity;
import android.app.NativeActivity;
import android.os.Bundle;

import android.util.Log;
import android.os.Handler;

public class BladeNativeActivity extends NativeActivity {

	static {

	}

	private final String TAG = "blade";
	private Handler mHandler;

	//////////////////////////////////////////////////////////////////////////
	public void hideNavigationBar() {

	    mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
				hideNavigationBarImmediately();
            }
        },1000);
	}

	//////////////////////////////////////////////////////////////////////////
	public void hideNavigationBarImmediately() {
		int uiFlags = View.SYSTEM_UI_FLAG_LAYOUT_STABLE
			| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
			| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
			| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION // hide nav bar
			| View.SYSTEM_UI_FLAG_FULLSCREEN; // hide status bar

		if( android.os.Build.VERSION.SDK_INT >= 19 ){
			uiFlags |= View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
		} else {
			uiFlags |= View.SYSTEM_UI_FLAG_LOW_PROFILE;
		}

		getWindow().getDecorView().setSystemUiVisibility(uiFlags);

		ActionBar actionBar = getActionBar();
		if( actionBar != null )
			actionBar.hide();
	}

	//////////////////////////////////////////////////////////////////////////
	@Override
	protected void onCreate (Bundle savedInstanceState) {

		mHandler = new Handler();

		hideNavigationBarImmediately();		//!important: called before super.onCreate
		super.onCreate(savedInstanceState);

		hideNavigationBar();

		BladeBase.setActivity(this);

		Log.d(TAG, Locale.getDefault().toString());
		//Log.d(TAG, getResources().getConfiguration().locale.toString());
		//Log.d(TAG, getResources().getConfiguration().locale.getLanguage());
		//Log.d(TAG, getResources().getConfiguration().locale.getDisplayName());
		//Log.d(TAG, Charset.defaultCharset().displayName(getResources().getConfiguration().locale));
	}

	//////////////////////////////////////////////////////////////////////////
    @Override
    public void onConfigurationChanged(Configuration newConfig) 
    {
        super.onConfigurationChanged(newConfig);
    }

}
