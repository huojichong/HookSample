package com.bytedance.android.bytehook.sample

object NativeHacker {
    fun hook() {
        nativeHook()
    }

    fun unhook() {
        nativeUnhook()
    }

    fun dumpRecords(pathname: String) {
        nativeDumpRecords(pathname)
    }

    private external fun nativeHook(): Int
    private external fun nativeUnhook(): Int
    private external fun nativeDumpRecords(pathname: String)
}