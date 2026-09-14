package com.pvr;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public final class MainActivity extends Activity {
    static { System.loadLibrary("pvr_android"); }
    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        TextView view = new TextView(this);
        view.setText("Perceptual Voxel Renderer\nNative engine initialized");
        view.setTextSize(18f);
        view.setGravity(android.view.Gravity.CENTER);
        setContentView(view);
    }
}
