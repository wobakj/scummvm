package org.scummvm.scummvm;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.Manifest;
import android.net.Uri;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Toast;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ScummVMNativeActivity extends NativeActivity {
    private static final int RQ_READ_EXT_STORAGE = 1;

    private static final String LOG_TAG = "ScummVMJava";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Permissions need to requested explicitly from version 23
        // On version 29 we can't access external storage outside of app directory and accessing app directory that doesn't require the permission
        if (Build.VERSION.SDK_INT >= 23 && Build.VERSION.SDK_INT < 29) {
            if(checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, RQ_READ_EXT_STORAGE);
            }
        }
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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        switch (requestCode) {
            case RQ_READ_EXT_STORAGE:
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                    && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted
                    Log.i(LOG_TAG, "Read External Storage permission was granted at Runtime");
                } else {
                    // permission denied! We won't be able to make use of functionality depending on this permission.
                    Toast.makeText(this, "Until permission is granted, some storage locations may be inaccessible!", Toast.LENGTH_SHORT)
                        .show();
                }
                break;
            default:
                break;
        }
    }

    public native void pushEvent(int type, int customType);

    protected void getDPI(float[] values) {
        DisplayMetrics metrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(metrics);

        values[0] = metrics.xdpi;
        values[1] = metrics.ydpi;
    }

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

    public String[] getAllStorageLocations() {
        List<String> locations = new ArrayList<>();

        if (Build.VERSION.SDK_INT < 19) {
            File externalFileDir = getExternalFilesDir(null);
            locations.add(externalFileDir.getAbsolutePath());
        } else {
            File[] externalFileDirs = getExternalFilesDirs(null);
            for (File f : externalFileDirs) {
                locations.add(f.getAbsolutePath());
            }
        }

        if (Build.VERSION.SDK_INT < 29) {
            File externalStorageDir = Environment.getExternalStorageDirectory();
            locations.add(externalStorageDir.getAbsolutePath());
        }

        return locations.toArray(new String[0]);

        // if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M && checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED
        // ) {
        //     requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, MY_PERMISSIONS_REQUEST_READ_EXT_STORAGE);
        // } else {
        //     return _externalStorage.getAllStorageLocations().toArray(new String[0]);
        // }
        // return new String[0]; // an array of zero length


    }

}
