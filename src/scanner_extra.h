#ifndef SCANNER_EXTRA_H
#define SCANNER_EXTRA_H

typedef struct _ScannerExtraData {
    void* caller;
} ScannerExtraData;

#define YY_EXTRA_TYPE struct _ScannerExtraData *

#endif
