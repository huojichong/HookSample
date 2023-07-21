package com.classic.hooksample

import android.app.Application
import android.content.Context
import android.util.Log
import com.bytedance.android.bytehook.ByteHook
import com.bytedance.android.bytehook.ByteHook.ConfigBuilder

class GApplication : Application() {
    private val TAG = "bytehook_tag"
    override fun attachBaseContext(base: Context) {
        super.attachBaseContext(base)
        System.loadLibrary("hooksample")

        // init bytehook
        val r = ByteHook.init(
            ConfigBuilder()
                .setMode(ByteHook.Mode.AUTOMATIC) //                .setMode(ByteHook.Mode.MANUAL)
                .setDebug(true)
                .setRecordable(true)
                .build()
        )
        Log.i(TAG, "bytehook init, return: $r")

        // load hacker
        System.loadLibrary("hacker")

        // load hookee
        //System.loadLibrary("hookee"); // test for load-after-init
    }
}