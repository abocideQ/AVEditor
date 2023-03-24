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
    }

    private fun goRepack() {
        //flv
        val inUrlFlv = AssetUtils.asset2cache(this, "play.flv")
        val outUrlFlv2Mp4 = File(File(inUrlFlv).parentFile, "play.mp4").absolutePath
        val outUrlFlv2Ts = File(File(inUrlFlv).parentFile, "play.ts").absolutePath
        AVEditor().repack(inUrlFlv, outUrlFlv2Mp4)
        AVEditor().repack(inUrlFlv, outUrlFlv2Ts)
        /*//mp4
        val inUrlMp4 = AssetUtils.asset2cache(this, "movie.mp4")
        val outUrlMp42Flv = File(File(inUrlFlv).parentFile, "movie_out.flv").absolutePath
        val outUrlMp42Ts = File(File(inUrlFlv).parentFile, "movie_out.ts").absolutePath
        AVEditor().repack(inUrlMp4, outUrlMp42Flv)
        AVEditor().repack(inUrlMp4, outUrlMp42Ts)
        //ts
        val inUrlTs = AssetUtils.asset2cache(this, "ts.ts")
        val outUrlTs2Flv = File(File(inUrlFlv).parentFile, "ts_out.flv").absolutePath
        val outUrlTs2Mp4 = File(File(inUrlFlv).parentFile, "ts_out.mp4").absolutePath
        AVEditor().repack(inUrlTs, outUrlTs2Flv)
        AVEditor().repack(inUrlTs, outUrlTs2Mp4)*/
    }
}