
---------------------------------
# Metadesk reflection system notes:

type_metadata.c and type_metadata.h contain the actual code for generating 
the output .c and .h files that contain our generated types

-----------------------------

## TODO:

- GENERATE multiple files after parsing - move toward all work being done *per-file* (parse, equip, write)
- #include in mdesk files
	- how we gonna do this...
	- need to get the global state into a data type and use one of those per file
	- question is: 
	- do we parse all files at once, THEN equip all types, THEN write to files? like in phases?
		- if we do this, would still need some seperation of what goes into what file for the writing phase...
		- if there's seperataion info like that might as well go all the way...?
	- or do we parse, equip, write for a single file?
		- make sure each file operates in it's own seperate arena for ez threading
		- easy to multithread/jobify
	- now that i wrote them down, it's very clear to me the second option is what I want to do. Yay!
	- mmm problem....
	- if we include a file with some types we want to use in our type file...
	- if we are parsing one file at a time and just plopping the include line in we end up not knowing what those types mean
	- Should we recursively parse included files? 
		- see an include - do a parse,equip,(optional:write) cycle
	- first option might actually be better....
- base default values(?) - default ctors? Or maybe all default ctors memset self to 0