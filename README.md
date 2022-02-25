# cog-group-convo
Scripts involved in simulating a group conversation for people who are deaf or hard-of-hearing.

## Prerequisites
- Python 3.8 should be installed
- [Git LFS](https://git-lfs.github.com/) should be installed.
- Build the [FlatBuffer compiler](http://google.github.io/flatbuffers/flatbuffers_guide_building.html), which is used for efficient serialization/deserialization of messages transmitted over the network.

## Setting up
1. (Optional, but recommended) Make a new virtual environment and activate it:
    1. On bash/zsh: `python -m venv env`, then `source env/bin/activate`
    1. On Windows command prompt: `python -m venv env`, then `<venv>\Scripts\activate.bat`
    1. On Windows PowerShell: `python -m venv env`, then `<venv>\Scripts\Activate.ps1`
**Note**: If you use an environment, you'll need to activate it every time you want to run the application.
2. Install requirements: `pip install -r requirements.txt`
3. Go into the `cog_flatbuffer_definitions` folder, and run:
 ```
 flatc --python caption_message.fbs
 flatc --python orientation_message.fbs
 ```
4. Open a terminal, and from the root directory of this repository, run `python server.py --help` for information on how to run the server.


## Development Notes
1. For ease of development work, a simple "client" script has been included in this repository, which will consume the messages similar to how the Android application in [cog-ip-glasses](https://github.com/SaltyQuetzals/cog-ip-glasses) will.
