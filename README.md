# frimco (fraret's image compressor)

This is the code for frimco, an loseless image compressor. frimco was created as a research tool for a high school project that tried to answer wheter the order in which the pixels are treated when compressing and image loselessly affects its compressed size. 

Right now it only supports grayscale images in pgm format. ffmpeg can convert images from/to this format. 

There are no plans to expand the program beyond its current state. Due to its purpose as a custom research tool it was deliberately written to be functional, easy and quick to code instead of easy to expand later on. I have thought multiple times what would make this program a little bit more useful and the conclusion has always been that it more options. The way it is coded now the end user has the option to toggle some algorithms and settings but a real research project would have to be able to change the design decisions that have been put in the code. 

There is no documentation as the only intended user was myself. For any doubts contact me somehow (preferrably via the email adress liked to my github account).
