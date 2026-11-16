package org.northisland.client;

import org.libsdl.app.SDLActivity;

/**
 * The game's one screen: SDL's activity, which makes the window and runs the game's SDL_main.
 */
public class GameActivity extends SDLActivity {

    /**
     * The game and SDL are one library (SDL is linked into it), so there is no libSDL3.so to load first.
     */
    @Override
    protected String[] getLibraries() {
        return new String[] { "main" };
    }

    /**
     * The game's command line options, for test runs: the words of the start intent's "args" text
     * (adb shell am start -n org.northisland.client/.GameActivity --es args "--perf").
     */
    @Override
    protected String[] getArguments() {
        String args = getIntent() != null ? getIntent().getStringExtra("args") : null;
        if (args == null || args.trim().isEmpty()) {
            return new String[0];
        }
        return args.trim().split(" +");
    }
}
