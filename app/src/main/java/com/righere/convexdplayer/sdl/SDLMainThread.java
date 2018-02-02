package com.righere.convexdplayer.sdl;

import android.util.Log;

/**
 Simple nativeInit() runnable
 */
public class SDLMainThread implements Runnable {
    @Override
    public void run() {

        String videoPath = SDLActivity.mSingleton.getPath();
        //videoPath = "/storage/emulated/0/zui.mp3";
        // Runs SDL_main()
        SDLActivity.nativeInit(videoPath);
        Log.v("SDL", "SDL thread terminated");
    }
}
