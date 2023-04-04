package com.av.editor

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

    fun filter(inUrl: String, outUrl: String, filterDesc: String) {
        native_ave_filter(inUrl, outUrl, filterDesc)
    }

    private external fun native_ave_repack(inUrl: String, outUrl: String)

    private external fun native_ave_recode(inUrl: String, outUrl: String)

    private external fun native_ave_filter(inUrl: String, outUrl: String, filterDesc: String)
}