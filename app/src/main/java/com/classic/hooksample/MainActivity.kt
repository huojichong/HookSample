package com.classic.hooksample

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import com.bytedance.android.bytehook.sample.NativeHacker
import com.classic.hooksample.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private val TAG: String = "MainActivity"
    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = stringFromJNI()


        binding.btnHook.setOnClickListener {
            NativeHacker.hook()
        }

        binding.btnUnhook.setOnClickListener {
            NativeHacker.unhook()
        }

        binding.btnDump.setOnClickListener {
            Log.d(TAG, "onCreate: " + stringFromJNI())
        }
    }

    /**
     * A native method that is implemented by the 'hooksample' native library,
     * which is packaged with this application.
     */
    external fun stringFromJNI(): String

//    companion object {
//        // Used to load the 'hooksample' library on application startup.
//        init {
//
//        }
//    }
}