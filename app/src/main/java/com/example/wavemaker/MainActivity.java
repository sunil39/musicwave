package com.example.wavemaker;

import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;
import android.view.MotionEvent;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    // These methods are defined in jni-bridge.cpp,
    // and need to be declared here in order to use them.
    private native void touchEvent(int action);

    private native void startEngine();

    private native void stopEngine();

    // Call the JNI bridge to start the audio engine.
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        startEngine();
    }

    // Override onTouchEvent to receive all touch events for our activity and pass them
    // directly to the JNI bridge to switch the tone on and off.
    @Override
    public boolean onTouchEvent(MotionEvent event) {
        touchEvent(event.getAction());
        return super.onTouchEvent(event);
    }

    // Call the JNI bridge to stop the audio engine.
    @Override
    public void onDestroy() {
        stopEngine();
        super.onDestroy();
    }
}