package com.av.editor

import android.annotation.SuppressLint
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.Button
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.Toast
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.RecyclerView.ViewHolder
import java.io.File

class MainActivity : AppCompatActivity() {

    private val mList = arrayListOf<String>();

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
        vg.addView(Button(this).apply {
            text = "merge"
            setOnClickListener { goMerge() }
        })
        vg.addView(Button(this).apply {
            text = "writeTimeLine"
            setOnClickListener { goWriteTimeLineJpg() }
        })
        val mRecyclerView = RecyclerView(this)
        mRecyclerView.layoutManager = LinearLayoutManager(this)
        mRecyclerView.adapter = object : RecyclerView.Adapter<ViewHolder>() {
            @SuppressLint("ResourceType")
            override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
                val imageView = ImageView(parent.context)
                imageView.id = 1000
                return object : ViewHolder(ImageView(parent.context)) {}
            }

            override fun getItemCount(): Int {
                return mList.size
            }

            @SuppressLint("ResourceType")
            override fun onBindViewHolder(holder: ViewHolder, position: Int) {
                val imageView = holder.itemView.findViewById<ImageView>(1000)
            }
        }
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
            val maskUrl = AssetUtils.asset2cache(this, "mask.png")
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrl = File(File(inUrl).parentFile, "movie_filter.mp4").absolutePath
//            AVEditor().filter(inUrl, outUrl, "scale=iw/2:ih/2", "")
//            AVEditor().filter(inUrl, outUrl, "crop=iw/2:ih/2:0:0", "")
            AVEditor().filter(
                inUrl,
                outUrl,
                ("movie=$maskUrl[mask];" +
                        "[mask]scale=iw/10:iw/10[mask_scaled];" +
                        "[in][mask_scaled]overlay=0:0[out]"),
                ""
            )
        }.start()
    }

    private fun goMerge() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val outUrl1 = File(File(inUrl).parentFile, "movie_recode1.mp4").absolutePath
            val outUrl2 = File(File(inUrl).parentFile, "movie_recode2.mp4").absolutePath
            AVEditor().recode(inUrl, outUrl1)
            AVEditor().recode(inUrl, outUrl2)
            val in_merge_urls: Array<Any> = arrayOf(outUrl1, outUrl2)
            val out_merge_url = File(File(inUrl).parentFile, "merge.mp4").absolutePath
            AVEditor().merge(in_merge_urls, out_merge_url)
        }.start()
    }

    private fun goWriteTimeLineJpg() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val folder = File(File(inUrl).parentFile, "TLCache")
            if (!folder.exists()) {
                folder.mkdirs()
            }
            AVEditor().writeTimeLine(inUrl, folder.absolutePath)
        }.start()
    }
}