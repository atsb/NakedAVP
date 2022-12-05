Aliens vs Predator Linux  http://icculus.org/avp/
------------------------

Table of Contents:
1. Introduction
2. Current Status
3. Compilation
4. Installation
5. Credits
a. Change History
b. Gold edition MD5s


Part 1: Introduction
--------------------

This is an unofficial Linux and Mac OS X port of the Fox Interactive /
Rebellion Developments game, Aliens Versus Predator Gold.

More information can be found at the project homepage and the Discourse
forum.

http://icculus.org/avp/
http://community.ioquake.org/c/avp

Please see LICENSE for copyright and licensing information.


Part 2: Current Status
----------------------

As of this release, i686 and x86_64 builds have been tested on Mac OS X
10.8, Ubuntu 14.04 LTS and Fedora 21.

Previously missing features are still missing.  Multiplayer, movies, etc.


Part 3: Compilation
-------------------

CMake, SDL 1.2, OpenAL and OpenGL support are required.  SDL 2.0 support is
also available but is experimental and incomplete at this time.

An example of how to use CMake to build the game:
$ tar xvzf <avp-source-code>.tar.gz
$ cd <avp-source-code>
$ mkdir build
$ cd build
$ cmake ..
$ make

If all goes well, an executable named "avp" will be built.


Part 4: Installation
--------------------

All of the AvP game data, files and directories, need to be lowercase. 
Either copy the avp executable to the game data directory or set the
AVP_DATA environment variable to the game data directory.

Local user settings are stored in ~/.avp.


Part 5: Credits
---------------

The original game is by Rebellion Developments.

Tim Beckmann, various code cleanup fixes
Chuck Mason, initial OpenAL implementation

For everything else, there's http://community.ioquake.org/c/avp

Appendix A: Change History
--------------------------

20150214:
OpenGL ES 1.1 compatibility
SDL2 support progress

20141225:
CMake build system
x86_64 support
Beginnings of SDL2 support

 
Appendix B: Gold edition MD5s
-----------------------------

A proper install of the Gold edition should look similar to this:
MD5 (avp_huds/alien.rif) = 4267d305ad3f18e6683f86b4eb755665
MD5 (avp_huds/alien_hud.rif) = ff6b892af4665c81082e24a43903e987
MD5 (avp_huds/disk.rif) = fe9a1f4e5caab35e38aabdbbb445aabc
MD5 (avp_huds/dropship.rif) = 4f8ad8e9a4c43e2216ad2bc0dbe29ea5
MD5 (avp_huds/hnpc_civvie.rif) = 5d06ea82a096558d70069c8a48830ec0
MD5 (avp_huds/hnpc_xenoborg.rif) = 92ab2760ff7abf2a4fdb691c8cfb8222
MD5 (avp_huds/hnpcalien.rif) = 1867c90c8f3101eaea42294b8da70f2d
MD5 (avp_huds/hnpchugger.rif) = b12d7b171ffb4c8699416543010663d0
MD5 (avp_huds/hnpcmarine.rif) = a220b5822b58b6c8f9a6d26a4774289e
MD5 (avp_huds/hnpcpred_alien.rif) = 85ae382e13c7f438d32bb971f21dedf0
MD5 (avp_huds/hnpcpredator.rif) = 31ab741eba3fbc9c72a801bc68901847
MD5 (avp_huds/hnpcpretorian.rif) = 4221affbcd0a132a87fc1ed3528f9abb
MD5 (avp_huds/marine.rif) = 906b7ad1522ee507f3ae635db174bd2e
MD5 (avp_huds/marwep.rif) = 709e70ece003374fba61640dcb73ed9f
MD5 (avp_huds/mdisk.rif) = bf5313faefed4f5ee5876bf811b3e16d
MD5 (avp_huds/multip.rif) = 0f47112c36bdcc7ce2d36d9f65ae264b
MD5 (avp_huds/pred ship fury.rif) = 514f1a7248db2315081c36e40a5cf041
MD5 (avp_huds/pred ship ob.rif) = 5c50c6a1b9b651c5b9bdf66fa1b2f955
MD5 (avp_huds/pred_hud.rif) = 2207090cd22a96afec84ed01ef72ea05
MD5 (avp_huds/predator.rif) = 735c0f33db055322ebf95363acc69119
MD5 (avp_huds/queen.rif) = 1800cd24226db14b36549b06175fb6e5
MD5 (avp_huds/sentry.rif) = 16d2efb5095fec300dd60698ff445fb2
MD5 (avp_huds/tongue.rif) = c09dbf413ea107bccce9b3b917a5d800
MD5 (avp_rifs/als-dm-coop.rif) = 8777d7f7bbfe7a87cc37e9b766be8bba
MD5 (avp_rifs/als-dm.rif) = 1a4fdffafb4678b8d0ca2537cc4ae689
MD5 (avp_rifs/area52.rif) = b52af8fa3ec154a83ca8fbe71fa9a59d
MD5 (avp_rifs/base.rif) = a054573e82728d0656eea4757f1077d3
MD5 (avp_rifs/battle.rif) = 107a3fb48ebdb000ae652f4a6d0a9c72
MD5 (avp_rifs/breakout.rif) = b94444bbc581d8aaf398ad07fdc425d3
MD5 (avp_rifs/breakout_p.rif) = 18d9dc2d979a99c410ddbd4ddf785b78
MD5 (avp_rifs/caverns.rif) = b761218c32e4f210aee42b7d40acde2c
MD5 (avp_rifs/caverns_a.rif) = 3bf4871492c958c1915615b292390181
MD5 (avp_rifs/co-op_meat_factory.rif) = fe6e7cb73750f18f0bfee097b229b6a4
MD5 (avp_rifs/colony_c.rif) = 2dc68e92a8cf2c47a47dd8ff827e5abd
MD5 (avp_rifs/compound.rif) = b954a582c4976cbab95859d31fd210f7
MD5 (avp_rifs/compoundcoop.rif) = 02c770a1e88533d7156fac9cbaa7f624
MD5 (avp_rifs/derelict.rif) = aa1a029da32eeffd3e30bb1291e6ad5d
MD5 (avp_rifs/derelict_a.rif) = 917cb15d9ffe404630544b0e41444316
MD5 (avp_rifs/e3demo.rif) = e102b1def6220aff18e57b0b9f6f6731
MD5 (avp_rifs/e3demosp.rif) = 6720212a41d0c7e51b7e3c03fbc32e84
MD5 (avp_rifs/elevator.rif) = ec7b75ca1730c8db5f239900c7c41fa3
MD5 (avp_rifs/elevator_co-op.rif) = 28403b2be75c24744aa3d0c3c3422fd7
MD5 (avp_rifs/escape.rif) = e0919deb6a55778f65ff9b2ef0068944
MD5 (avp_rifs/escape_p.rif) = 9e1fb4bcb05f2c640a95ba0ad494233b
MD5 (avp_rifs/fall.rif) = a3d7902cdf78253d742818153241248c
MD5 (avp_rifs/fall_m.rif) = d7af5d070fedb847ded66206c475546e
MD5 (avp_rifs/furyall.rif) = 09c0510003892184d760b278c1da9476
MD5 (avp_rifs/furyall_a.rif) = 4b9bb88e60a115136bcf66c4a56be2cb
MD5 (avp_rifs/genshd1.rif) = 38865339152458110bee6c0e87b5d509
MD5 (avp_rifs/hadleyshope.rif) = 773c69733b063cff7da53d53e8de443a
MD5 (avp_rifs/hadleyshope_coop.rif) = d95a1981e3ea6187160ae6c5ca6dffe8
MD5 (avp_rifs/hangar.rif) = 46f27e38c2f1e39f5b8af85d25f97d21
MD5 (avp_rifs/hive.rif) = aff965e71897a2efa2919096559a1f29
MD5 (avp_rifs/hive_c.rif) = 861a90530bb903bbd4daa1b25c484bed
MD5 (avp_rifs/invasion.rif) = 2355ae72ef68960d881a2df1f7cdfbb5
MD5 (avp_rifs/invasion_a.rif) = e57cd3ba63064229b2b11a2f83014089
MD5 (avp_rifs/invasion_p.rif) = 13372362090e5e52d40fae53bcb221a0
MD5 (avp_rifs/jockey.rif) = a2942f0c76ca13ff05e72e059f4e8511
MD5 (avp_rifs/jockeycoop.rif) = eb1064bdbfff5befba0148bd9136f263
MD5 (avp_rifs/kens-co-op.rif) = 55d8ff72d70c9d416509df7c5eca7ba6
MD5 (avp_rifs/lab14.rif) = 283b3b88adfa848b8d6bfbb21bd50013
MD5 (avp_rifs/lab14coop.rif) = 7a391b85893f457c833643ce65970742
MD5 (avp_rifs/leadworks.rif) = 2bb1a602ae00ff843f8cd671a3412b5b
MD5 (avp_rifs/leadworks_coop.rif) = 896bf46cd67a7c2a68f99ae5824c43ea
MD5 (avp_rifs/lockdown4.rif) = ae6ff47adeb2fc7969591bc2e278ce93
MD5 (avp_rifs/meat_factory.rif) = 90078c255d306961836b7bf3a6ce026e
MD5 (avp_rifs/nost03.rif) = b6c20145b51e9abc46939b1ea55aa986
MD5 (avp_rifs/nost03_m.rif) = 2cc94c22cb23cd02c708a4c076ec324a
MD5 (avp_rifs/nostromo.rif) = ee84b908593ebf05558b074a87611645
MD5 (avp_rifs/nostromo_coop.rif) = af556b4a5cf6c66695bdb07e6823d984
MD5 (avp_rifs/odobenus.rif) = 33fcb7fd8879c1a3af030830eb3540a3
MD5 (avp_rifs/office.rif) = 8d70b915a2e0c276481dfda1c265f9f0
MD5 (avp_rifs/stat101.rif) = fe44c96e2f3a031f3a4374da4df445a5
MD5 (avp_rifs/stat101_m.rif) = 205089ea581f03cc56d251ca31fc2df8
MD5 (avp_rifs/statue.rif) = ff47b9db187211920994f1dcd5ed5599
MD5 (avp_rifs/subway.rif) = 620ea0cc73f516ed8adecbdff6f789ed
MD5 (avp_rifs/subwaycoop.rif) = 63e4ef8b8f1a93b1c461b34350c8d217
MD5 (avp_rifs/sulaco.rif) = d401b6f78170161623893d97ff59b53a
MD5 (avp_rifs/sulaco_a.rif) = c4fd18fd7e27b734318c2a2e5e4475c3
MD5 (avp_rifs/sulaco_p.rif) = 4d794c520a0ffbc597514d1b9eaf4593
MD5 (avp_rifs/temple.rif) = 1302a03fb0dba9b45c76e55ea89683fe
MD5 (avp_rifs/temple_m.rif) = b7a024ffd6f2b50b93338ba84996093d
MD5 (avp_rifs/temple_p.rif) = 8e4fe32448e5dbb9b0b571d126b81fba
MD5 (avp_rifs/trapped.rif) = a5ceca52bd19098daa02fe8138ed29a4
MD5 (avp_rifs/vaults.rif) = ef7d4e3fd13fc5b4294e58033d582c65
MD5 (avp_rifs/vaults_m.rif) = ac4b4bdae7f6dd2e8e4a2be9e38a0092
MD5 (fastfile/aliensound.dat) = 663211754e9f742cc66dd7b67d99d9ea
MD5 (fastfile/common.ffl) = 6c3818f03a987b99e28713289eb84556
MD5 (fastfile/ffinfo.txt) = 8011119b8456329457f0872037b22243
MD5 (fastfile/marsound.dat) = aa69bb3181234fabb025ea50c036a311
MD5 (fastfile/predsound.dat) = 4fef41f3367e6b2325703f0d6bbbe578
MD5 (fastfile/queensound.dat) = 25dc700a67228db461cadb914efee05e
MD5 (fastfile/snd10.ffl) = 2142e48b243d71ed93c92f54d7b8a605
MD5 (fastfile/snd11.ffl) = 7d6efc94c1eac7cab969ab2cf4a8f0f8
MD5 (fastfile/snd12.ffl) = 993d75ed2eaa92db171437de90d0d021
MD5 (fastfile/snd13.ffl) = 27bd66d194e811d7a004e1143aff9cee
MD5 (fastfile/snd14.ffl) = 7c0694d9ac8d7fff5a71bf268f296610
MD5 (fastfile/snd15.ffl) = d4e717f36d3244b67fc44375ca747eed
MD5 (fastfile/snd16.ffl) = 9fecdd75362782f8cd695a9cbb716509
MD5 (fastfile/snd17.ffl) = 1e3c73765ab4bf012f7a0c8835536d68
MD5 (fastfile/snd18.ffl) = a36a92af0c981896496d2b1e2fa16d69
MD5 (fastfile/snd19.ffl) = 2cba77b90216cf238e8e67431c0c2509
MD5 (fastfile/snd2.ffl) = 6447697cc0bf026ceac07e58c8e6acb0
MD5 (fastfile/snd20.ffl) = 5712e0d0f8d6dca2ecf12e154c2f3668
MD5 (fastfile/snd21.ffl) = 8648a5360ef375ab812fb5f1aa98deaa
MD5 (fastfile/snd22.ffl) = 625d35cf9eabb987172ecd01385de5a8
MD5 (fastfile/snd23.ffl) = e17a0b0135e47a3f0284f01b46e54ba4
MD5 (fastfile/snd24.ffl) = f083983ecb863bdfd414c76a6dd0cc4c
MD5 (fastfile/snd25.ffl) = 9d342e0e05cf34d62ad19d92a336aa94
MD5 (fastfile/snd26.ffl) = b298641d60b39dfe1f20a331c8def4f4
MD5 (fastfile/snd27.ffl) = fa5521d0c54c256931a6dfa1007b226d
MD5 (fastfile/snd28.ffl) = 4175b3f72363d0e90a84c8f87b098160
MD5 (fastfile/snd29.ffl) = 4814847b1ce58b97c510712089a96088
MD5 (fastfile/snd3.ffl) = 88f6cb4ebca040a39e33671b60eaeb95
MD5 (fastfile/snd30.ffl) = 417e358abaf2600c8c893bb7c83e6bfa
MD5 (fastfile/snd31.ffl) = 53020a87d8e31c9c352411b6ef3d3327
MD5 (fastfile/snd32.ffl) = 4cb3a5e71ea6e75aa7dac3da93a47c0d
MD5 (fastfile/snd33.ffl) = 82bfe46770fc10f2bb766bc833de5d8a
MD5 (fastfile/snd34.ffl) = 2e0ceccebd393007c649f95972ae1217
MD5 (fastfile/snd35.ffl) = f94bac7b5f8d412683282608bbacf87f
MD5 (fastfile/snd36.ffl) = 60c6f42caba59a668b556fcd82c09110
MD5 (fastfile/snd37.ffl) = 5923b35cc4c85b4b00c683db2a967009
MD5 (fastfile/snd38.ffl) = 9707d7ec21860d27926b8ca1fea32511
MD5 (fastfile/snd39.ffl) = 6bab76db16787a6adb575f62482ec47a
MD5 (fastfile/snd4.ffl) = 97a113d56b355912b32e05c69e0d10bf
MD5 (fastfile/snd40.ffl) = 38234404a17fb33c44ad6f76e6bb4992
MD5 (fastfile/snd41.ffl) = f1126f5b21b17b2f708a1cf7cf774e32
MD5 (fastfile/snd42.ffl) = 69205ccd483d62668f6d0c9f15024bc0
MD5 (fastfile/snd43.ffl) = e35b56f2bd2172a6e17a1c251488f40e
MD5 (fastfile/snd44.ffl) = f76dfefe3a2b7ca675e71abcb8ee8771
MD5 (fastfile/snd45.ffl) = db700d990dcedb5a90dd33fedc443b6d
MD5 (fastfile/snd46.ffl) = c2f499f4b98c65c2c9307eb4d225503c
MD5 (fastfile/snd47.ffl) = a7d893d38d6b5532693124c3192ca199
MD5 (fastfile/snd48.ffl) = 30ee9a7447ea82744b35a32709e3433f
MD5 (fastfile/snd49.ffl) = 0580d2915e8ef3c4bef5f6c59de94fbb
MD5 (fastfile/snd5.ffl) = e0dea9b8de9e1303ebd137d6a792c4d0
MD5 (fastfile/snd50.ffl) = e13f9db8c08c8f055efc19945a078359
MD5 (fastfile/snd51.ffl) = 2d8b4e528d2d784c374ad5d17424708c
MD5 (fastfile/snd6.ffl) = 5eb1c874e57349b79255f07786d77519
MD5 (fastfile/snd7.ffl) = 7abbdcd4cb9ec878eeec279ffce8eaa0
MD5 (fastfile/snd8.ffl) = 1ea6f5460788946c6eb5e117f31bd085
MD5 (fastfile/snd9.ffl) = 7abc7189c778e2627a6d2288067be00b
MD5 (fastfile/tex1.ffl) = d9ecc8666917d167fc0f151287d3546a
MD5 (fastfile/tex10.ffl) = 519858b7d238a587a85652192ec2468d
MD5 (fastfile/tex11.ffl) = c49461369e40c098ecb706ce7ae59495
MD5 (fastfile/tex12.ffl) = 117311a6e1c96fa8e6834071ec10c2c6
MD5 (fastfile/tex13.ffl) = b8d2ce2970349d1e03c80a691f00b4c2
MD5 (fastfile/tex14.ffl) = c67da430a9d2949dabbfecc8efac5d33
MD5 (fastfile/tex15.ffl) = b4c10dfec3c73002ce2b0a38666fc171
MD5 (fastfile/tex16.ffl) = 0411c98c2db4c8bd9d67925b4924dbb3
MD5 (fastfile/tex17.ffl) = 2a1de9941e80dcdd6d252575189f26a8
MD5 (fastfile/tex18.ffl) = 15fbbc0ce657e597b960349930d2c2c8
MD5 (fastfile/tex19.ffl) = 4881d6cc9ee0adcd50b3392bab88523a
MD5 (fastfile/tex2.ffl) = 655c23f494879ea826b6dadcd2669d1d
MD5 (fastfile/tex20.ffl) = 7602a80deb49813c96e7e9c6804ba27d
MD5 (fastfile/tex21.ffl) = e4eb4c0f3269939a8d9c6cca413e95f0
MD5 (fastfile/tex22.ffl) = ae0caf7209f457312c5333338cc7021c
MD5 (fastfile/tex23.ffl) = 058930e377fa413fb5dcd2018d2ddee7
MD5 (fastfile/tex24.ffl) = a611dbc06035aefd1e38b6eaac80fab9
MD5 (fastfile/tex25.ffl) = 196b5eca90fcf921d18697ceec96565a
MD5 (fastfile/tex26.ffl) = b5bd750e210e7c82bc4c7010401df70a
MD5 (fastfile/tex27.ffl) = 85cb0b4a083be8f87ab04fa76dbc6478
MD5 (fastfile/tex28.ffl) = ec1949224ea3a2c104c9908d89c5f8f1
MD5 (fastfile/tex29.ffl) = 45abffc0107966677adfdd89cba9e18e
MD5 (fastfile/tex3.ffl) = feee0e06c91c88ca0f96aeaf1a724301
MD5 (fastfile/tex30.ffl) = 61d983086a4e5ad2e1d1685cb9b47aa2
MD5 (fastfile/tex31.ffl) = 80ecacc0282f6d917befd8328ee60272
MD5 (fastfile/tex32.ffl) = 21d2777d86e6a6ce0fb3f12c74cc978f
MD5 (fastfile/tex33.ffl) = 5490df39f625c7d2f2d837645bc91822
MD5 (fastfile/tex34.ffl) = c14eb4ce768a7cae4e474016320e09da
MD5 (fastfile/tex35.ffl) = 28074571a4fc9804e612244775d6aefc
MD5 (fastfile/tex36.ffl) = 788fadfef9603d1ba3ccb8c74f5ddaf9
MD5 (fastfile/tex37.ffl) = 53fa29efc83880c9e56f26ab727b1c6e
MD5 (fastfile/tex38.ffl) = 58c6103dd399bab9fd1554aa848916aa
MD5 (fastfile/tex39.ffl) = 91cb7a34c64620c74cd2c3dcb8f02bb6
MD5 (fastfile/tex4.ffl) = b0e9dbca444b38ddd5570ee94847071a
MD5 (fastfile/tex40.ffl) = 77a28d333b39b33f08d87a5963b17aa9
MD5 (fastfile/tex41.ffl) = 778644485c113c023a638ab70789983d
MD5 (fastfile/tex42.ffl) = 83b442ff98786f2cb033462dc2ef56f7
MD5 (fastfile/tex43.ffl) = 9fe624873f73f55a7af95ea6ecc804f6
MD5 (fastfile/tex44.ffl) = 188cca218e1dc4f43ebacbf7ee1cce47
MD5 (fastfile/tex45.ffl) = 8994489ef0c9fb825663ae603149c94b
MD5 (fastfile/tex46.ffl) = 7b8a28452f37f9630b47c1d13cfa48d5
MD5 (fastfile/tex47.ffl) = bd1a5f16278e8c204d653521726aee18
MD5 (fastfile/tex48.ffl) = 6456a8731e32824759a09860a0edf76a
MD5 (fastfile/tex49.ffl) = dbcb3bfeaaf85212b2ad27528f3697a2
MD5 (fastfile/tex5.ffl) = 77de24b3fd5d2904806d2adefe398d1e
MD5 (fastfile/tex50.ffl) = c396dd292505f01b264c398dd64bec16
MD5 (fastfile/tex51.ffl) = 655d729709ade2c7d10afeef349a441d
MD5 (fastfile/tex52.ffl) = 7a6644197c44767ef920ee880f5b9631
MD5 (fastfile/tex53.ffl) = 7db4a4a3643eea4ad03b5b7532a0da28
MD5 (fastfile/tex54.ffl) = 1cafa7df826f33138eb535c249681106
MD5 (fastfile/tex55.ffl) = ee6473804edb88692ac8d1e34782228d
MD5 (fastfile/tex56.ffl) = 6a5504d9dc663a96ea6fc81426f827b5
MD5 (fastfile/tex57.ffl) = a76b1a4f582e57933f9107f25fab43ed
MD5 (fastfile/tex58.ffl) = e9c4c5f233a71a39432765f084cd4c26
MD5 (fastfile/tex6.ffl) = ffad911262fe7e8e6744e288aa3e62d9
MD5 (fastfile/tex7.ffl) = a1ee192c4a44d5bdb6dc1fe290c9fa77
MD5 (fastfile/tex8.ffl) = 69c927deb5448b63a9c11186e8c29d3b
MD5 (fastfile/tex9.ffl) = 156b6dcdc3b92a9d6c0cfdfbc2421b10
MD5 (language.txt) = 10564fea944ef6680191ecc89a4616d5
