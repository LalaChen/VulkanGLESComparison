package com.sdengine.vulkanglescomparison

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.content.res.AssetManager
import android.util.Log
import android.view.MotionEvent
import android.view.Surface
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import com.sdengine.vulkanglescomparison.R
import android.view.View.OnTouchListener


class MainActivity : AppCompatActivity() {

    companion object {
        init {
            System.loadLibrary("vulkanglescomparison")
        }
    }

    val TAG : String = "GLESVulkanMainActivity"

    external fun InitializeApplication(assetMgr: AssetManager?)
    external fun ChangeSurface(surface: Surface?, format: Int, width: Int, height: Int)
    external fun Pause()
    external fun TerminateApplication()

    override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "onCreate")
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val sv = findViewById<View>(R.id.surfaceView) as SurfaceView
        //-------------- set on surface callback
        sv.holder.addCallback(object : SurfaceHolder.Callback {
            override fun surfaceCreated(holder: SurfaceHolder) {
                Log.i(TAG, "surfaceCreated")
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {
                Log.i(TAG, "surfaceChanged ")
                ChangeSurface(holder.surface, format, width, height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                Log.i(TAG, "surfaceDestroyed ")
            }
        })

        //-------------- set on touch listener
        sv.setOnTouchListener { v, event ->
            //Log.i(TAG, "Touch event !!! Event action is " + (event.getAction() & MotionEvent.ACTION_MASK) + " touch count = " + event.getPointerCount());
            //onMotionEventReceived(event)
            true
        }

        InitializeApplication(assets)
    }
}