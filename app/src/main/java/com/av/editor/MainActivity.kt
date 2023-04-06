package com.av.editor

import android.annotation.SuppressLint
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import android.widget.Toast
import java.io.File

class MainActivity : AppCompatActivity() {
    @SuppressLint("SetTextI18n")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        requestPermissions(
            arrayOf(
                android.Manifest.permission.READ_EXTERNAL_STORAGE,
                android.Manifest.permission.WRITE_EXTERNAL_STORAGE
            ), 100
        )
        val vg: LinearLayout = findViewById(R.id.ll_container)
        vg.addView(Button(this).apply {
            text = "repack"
            setOnClickListener { goRepack() }
        })
        vg.addView(Button(this).apply {
            text = "recode"
            setOnClickListener { goRecode() }
        })
        vg.addView(Button(this).apply {
            text = "filter"
            setOnClickListener { goFilter() }
        })
    }

    private fun goRepack() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrl = File(File(inUrl).parentFile, "movie_repack.flv").absolutePath
            AVEditor().repack(inUrl, outUrl)
        }.start()
    }

    private fun goRecode() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrl = File(File(inUrl).parentFile, "movie_recode.mp4").absolutePath
            AVEditor().recode(inUrl, outUrl)
        }.start()
    }

    private fun goFilter() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrl = File(File(inUrl).parentFile, "movie_filter.mp4").absolutePath
            AVEditor().filter(inUrl, outUrl, "scale=iw*2:ih*2", "")
        }.start()
    }
}