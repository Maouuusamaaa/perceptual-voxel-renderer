package com.pvr;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.view.Gravity;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public final class MinecraftWorldActivity extends Activity {
    private static final int OPEN_WORLD = 42;
    static { System.loadLibrary("pvr_android"); }
    private TextView status;
    private static native int nativeImportWorld(String path);

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER);
        status = new TextView(this);
        status.setText("Perceptual Voxel Renderer\nPilih file .mcworld untuk import");
        status.setGravity(Gravity.CENTER);
        Button open = new Button(this);
        open.setText("Open .mcworld");
        open.setOnClickListener(v -> openWorld());
        root.addView(status); root.addView(open);
        setContentView(root);
    }

    private void openWorld() {
        Intent i = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        i.addCategory(Intent.CATEGORY_OPENABLE);
        i.setType("application/octet-stream");
        startActivityForResult(i, OPEN_WORLD);
    }

    @Override protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != OPEN_WORLD || resultCode != RESULT_OK || data == null) return;
        Uri uri = data.getData();
        if (uri == null) return;
        try {
            File copy = new File(getCacheDir(), "import.mcworld");
            try (InputStream in = getContentResolver().openInputStream(uri); FileOutputStream out = new FileOutputStream(copy)) {
                byte[] buffer = new byte[8192]; int n; while ((n = in.read(buffer)) != -1) out.write(buffer, 0, n);
            }
            int rc = nativeImportWorld(copy.getAbsolutePath());
            status.setText(rc == 0 ? "Import berhasil\nPVR World Truth siap" : "Import gagal\ncode=" + rc);
        } catch (Exception e) { status.setText("Gagal membaca world\n" + e.getMessage()); }
    }
}
