Anselization:
Collection:
  all until commit 43e4fce
Modulegroups and others:
  Commits:
    cd3f1d3
    dbabd51
    32e8052 (Like darktable 3.2!)
    cef8334
    d813577
    9934805
    1004f69
    19cf402
    6e58fa3
    19cf402
    c809678
    5ead9a4
    192af7a
    732342b
    ed956d2
    d5ec519
Flter:
    17888dc (main commit!)
    e288737
    fbccd14
    1d38ee8 (only filter.c)
    e855be8
    31d406c (only filter.c)
    bb7e96c
    7556d9f
Recent collection:
    1bdf797
    

We're proud to announce the new feature release of darktable, 4.0.1!

The github release is here: [https://github.com/darktable-org/darktable/releases/tag/release-4.0.1](https://github.com/darktable-org/darktable/releases/tag/release-4.0.1).

As always, please don't use the autogenerated tarball provided by
github, but only our tar.xz file. The checksums are:

```
$ sha256sum darktable-4.0.1.tar.xz
??? darktable-4.0.1.tar.xz
$ sha256sum darktable-4.0.1.dmg
??? darktable-4.0.1.dmg
$ sha256sum darktable-4.0.1.exe
??? darktable-4.0.1.exe
```

When updating from the stable 3.8.x series, please bear in
mind that your edits will be preserved during this process, but the new
library and configuration will no longer be usable with 3.8.x.

You are strongly advised to take a backup first.

#### Important note: to make sure that darktable can keep on supporting the raw file format for your camera, *please* read [this post](https://discuss.pixls.us/t/raw-samples-wanted/5420?u=lebedevri) on how/what raw samples you can contribute to ensure that we have the *full* raw sample set for your camera under CC0 license!

Since darktable 4.0.0:

- Almost 180 commits to darktable+rawspeed
- 103 pull requests handled
- 19 issues closed

## The Big Ones

N/A

## Other Changes

- Variables `$(MAKER)` and `$(MODEL)` can now be used when importing files.

- Display infinity for very large focus distance as supported by the
  exif standard.

## Bug Fixes

- Fix overexposed indicators after cropping.

- Fix DNG creation in the HDR merge module (ColorMatrix could be zero).

- Fix culling layout, which could be broken after a second use when
  changing the number of images (e.g. when rejecting an image).

- Fix GUI reset of the export module when using TIFF format.

- Fix copy of iop-order in merge mode when multiple instances are used in
  the destination images.

- Fix color filtering issue on Windows.

- Fix possible black screen when using auto-white balance on Windows.

- Properly retain the access and modified timestamps when using copy and
  import.

- Fix color picker reset in color zones module.

- Fix help link for module layout management.

- Fix crash after a failed camera tethering attempt.

- Add support for fast pipe mode on the diffuse module. Fast pipe mode
  is used to bypass long-running modules while interactively changing another
  module's parameters where that module does not require accurate image display.
  For example, it is used to improve responsiveness of the on-screen display in the
  crop and rotate/perspective modules.

- Fix some color picker button action types.

- Fix crash in Latex export.

- Fix website gallery export.

- Fix a file-handle leak.

- Fix a memory corruption in sharpen module that could lead to a
  crash.

- Fix vectorscope histogram display to avoid cropping the primary and
  secondary nodes (top and bottom).

- Add HEIF media type association for the desktop.

- Fix possible crash when using the color checker without a properly
  selected patch.

- Fix sorting of LUT 3D files and left-align the entries for
  better readability.

- Disable upscaling when export size is set to original image dimensions (0)
  as this does not have meaning and could crash darktable.

- Fix possible crash when applying a time-offset to images.

- Fix crop-size information displayed on screen (rounding issue).

- Fix blending detail mask memory requirement.

- Fix crash when zooming with fingers on an empty lighttable.

- Ensure that external format size limits are enforced during export.

- Some CSS fixes on color, contrast, positioning of combo-boxes,
  check-box size, scrollbars and progress bars.

- Fix JPEG APP1 header (skip 6-byte header).

- Fix some missing translations in notebook tabs.

- Use a color-managed background for color balance rgb module sliders.

- Light up midi modifier keys when shift/ctlr/alt are pressed.

- Some speed improvements (tuned compiler options, some optimized
  routines)

## Lua


## Notes


## Changed Dependencies

N/A

## RawSpeed changes


## Camera support, compared to 3.8

### Base Support

- CANON EOS R7
- CANON EOS R10

### White Balance Presets


### Noise Profiles

- NIKON D780
- NIKON D4s
- CANON EOS M50 Mark II
- CANON EOS 850D

### Custom Color Matrices

### Suspended Support

No samples on raw.pixls.us

- Canon EOS M2
- Creo/Leaf Aptus 22(LF3779)/Hasselblad H1
- Fujifilm FinePix HS50EXR
- Fujifilm FinePix S6000fd
- Fujifilm FinePix S9600fd
- Fujifilm IS-1
- GoPro FUSION
- Kodak EasyShare Z980
- Leaf Aptus-II 5(LI300059)/Mamiya 645 AFD
- Leaf Credo 60
- Leaf Credo 80
- Leica D-LUX 6
- Minolta DiMAGE 5
- Nikon 1 J4
- Nikon 1 S1
- Nikon Coolpix P7700
- Nikon D1H
- Nikon D2H
- Nikon D3S
- Olympus E-10
- Olympus SP320
- Olympus SP570UZ
- Panasonic DMC-FX150
- Panasonic DMC-G2
- Pentax K200D
- Pentax K2000
- Pentax Q10
- Phase One IQ250
- Samsung GX10
- Samsung GX20
- Samsung NX5
- Samsung NX10
- Samsung NX11
- Samsung NX20
- Samsung NX2000
- Samsung EK-GN120
- Samsung SM-G920F
- Samsung SM-G935F
- Sinar Hy6/ Sinarback eXact
- Sony DSLR-A380
- Sony DSLR-A560
- ST Micro STV680

## Translations

- Czech
- Brazilian Portuguese
- Chinese - China
- Chinese - Taiwan
- Dutch
- Esperanto
- European Spanish
- Finnish
- French
- German
- Hebrew
- Hungarian
- Italian
- Japanese
- Russian
- Slovenian
- Turkish
- Ukrainian
