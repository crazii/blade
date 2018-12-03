package com.blade.foundation;

import android.app.Activity;
import android.app.NativeActivity;

import android.app.Dialog;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.view.ContextThemeWrapper;
import android.os.Bundle;

public class BladeBase {

	static {
		System.loadLibrary("$(TARGET_SO_LIB)");	//note: library must be loaded to reflect native method nativeOnSystemDialogResult
	}

	////////////////////////////////////////////////////////////////////////////////////////////////	
	//native functions
	////////////////////////////////////////////////////////////////////////////////////////////////	
	public static native void nativeOnSystemDialogResult(int retCode);

	static class DialogRunnable implements Runnable {
		public String mTitle;
		public String mMessage;
		public String mYes;
		public String mNo;
		public String mCancel;
		public int mButtonCount;

		//////////////////////////////////////////////////////////////////////////
		///title, message, localized Yes No
		DialogRunnable(String tittle, String message, String locYes, String locNo, String locCancel, int buttons)
		{
			mTitle = tittle;
			mMessage = message;
			mYes = locYes;
			mNo = locNo;
			mCancel = locCancel;
			mButtonCount = buttons;
		}

		//////////////////////////////////////////////////////////////////////////
		public void run() {
			if( mButtonCount == 3 ) {
				Dialog dialog = new AlertDialog.Builder( BladeBase.getActivity() )
				.setTitle(mTitle)
				.setMessage(mMessage)
				.setPositiveButton( mYes, new DialogInterface.OnClickListener() {
													public void onClick(DialogInterface dialog, int whichButton) {
														BladeBase.nativeOnSystemDialogResult( 1 );
													}
												})
				.setNegativeButton( mNo, new DialogInterface.OnClickListener() {
													public void onClick(DialogInterface dialog, int whichButton) {
														BladeBase.nativeOnSystemDialogResult( 0 );
													}
												})
				.setNeutralButton( mCancel, new DialogInterface.OnClickListener() {
													public void onClick(DialogInterface dialog, int whichButton) {
														BladeBase.nativeOnSystemDialogResult( 2 );
													}
												})
				.setCancelable(false)
				.create();
				dialog.show();
			}
			else if( mButtonCount == 2 ) {
				Dialog dialog = new AlertDialog.Builder( BladeBase.getActivity() )
				.setTitle(mTitle)
				.setMessage(mMessage)
				.setPositiveButton( mYes, new DialogInterface.OnClickListener() {
													public void onClick(DialogInterface dialog, int whichButton) {
														BladeBase.nativeOnSystemDialogResult( 1 );
													}
												})
				.setNegativeButton( mNo, new DialogInterface.OnClickListener() {
													public void onClick(DialogInterface dialog, int whichButton) {
														BladeBase.nativeOnSystemDialogResult( 0 );
													}
												})
				.setCancelable(false)
				.create();
				dialog.show();
			}else {
				Dialog dialog = new AlertDialog.Builder( BladeBase.getActivity() )
				.setTitle(mTitle)
				.setMessage(mMessage)
				.setPositiveButton( mYes, new DialogInterface.OnClickListener() {
													public void onClick(DialogInterface dialog, int whichButton) {
														BladeBase.nativeOnSystemDialogResult( 1 );
													}
												})
				.setCancelable(false)
				.create();
				dialog.show();
			}
		}
	}

	private static Activity msActivity;

	public static void setActivity(Activity activity)
	{
		msActivity = activity;
	}
	public static Activity getActivity()
	{
		return msActivity;
	}
	
	//////////////////////////////////////////////////////////////////////////
	//Cooperate with native code. DO NOT call it at Java side
	//////////////////////////////////////////////////////////////////////////
	public static void showDialog(String title, String showText, String locYes, String locNo, String locCancel, int count) {
		DialogRunnable runnable = new DialogRunnable(title, showText, locYes, locNo, locCancel, count);
		msActivity.runOnUiThread( runnable );
	}
}
