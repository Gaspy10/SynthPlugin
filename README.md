# Canya

Canya is a VST3 software synthesizer built with the JUCE framework.  
It combines sound synthesis with optional AI-based preset generation.

---

## Features

- Oscillator (sine, square, saw, triangle)
- ADSR envelope
- FIR filter (windowed sinc)
- LFO modulation (e.g. tremolo)
- Text-to-preset generation using OpenAI API
- Real-time safe architecture (separate audio and UI threads)

---

## Requirements

- JUCE
- C++ compiler (C++17 or newer)
- Projucer
- OpenAI API key (only if using AI preset generation)

---

## API Key Setup

To use AI preset generation, open `Secrets.h` and replace the placeholder with your own API key:

```cpp
inline const char* getOpenAIKey()
{
    return "YOUR_OPENAI_API_KEY_HERE";
}

## Build Instructions

1. Open the `.jucer` project file in Projucer.
2. If prompted, set the correct JUCE modules path.
3. Export the project for your target platform (Visual Studio, Xcode, or Makefile).
4. Build the project:
   - In Visual Studio or Xcode: build in **Release** configuration.
   - When using the generated Makefile: open a terminal in the project’s `Builds/LinuxMakefile` directory and run:

     ```bash
     make CONFIG=Release
     ```

5. After a successful build, locate the compiled `.vst3` file in the build output directory.
6. Copy the `.vst3` file to your system’s VST3 plugin folder and load it in your DAW.
