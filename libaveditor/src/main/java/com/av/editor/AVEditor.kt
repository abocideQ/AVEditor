package com.av.editor

class AVEditor(inUrl: String, outUrl: String) {

    companion object {
        init {
            System.loadLibrary("aveditor")
        }
    }

    init {
        native_ave_run(inUrl, outUrl)
    }

    private external fun native_ave_run(inUrl: String, outUrl: String)
}