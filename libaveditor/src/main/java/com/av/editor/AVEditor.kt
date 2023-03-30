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

    fun recode(inUrl: String, outUrl: String){
        native_ave_recode(inUrl, outUrl)
    }

    private external fun native_ave_repack(inUrl: String, outUrl: String)

    private external fun native_ave_recode(inUrl: String, outUrl: String)
}