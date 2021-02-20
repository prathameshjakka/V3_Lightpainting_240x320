#include <Update.h>
#include <FS.h>
#include <SD.h>
#include <TFT_eSPI.h>

void performUpdate(Stream &updateSource, size_t updateSize);
void updateFromFS(fs::FS &fs);
void rebootEspWithReason(String reason);