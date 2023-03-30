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
        goRecode()
    }

    private fun goRecode() {
        Thread {
            val inUrlMp4 = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrlMp42Flv = File(File(inUrlMp4).parentFile, "movieo.mp4").absolutePath
            AVEditor().recode(inUrlMp4, outUrlMp42Flv)
        }.start()
    }

    private fun goRepack() {
        Thread {
            val inUrlMp4 = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrlMp42Flv = File(File(inUrlMp4).parentFile, "movie_o.flv").absolutePath
            AVEditor().repack(inUrlMp4, outUrlMp42Flv)
        }.start()
    }
}