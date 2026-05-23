#pragma region Memory management class

enum ptr_type
{
 ptMALLOC = 0,     // C-style malloc/free
 ptNEW = 1,        // C++ new/delete
 ptNEW_ARRAY = 2,  // C++ new[]/delete[]
 ptBMP = 3         // Special: bmpdraw* objects
};

struct mem_entry
{
 void *ptr;
 ptr_type type;
};

class MemList
{
 mem_entry *list;
 int capacity;
 int count;

 public:
 MemList (int initial = 256) : capacity (initial), count (0)
 {
  list = (mem_entry *)malloc (capacity * sizeof (mem_entry));
  if (list) memset (list, 0, capacity * sizeof (mem_entry));
 }

 ~MemList () { free (list); }

 void init_mem_list ()
 {
  if (list) memset (list, 0, capacity * sizeof (mem_entry));
  count = 0;
 }

 int search_mem (void *mem)
 {
  for (int i = 0; i < count; i++)
   if (list[i].ptr == mem) return i;
  return -1;
 }

 void *register_mem (void *mem, ptr_type type = ptMALLOC)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1) return mem; // already registered

  // fill holes first
  for (int i = 0; i < count; i++)
   if (!list[i].ptr)
    {
     list[i].ptr = mem;
     list[i].type = type;
     return mem;
    }

  // no holes - append
  if (count < capacity)
   {
    list[count].ptr = mem;
    list[count].type = type;
    count++;
    return mem;
   }

  // grow
  int newcap = capacity * 2;
  mem_entry *newlist = (mem_entry *)realloc (list, newcap * sizeof (mem_entry));
  if (!newlist) return nullptr;
  list = newlist;
  memset (list + capacity, 0, (newcap - capacity) * sizeof (mem_entry));
  capacity = newcap;
  list[count].ptr = mem;
  list[count].type = type;
  count++;
  return mem;
 }

 void *unregister_mem (void *mem)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1) 
   {
    list[idx].ptr = nullptr;
    list[idx].type = ptMALLOC;
   }
  return mem;
 }

 void *sf_alloc (int size)
 {
  if (!size) return nullptr;
  void *mem = malloc (size);
  if (mem) register_mem (mem, ptMALLOC);
  return mem;
 }

 void sf_free (void *dat)
 {
  if (dat)
   {
    unregister_mem (dat);
    free (dat);
   }
 }

 void free_all ()
 {
  for (int i = 0; i < count; i++)
   {
    if (list[i].ptr)
     {
      switch (list[i].type)
       {
        case ptMALLOC:
         free (list[i].ptr);
         break;

        case ptNEW:
         // Generic C++ object - can't delete without type info
         // For now, treat as error or use virtual destructor pattern
         free (list[i].ptr); // WRONG but prevents crash
         break;

        case ptNEW_ARRAY:
         // Can't delete[] without type info
         free (list[i].ptr); // WRONG but prevents crash
         break;

        case ptBMP:
         delete (bmpdraw *)list[i].ptr; // Correct deletion
         break;
       }
      list[i].ptr = nullptr;
     }
   }
  count = 0;
 }

 int size () const { return count; }
};
#pragma endregion

// For BMP objects:
bmpdraw *bmp = new bmpdraw();
mem_list.register_mem(bmp, ptBMP);

// For malloc strings:
char *str = (char*)malloc(100);
mem_list.register_mem(str, ptMALLOC);

// For matrices (assuming malloc):
float__t *matrix = (float__t*)malloc(sizeof(float__t) * rows * cols);
mem_list.register_mem(matrix, ptMALLOC);


typedef void (*deleter_fn)(void*);

struct mem_entry
{
 void *ptr;
 deleter_fn deleter;
};

class MemList
{
 // ... (same as above, but use deleter instead of type)

 void free_all ()
 {
  for (int i = 0; i < count; i++)
   {
    if (list[i].ptr && list[i].deleter)
     {
      list[i].deleter(list[i].ptr);
      list[i].ptr = nullptr;
     }
   }
  count = 0;
 }
};

// Deleters:
void delete_malloc(void* p) { free(p); }
void delete_bmp(void* p) { delete (bmpdraw*)p; }
void delete_array(void* p) { delete[] (char*)p; }

// Usage:
mem_list.register_mem(bmp, delete_bmp);
mem_list.register_mem(str, delete_malloc);



#pragma region Memory management class

enum ptr_type
{
 ptMALLOC = 0,  // Default: C-style malloc/free
 ptBMP = 1      // C++ bmpdraw* objects (new/delete)
};

struct mem_entry
{
 void *ptr;
 ptr_type type;
};

class MemList
{
 mem_entry *list;
 int capacity;
 int count;

 public:
 MemList (int initial = 256) : capacity (initial), count (0)
 {
  list = (mem_entry *)malloc (capacity * sizeof (mem_entry));
  if (list) memset (list, 0, capacity * sizeof (mem_entry));
 }

 ~MemList () { free (list); }

 void init_mem_list ()
 {
  if (list) memset (list, 0, capacity * sizeof (mem_entry));
  count = 0;
 }

 int search_mem (void *mem)
 {
  for (int i = 0; i < count; i++)
   if (list[i].ptr == mem) return i;
  return -1;
 }

 void *register_mem (void *mem, ptr_type type = ptMALLOC)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1) return mem; // already registered

  // fill holes first
  for (int i = 0; i < count; i++)
   if (!list[i].ptr)
    {
     list[i].ptr = mem;
     list[i].type = type;
     return mem;
    }

  // no holes - append
  if (count < capacity)
   {
    list[count].ptr = mem;
    list[count].type = type;
    count++;
    return mem;
   }

  // grow
  int newcap = capacity * 2;
  mem_entry *newlist = (mem_entry *)realloc (list, newcap * sizeof (mem_entry));
  if (!newlist) return nullptr;
  list = newlist;
  memset (list + capacity, 0, (newcap - capacity) * sizeof (mem_entry));
  capacity = newcap;
  list[count].ptr = mem;
  list[count].type = type;
  count++;
  return mem;
 }

 void *unregister_mem (void *mem)
 {
  if (!mem) return nullptr;
  int idx = search_mem (mem);
  if (idx != -1)
   {
    list[idx].ptr = nullptr;
    list[idx].type = ptMALLOC;
   }
  return mem;
 }

 void *sf_alloc (int size)
 {
  if (!size) return nullptr;
  void *mem = malloc (size);
  if (mem) register_mem (mem, ptMALLOC);
  return mem;
 }

 void sf_free (void *dat)
 {
  if (dat)
   {
    unregister_mem (dat);
    free (dat);
   }
 }

 void free_all ()
 {
  for (int i = 0; i < count; i++)
   {
    if (list[i].ptr)
     {
      switch (list[i].type)
       {
        case ptBMP:
         delete (bmpdraw *)list[i].ptr;
         break;

        case ptMALLOC:
        default:
         free (list[i].ptr);
         break;
       }
      list[i].ptr = nullptr;
     }
   }
  count = 0;
 }

 int size () const { return count; }
};
#pragma endregion

// В классе calculator, в private секции:
void *register_mem (void *mem, ptr_type type = ptMALLOC) { return mem_list.register_mem (mem, type); }