package org.scummvm.scummvm;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.View;

import java.io.File;

public class ScummVMNativeActivity extends NativeActivity {
    private static final int RQ_READ_EXT_STORAGE = 1;
    private static final int RQ_SELECT_DIRECTORY = 2;
    private static final String LOG_TAG = "ScummVMJava";
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        File[] files = this.getExternalFilesDirs(null);
    }

    @Override
    protected void onResume(){
        super.onResume();

        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
            | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
            | View.SYSTEM_UI_FLAG_LOW_PROFILE
            | View.SYSTEM_UI_FLAG_FULLSCREEN
            | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
            | View.SYSTEM_UI_FLAG_IMMERSIVE
            | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        );
    }

    protected void getDPI(float[] values) {
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);

        values[0] = metrics.xdpi;
        values[1] = metrics.ydpi;
    }

    public native void pushEvent(int type, int customType);

    public void showVirtualKeyboard(boolean enable) {
        InputMethodManager imm = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        if (enable) {
            imm.showSoftInput(this.getWindow().getDecorView(), InputMethodManager.SHOW_FORCED);
        } else {
            imm.hideSoftInputFromWindow(this.getWindow().getDecorView().getWindowToken(), 0);
        }
    }

    public String stringFromKeyCode(long downTime, long eventTime, int eventAction, int keyCode, int repeatCount, int metaState, int deviceId, int scanCode, int flags, int source){
        String strReturn;

        KeyEvent keyEvent = new KeyEvent(downTime, eventTime, eventAction, keyCode, repeatCount, metaState, deviceId, scanCode, flags, source);

        if (metaState == 0) {
            int unicodeChar = keyEvent.getUnicodeChar();
            if (eventAction == KeyEvent.ACTION_MULTIPLE && unicodeChar == keyEvent.KEYCODE_UNKNOWN) {
                strReturn = keyEvent.getCharacters();
            } else {
                strReturn = Character.toString((char)unicodeChar);
            }
        } else {
            strReturn = Character.toString((char)(keyEvent.getUnicodeChar(metaState)));
        }

        return strReturn;
    }

    public void selectDirectory() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        startActivityForResult(intent, RQ_SELECT_DIRECTORY);
    }
}
