#ifndef INCLUDE_nordlicht_h__
#define INCLUDE_nordlicht_h__

typedef struct nordlicht nordlicht;

// Allocate a new nordlicht of specific width and height, for a given video
// file. Use `nordlicht_free` to free it again. Returns NULL on errors.
nordlicht* nordlicht_init(char *filename, int width, int height);

// Free a nordlicht.
void nordlicht_free(nordlicht *n);

// Generate the nordlicht. Returns 0 on success.
int nordlicht_generate(nordlicht *n);

// Returns a value between 0 and 1 indicating how much of the nordlicht is done.
float nordlicht_progress(nordlicht *n);

// Write the nordlicht to a PNG file. Return 0 on success.
int nordlicht_write(nordlicht *n, char *filename);

#endif
