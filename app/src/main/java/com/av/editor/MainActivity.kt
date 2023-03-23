package com.av.editor

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import java.io.File

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        requestPermissions(
            arrayOf(
                android.Manifest.permission.READ_EXTERNAL_STORAGE,
                android.Manifest.permission.WRITE_EXTERNAL_STORAGE
            ), 100
        )
//        val inUrl = AssetUtils.asset2cache(this, "flv.flv")
//        val outUrl = File(File(inUrl).parentFile, "flv_out.flv").absolutePath
//        val inUrl = AssetUtils.asset2cache(this, "mp4.mp4")
//        val outUrl = File(File(inUrl).parentFile, "mp4_out.mp4").absolutePath
//        val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
//        val outUrl = File(File(inUrl).parentFile, "movie_out.mp4").absolutePath
        val inUrl = AssetUtils.asset2cache(this, "ts.ts")
        val outUrl = File(File(inUrl).parentFile, "ts_out.ts").absolutePath
        AVEditor(inUrl, outUrl)
    }
}