#ifndef DUMALLOC_H
#define DUMALLOC_H

#define Managed(p) (*p)
#define Managed_t(t) t*

#define FIRST_FIT 0
#define BEST_FIT 1

// The interface for DU malloc and free

void duMemoryDump();
void duManagedInitMalloc(int searchType);
void** duManagedMalloc(int size);
void duManagedFree(void** mptr);
void minorCollection();
void majorCollection();






#endif
