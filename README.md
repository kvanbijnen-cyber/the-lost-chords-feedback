# The Lost Chords: Feedback! — Online Build

This project is set up so **GitHub compiles the Mega Drive ROM for you**.

The game is intentionally still a tiny 60 Hz prototype. The first goal is simply to get a real `.bin` running on your Sega Mega Drive / Genesis through the EverDrive.

## What you need

- A free GitHub account
- This folder uploaded to a GitHub repository
- No SGDK installation on your phone
- No local compiler

## Easiest route from Android

1. Go to GitHub and create a new repository, for example:
   `the-lost-chords-feedback`
2. Upload **the contents of this folder** to the repository.
   Important: `.github` is a hidden-style folder, but it must also be uploaded.
3. Open the repository on GitHub.
4. Tap **Actions**.
5. Choose **Build Mega Drive ROM**.
6. Tap **Run workflow**.
7. When the build has a green checkmark, open that completed workflow run.
8. Under **Artifacts**, download:
   `The_Lost_Chords_Feedback_ROM`
9. Unzip it. Inside is:
   `The_Lost_Chords_Feedback.bin`
10. Put that `.bin` on the SD card used by your Mega EverDrive and launch it.

## Automatic builds

Once this is in GitHub, changing anything under `src/` and committing it to `main`
will automatically start another ROM build.

## Output

SGDK normally produces:

    out/rom.bin

The workflow copies that to:

    The_Lost_Chords_Feedback.bin

and makes it downloadable as a GitHub Actions artifact.

## 60 Hz

The current game code deliberately checks for PAL/50 Hz and refuses to run the game in that mode.
It is meant for a console running at 60 Hz.

## If the build turns red

Open the failed Action run and copy the error log into ChatGPT. I can then update the project/code
from the actual compiler error instead of guessing.

## Technical note

The workflow uses a Linux SGDK Docker image containing the m68k cross-compiler.
Your Android device is therefore only controlling the cloud build; GitHub's Linux runner does the compilation.
