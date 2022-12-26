# jubeat hook
A small hook to gather scores from the arcade rhythm game jubeat by reading the game memory, and send them on the internet !

# Features
* Export your jubeat scores to tachi leaderboards
* Multiple versions support : see [jubeat-hook.h](jubeat-hook/jubeat-hook.h). If your version is not supported, feel free to send me a message or even do a PR yourself
* Filter scores for a precise player (using card tag or ID)
* Enable debug mode to see every infos about your play

# How to use

* Download the latest [release](https://github.com/Meta-link/jubeat-hook/releases)
* Unzip everything in the game folder
* Edit jubeat-hook.ini to your liking (don't forget your API key if you want to export to tachi)
* Hook the dll using your favorite method (probably -k)
* Play the game and enjoy your scores everywhere !

# Planned features

* Local matching support (right now it will send wrong scores for each song)
* Export session scores into a single .json file
* More games support

# How to build

* git pull the repo
* Use vcpkg to install :
  - cpr:x86-windows
  - nlohmann-json:x86-windows
  - openssl:x86-windows
  - simpleini:x86-windows
  - tinyxml2:x86-windows
* Open the project with Visual Studio, pray a bit and build the solution

# Thanks

All of this was possible thanks to the incredible work of aixxe and their wonderful articles about hacking IIDX scores (starting [here](https://aixxe.net/2019/06/iidx-score-data)). I also took a lot of inspirations from  [ChunItachi](https://github.com/tomatosoupcan/ChunItachi). And of course, thanks to all the people and friends from various discord who helped me during the creating of this tool (CrazyRedMachine, zkldi, Amy, mowfax ...) !
