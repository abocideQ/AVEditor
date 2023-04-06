package com.av.editor

import androidx.annotation.NonNull
import org.jetbrains.annotations.NotNull

class AVEditor {

    companion object {
        init {
            System.loadLibrary("aveditor")
        }
    }

    fun repack(inUrl: String, outUrl: String) {
        native_ave_repack(inUrl, outUrl)
    }

    fun recode(inUrl: String, outUrl: String) {
        native_ave_recode(inUrl, outUrl)
    }

    fun filter(
        inUrl: String,
        outUrl: String,
        filterVideo: String,
        filterAudio: String
    ) {
        native_ave_filter(inUrl, outUrl, filterVideo, filterAudio)
    }

    private external fun native_ave_repack(inUrl: String, outUrl: String)

    private external fun native_ave_recode(inUrl: String, outUrl: String)

    private external fun native_ave_filter(
        inUrl: String,
        outUrl: String,
        filterVideo: String,
        filterAudio: String
    )
}