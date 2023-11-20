package com.av.editor

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Point
import android.os.Bundle
import android.util.Log
import android.util.LruCache
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.LinearLayout
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import androidx.recyclerview.widget.RecyclerView.*
import java.io.File

class MainActivity : AppCompatActivity() {

    private val mBitCache: LruCache<Int, Bitmap> = LruCache(1024 * 1024 * 128)
    private val mList = arrayListOf<String>()
    private var mAdapter: Adapter<ViewHolder>? = null

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
            text = "timeline"
            setOnClickListener {
                goWriteTimeLineJpg()
            }
        })
        val mRecyclerView = RecyclerView(this)
        mRecyclerView.layoutManager = LinearLayoutManager(this, HORIZONTAL, false)
        mAdapter = object : Adapter<ViewHolder>() {
            @SuppressLint("ResourceType")
            override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
                val layout = FrameLayout(parent.context)
                layout.layoutParams = FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT,
                    ViewGroup.LayoutParams.WRAP_CONTENT
                )
                val imageView = ImageView(parent.context)
                imageView.id = 1000
                layout.addView(
                    imageView,
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
                )
                return object : ViewHolder(layout) {}
            }

            override fun getItemCount(): Int {
                return mList.size
            }

            @SuppressLint("ResourceType")
            override fun onBindViewHolder(holder: ViewHolder, position: Int) {
                try {
                    val imageView = holder.itemView.findViewById<ImageView>(1000)
                    if (null == mBitCache.get(holder.adapterPosition)) {
                        BitmapFactory.decodeFile(mList[holder.adapterPosition]).apply {
                            mBitCache.put(holder.adapterPosition, this)
                            imageView.setImageBitmap(this)
                        }
                    } else {
                        imageView.setImageBitmap(mBitCache.get(holder.adapterPosition))
                    }
                } catch (e: Exception) {
                    e.printStackTrace()
                }
            }
        }
        mRecyclerView.adapter = mAdapter
        vg.addView(
            mRecyclerView,
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
        )
    }

    private fun goRepack() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "h265_3.mkv")
            val outUrl = File(File(inUrl).parentFile, "h265_3_repack.ts").absolutePath
            AVEditor().repack(inUrl, outUrl)
        }.start()
    }

    private fun goRecode() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "h265_2.mp4")
            val outUrl = File(File(inUrl).parentFile, "h265_2_recode.mp4").absolutePath
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

    @SuppressLint("NotifyDataSetChanged")
    private fun goWriteTimeLineJpg() {
        Thread {
            val inUrl = AssetUtils.asset2cache(this, "movie.mp4")
            val folder = File(File(inUrl).parentFile, "TLCache")
            if (!folder.exists()) {
                folder.mkdirs()
            }
            AVEditor().writeTimeLine(inUrl, folder.absolutePath)
            runOnUiThread {
                for (file: File in folder.listFiles() ?: arrayOf()) {
                    mList.add(file.absolutePath)
                }
                mAdapter?.notifyDataSetChanged()
            }
        }.start()
    }
}