package com.emberengine.app;

import android.content.pm.ActivityInfo;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;

public class MainActivity extends SDLActivity {


    @Override
    protected String[] getLibraries() {

        return new String[]{
                "SDL3",
                "client"
        };
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

    }
}
