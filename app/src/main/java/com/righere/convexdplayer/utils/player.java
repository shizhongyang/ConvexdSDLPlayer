package com.righere.convexdplayer.utils;

/**
 * Created by TT on 2018-01-30.
 */

public class player {
    public static String[] getLibraries() {
        return new String[] {
                "SDL2",
                // "SDL2_image",
                // "SDL2_mixer",
                // "SDL2_net",
                // "SDL2_ttf",
                "SDL2main"
        };
    }

    // Load the .so
    public static  void loadLibraries() {
        for (String lib : getLibraries()) {
            System.loadLibrary(lib);
        }
    }

    static {
        loadLibraries();
    }
    public static native void playaduio();

}
