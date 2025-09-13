WeightAlpha — Frequency-Dependent VST3 Plugin for Mixing & Mastering
![WeightAlpha]([https://github.com/WilliamAshley2019/WeightAlpha/WeightAlpha.png)]
Modern GUI screenshot placeholder — replace with actual image.

WeightAlpha is a VST3 audio plugin for mixing, mastering, and sound design, designed to add subtle or pronounced frequency-dependent
tonal enhancement. Inspired by Chris Johnson’s Airwindows Weight plugin (https://www.airwindows.com/?s=weight), WeightAlpha extends its functionality with a polished GUI,
new parameters, presets, and support for 32-bit & 64-bit double precision processing. The plugin is essentially just reworking Weight
adding a few new things and some knobs for the gui. Its still a little unpolished but functionining.

Built with JUCE 8.0.9 and the VST3 SDK, WeightAlpha should works in all major DAWs such as Reaper, Ableton Live, Logic Pro
(via AU build), Cubase, and FL Studio. However it has only been tested in FL Studio in Windows 11.

WeightAlpha applies a predictive filter that emphasizes or smooths specific frequency ranges, giving tracks more weight,
warmth, and presence. Whether you need to add bass depth, vocal warmth, or airy highs, this plugin provides flexible control 
with an intuitive interface.

Plugin formats: VST3
Framework: JUCE 8.0.9        Makeusing plugin basics with DSP module added.

Target Users: Producers, mixing engineers, mastering engineers, and sound designers - anyone who would like to try the plugin really
though.

 Use Cases

 Subtle Frequency Enhancement

Add warmth to bass/kick (80–120 Hz)

Smooth harsh vocals/guitars (500 Hz–2 kHz)

Add air to cymbals/synths (2–20 kHz)       (I still think it is only working for the lowend I havn't tested it much with the highend
this likely needs to be fixed)


 Mix Bus Processing

Apply gentle tonal shaping across entire mixes

 Creative Sound Design

Sweep the frequency knob for evolving pads, textures & ambient FX

 Corrective EQ

Tame harsh transients

Boost weak bands for better balance

 Features

 Frequency Control: 20 Hz – 20 kHz (full) or 20–120 Hz (narrow)

 Weight (Mix) Control: 0–100% wet/dry balance

 Strength Control: Adjust tonal intensity (subtle → aggressive)

 Frequency Range Toggle: Full spectrum vs. narrow low-end focus

 Presets:

Default (Balanced)

Bass Boost (80 Hz low-end emphasis)

Vocal Warmth (midrange shaping)

 Modern GUI with rotary knobs & dropdown menu

 Double Precision Processing (32/64-bit float)

 Bypass Toggle with clear visual feedback

 Installation
Pre-Built VST3

Copy WeightAlpha.vst3 to your DAW’s VST3 folder:
Windows: C:\Program Files\Common Files\VST3
Rescan plugins in your DAW.

No registration no bs just download and drop it in the vst3 folder scan and use.



C++17 compiler (made with Visual Studio Community 2022)

Insert WeightAlpha on a mixer track, bus, or master channel.

Adjust Freq, Weight, Strength parameters.

Toggle Range between narrow and full spectrum.

Save time with presets or automate parameters for evolving sound design.

 Contributing

Contributions welcome!
Fork repo → create branch → commit → PR.
Include description + test notes.

License

This project is licensed under GPLv3 and the MIT License. The Plugin elements that are JUCE or VST3 are GPLv3 while the portions that
are Airwindows weight derived at MIT license. All other sections are under GPLv3.
JUCE License:
This project uses the JUCE framework
UCE is subject to its own license terms

VST3 SDK License:
This project uses the Steinberg VST3 SDK
The SDK is subject to Steinberg’s licensing terms.

Acknowledgements
Chris Johnson (Airwindows) — original Weight concept https://www.airwindows.com/?s=weight - this is essentially a rework of Airwindows weight but adjusted to build in 
JUCE using juce's default project files PluginProcessor and PluginEditor along with some changes the idea is to expand the plugin
to rather than just work with the low end frequency work across the full range of the spectrum and add a little more ability to 
make the sound heavier.

JUCE Team — cross-platform audio framework

Steinberg — VST3 SDK

Contact
William Ashley Music producer, audio software developer, and digital creator.
GitHub: WilliamAshley2019 github.com/WilliamAshley2019
Profolio page: https://12264447666william.wixsite.com/williamashley
Social Media: https://www.hopp.bio/william-ashley

If you would like to support my efforts in music content such as plugins or audio feel free to support by spreading word of anything
you think is good to share. Also you can donate at https://buymeacoffee.com/williamashley

Also consider supporting Airwindows support at https://www.airwindows.com/support/
